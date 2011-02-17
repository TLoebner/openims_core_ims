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

Rf_ACR_t * new_Rf_ACR(str origin_host, str origin_realm,
		str destination_realm,
		str * user_name, str * service_context_id,
		str callid, int side){

	Rf_ACR_t *x=0;
	mem_new(x, sizeof(Rf_ACR_t), pkg);

	str_dup(x->origin_host, origin_host, pkg);
	str_dup(x->origin_realm, origin_realm, pkg);
	str_dup(x->destination_realm, destination_realm, pkg);

	if(user_name)
		str_dup_ptr_ptr(x->user_name, user_name, pkg);
	if(service_context_id)
		str_dup_ptr(x->service_context_id, *service_context_id, pkg);
	return x;

out_of_memory:
	LOG(L_ERR, "out of pkg memory\n");
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
	
	str_free_ptr(x->service_id,pkg);
	
	WL_FREE_ALL(&(x->service_specific_info),service_specific_info_list_t,pkg);
	
	mem_free(x->cause_code,pkg);
	
	mem_free(x,pkg);
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


