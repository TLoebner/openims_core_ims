/*
 * $Id$
 * 
 * Copyright (C) 2009 FhG Fokus
 * 
 * This file is part of the Wharf project.
 * 
 */

/**
 * \file
 * 
 * Client_Rf module - Rf Data structures
 * 
 * 
 *  \author Dragos Vingarzan dragos dot vingarzan -at- fokus dot fraunhofer dot de
 * 
 */ 


#include "Rf_data.h"
#include "config.h"

#ifndef WHARF

/**
 * Duplicate a str, safely.
 * \Note This checks if:
 *  - src was an empty string
 *  - malloc failed
 * \Note On any error, the dst values are reset for safety
 * \Note A label "out_of_memory" must be defined in the calling function to handle
 * allocation errors. 
 * @param dst - destination str
 * @param src - source src
 * @param mem - type of mem to duplicate into (shm/pkg)
 */
#define str_dup(dst,src,mem) \
do {\
	if ((src).len) {\
		(dst).s = mem##_malloc((src).len);\
		if (!(dst).s){\
			LOG(L_ERR,"Error allocating %d bytes in %s!\n",(src).len,#mem);\
			(dst).len = 0;\
			goto out_of_memory;\
		}\
		memcpy((dst).s,(src).s,(src).len);\
		(dst).len = (src).len;\
	}else{\
		(dst).s=0;(dst).len=0;\
	}\
} while (0)

/**
 * Frees a str content.
 * @param x - the str to free
 * @param mem - type of memory that the content is using (shm/pkg)
 */
#define str_free(x,mem) \
do {\
	if ((x).s) mem##_free((x).s);\
	(x).s=0;(x).len=0;\
} while(0)

#endif /* WHARF */


extern client_rf_cfg cfg;
acct_record_info_list_t * acct_records;

int init_acct_records(){

	int i;

	mem_new(acct_records, cfg.hash_table_size*sizeof(acct_record_info_list_t), shm);

	for(i=0; i< cfg.hash_table_size; i++){
		acct_records[i].lock = lock_alloc();
		if(!acct_records[i].lock) goto out_of_memory;
		acct_records[i].lock=lock_init(acct_records[i].lock);

	}

	return 1;
out_of_memory:
	LOG(L_ERR, "out of shm memory");
	return 0;
}

void destroy_acct_records(){
	
	int i;
	for(i=0; i< cfg.hash_table_size; i++){

		lock_get(acct_records[i].lock);
		lock_destroy(acct_records[i].lock);
		lock_dealloc(acct_records[i].lock);

		WL_FREE_ALL(acct_records+i, acct_record_info_list_t,shm);
	}
}

/**
 * Computes the hash for a string.
 * @param id - input string
 * @returns - the hash
 */
inline unsigned int calc_hash(str id){
	if (id.len==0) return 0;
#define h_inc h+=v^(v>>3)
   char* p;
   register unsigned v;
   register unsigned h;

   h=0;
   for (p=id.s; p<=(id.s+id.len-4); p+=4){
       v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
       h_inc;
   }
   v=0;
   for (;p<(id.s+id.len); p++) {
       v<<=8;
       v+=*p;
   }
   h_inc;

   h=((h)+(h>>11))+((h>>13)+(h>>23));
   return (h)%cfg.hash_table_size;
#undef h_inc 
}

int add_new_acct_record_safe(int hash_index, str id, uint32_t acct_record_number, int dir, uint32_t expires){

	acct_record_info_list_slot_t * acct_rec = NULL;

	mem_new(acct_rec, sizeof(acct_record_info_list_slot_t), shm);
	acct_rec->acct_record_number = acct_record_number;
	acct_rec->expires = expires;
	acct_rec->dir = dir;
	str_dup(acct_rec->id, id, shm);
	WL_APPEND(acct_records+hash_index, acct_rec);

	return 1;

out_of_memory:
	LOG(L_ERR, "ERR: add_new_acct_record_safe: out of shm memory\n");
	return 0;
}

/**
 * search for the entry with the specific id, otherwise add it and initialize it with expires, if expires not 0 update
 */
int get_subseq_acct_record_nb(str id, int32_t acct_record_type, uint32_t * value, int dir, uint32_t expires){
	
	int hash_index;
	acct_record_info_list_slot_t * acct_rec = NULL;

	if(acct_record_type == AAA_ACCT_EVENT){
		*value = 1;
		return 1;
	}

	hash_index = calc_hash(id);
	lock_get(acct_records[hash_index].lock);
		WL_FOREACH(&(acct_records[hash_index]), acct_rec){
			if(acct_rec->dir ==dir && str_equal(acct_rec->id, id)){
				*value = acct_rec->acct_record_number +1;
				acct_rec->acct_record_number = *value;
				if(expires>0) acct_rec->expires = expires;
				break;
			}
		}
		if(!acct_rec && acct_record_type != AAA_ACCT_STOP){
			*value = 1;
			if(!add_new_acct_record_safe(hash_index, id, *value, dir, expires))
				goto error;
		}

		if(acct_rec && acct_record_type == AAA_ACCT_STOP){
			WL_DELETE(&(acct_records[hash_index]), acct_rec);
		}
	lock_release(acct_records[hash_index].lock);
	
	return 1;
error:
	lock_release(acct_records[hash_index].lock);
	return 0;
}


event_type_t * new_event_type(str * sip_method,
				str * event,
				uint32_t * expires)
{
	event_type_t * x = 0;
	
	mem_new(x, sizeof(event_type_t), pkg);
	if(sip_method && sip_method->s)
		str_dup_ptr(x->sip_method, *sip_method,pkg);
	if(event && event->s)
		str_dup_ptr(x->event,*event,pkg);
	if(expires && *expires!=0){
		mem_new(x->expires, sizeof(uint32_t), pkg);
		*(x->expires) = *expires;
	}
	return x;

out_of_memory:
	LOG(L_ERR, "out of pkg memory\n");
	event_type_free(x);
	return NULL;
}

time_stamps_t * new_time_stamps(time_t	*sip_request_timestamp,
		uint32_t *sip_request_timestamp_fraction,
		time_t 	*sip_response_timestamp,
		uint32_t *sip_response_timestamp_fraction){

	time_stamps_t * x= 0;

	mem_new(x, sizeof(time_stamps_t),pkg);

	if(sip_request_timestamp && *sip_request_timestamp>0){
		mem_new(x->sip_request_timestamp, sizeof(time_t), pkg);
		*(x->sip_request_timestamp) = *sip_request_timestamp;
	}

	if(sip_request_timestamp_fraction && *sip_request_timestamp_fraction>0){
		mem_new(x->sip_request_timestamp_fraction, sizeof(uint32_t), pkg);
		*(x->sip_request_timestamp_fraction) = *sip_request_timestamp_fraction;
	}

	if(sip_response_timestamp && *sip_response_timestamp>0){
		mem_new(x->sip_response_timestamp, sizeof(time_t), pkg);
		*(x->sip_response_timestamp) = *sip_response_timestamp;
	}

	if(sip_response_timestamp_fraction && *sip_response_timestamp_fraction>0){
		mem_new(x->sip_response_timestamp_fraction, sizeof(uint32_t), pkg);
		*(x->sip_response_timestamp_fraction) = *sip_response_timestamp_fraction;
	}


	return x;

out_of_memory:
	LOG(L_ERR, "out of pkg memory\n");
	time_stamps_free(x);
	return 0;
}

ims_information_t * new_ims_information(event_type_t * event_type,
					time_stamps_t * time_stamps,
					str * user_session_id, 
					str * outgoing_session_id,
					str * calling_party,
					str * called_party,
					str * icid,
					str * orig_ioi,
					str * term_ioi,
					sdp_media_component_list_t * sdp_media_comps,
					int node_role)
{

	str_list_slot_t *sl =0;
	ims_information_t *x = 0;
	ioi_list_element_t * ioi_elem = 0;

	mem_new(x, sizeof(ims_information_t), pkg);

	x->event_type = event_type;
	x->time_stamps = time_stamps;

	mem_new(x->role_of_node,sizeof(int32_t),pkg);
	*(x->role_of_node) = node_role;

	x->node_functionality = cfg.node_func;

	if(outgoing_session_id && outgoing_session_id->s)
		str_dup_ptr(x->outgoing_session_id,*outgoing_session_id, pkg);

	if(user_session_id && user_session_id->s)
		str_dup_ptr(x->user_session_id, *user_session_id, pkg);
	
	if(calling_party && calling_party->s){
		mem_new(sl, sizeof(str_list_slot_t), pkg);
		str_dup(sl->data, *calling_party, pkg);
		WL_APPEND(&(x->calling_party_address), sl);
	}

	if(called_party && called_party->s)
		str_dup_ptr(x->called_party_address, *called_party, pkg);

	//WL_FREE_ALL(&(x->called_asserted_identity),str_list_t,pkg);
	//str_free_ptr(x->requested_party_address,pkg);
	
	if ((orig_ioi && orig_ioi->s) || 
			(term_ioi && term_ioi->s)){
		mem_new(ioi_elem, sizeof(ioi_list_element_t),pkg) ;
		if(orig_ioi)
			str_dup_ptr_ptr(ioi_elem->info.originating_ioi, orig_ioi, pkg);
		if(term_ioi)
			str_dup_ptr_ptr(ioi_elem->info.terminating_ioi, term_ioi, pkg);
	}

	if (icid && icid->s)
		str_dup_ptr(x->icid,*icid,pkg);

	x->sdp_media_component.head = sdp_media_comps->head;
	x->sdp_media_component.tail = sdp_media_comps->tail;
	
	return x;

out_of_memory:
	LOG(L_ERR, "out of pkg memory\n");
	ims_information_free(x);
	return NULL;
}

service_information_t * new_service_information(ims_information_t * ims_info,
		subscription_id_t * subscription)
{
	service_information_t * x = 0;
	subscription_id_list_element_t * sl =0;

	mem_new(x, sizeof(service_information_t), pkg);
	
	x->ims_information = ims_info;
	if(subscription){
		mem_new(sl, sizeof(subscription_id_list_element_t), pkg);
		subscription_id_list_t_copy(&(sl->s),subscription,pkg); 
		WL_APPEND(&(x->subscription_id),sl);
	}
	
	return x;

out_of_memory:
	LOG(L_ERR, "new service information: out of pkg memory\n");
	service_information_free(x);
	return 0;
}


Rf_ACR_t * new_Rf_ACR(int32_t acct_record_type, uint32_t acct_record_number, 
			str * user_name, ims_information_t * ims_info,
			subscription_id_t * subscription){


	Rf_ACR_t *x=0;
	service_information_t * service_info=0;
	
	mem_new(x, sizeof(Rf_ACR_t), pkg);

	str_dup(x->origin_host, cfg.origin_host, pkg);
	str_dup(x->origin_realm, cfg.origin_realm, pkg);
	str_dup(x->destination_realm, cfg.destination_realm, pkg);
	x->acct_record_type = acct_record_type;
	x->acct_record_number = acct_record_number;

	if(user_name){
		str_dup_ptr_ptr(x->user_name, user_name, pkg);
	}
	
	if(cfg.service_context_id && cfg.service_context_id->s)
		str_dup_ptr(x->service_context_id, *(cfg.service_context_id), pkg);

	if(ims_info)	
		if(!(service_info = new_service_information(ims_info, subscription)))
			goto error;
	
	x->service_information = service_info;
	service_info = 0;

	return x;

out_of_memory:
	LOG(L_ERR, "out of pkg memory\n");
error:
	Rf_free_ACR(x);
	service_information_free(service_info);

	return 0;
}

void event_type_free(event_type_t *x)
{
	if (!x) return;
	str_free_ptr(x->sip_method,pkg);
	str_free_ptr(x->event,pkg);
	mem_free(x->expires,pkg);
	mem_free(x,pkg);
}

void time_stamps_free(time_stamps_t *x)
{
	if (!x) return;
	mem_free(x->sip_request_timestamp,pkg);
	mem_free(x->sip_request_timestamp_fraction,pkg);
	mem_free(x->sip_response_timestamp,pkg);
	mem_free(x->sip_response_timestamp_fraction,pkg);
	mem_free(x,pkg);
}

void ims_information_free(ims_information_t *x)
{
	if (!x) return;

	LOG(L_DBG, "freeing ims info\n");
	event_type_free(x->event_type);
	
	mem_free(x->role_of_node,pkg);
	str_free_ptr(x->user_session_id,pkg);
	str_free_ptr(x->outgoing_session_id,pkg);
	
	WL_FREE_ALL(&(x->calling_party_address),str_list_t,pkg);
	str_free_ptr(x->called_party_address,pkg);
	WL_FREE_ALL(&(x->called_asserted_identity),str_list_t,pkg);
	str_free_ptr(x->requested_party_address,pkg);
	
	time_stamps_free(x->time_stamps);
	
	WL_FREE_ALL(&(x->as_info),as_info_list_t,pkg);
	
	WL_FREE_ALL(&(x->ioi),ioi_list_t,pkg);
	str_free_ptr(x->icid,pkg);
	
	WL_FREE_ALL(&(x->sdp_media_component),sdp_media_component_list_t,pkg);
	WL_FREE_ALL(&(x->sdp_session_description),str_list_t,pkg);
	
	str_free_ptr(x->service_id,pkg);
	
	WL_FREE_ALL(&(x->service_specific_info),service_specific_info_list_t,pkg);
	
	mem_free(x->cause_code,pkg);
	
	mem_free(x,pkg);
	LOG(L_DBG, "end of free ims info\n");
}

void service_information_free(service_information_t *x)
{
	if (!x) return;

	WL_FREE_ALL(&(x->subscription_id),subscription_id_list_t,pkg);
	ims_information_free(x->ims_information);
	
	mem_free(x,pkg);
}


void Rf_free_ACR(Rf_ACR_t *x)
{
	if (!x) return;

	str_free(x->origin_host,pkg);
	str_free(x->origin_realm,pkg);
	str_free(x->destination_realm,pkg);
	
	str_free_ptr(x->user_name,pkg);
	mem_free(x->acct_interim_interval,pkg);
	mem_free(x->origin_state_id,pkg);
	mem_free(x->event_timestamp,pkg);
	
	str_free_ptr(x->service_context_id,pkg);
	
	service_information_free(x->service_information);
	
	mem_free(x,pkg);
}

/*
void Rf_free_ACA(Rf_ACA_t *x)
{
	if (!x) return;
	
	str_free_ptr(x->user_name,pkg);
	mem_free(x->acct_interim_interval,pkg);
	mem_free(x->origin_state_id,pkg);
	mem_free(x->event_timestamp,pkg);
	
	mem_free(x,pkg);
}
*/


