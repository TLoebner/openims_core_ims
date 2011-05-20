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
 * Diameter Rf interface towards the CDF - Wharf module interface
 * 
 * 
 *  \author Andreea Ancuta Corici andreea dot ancuta dot corici -at- fokus dot fraunhofer dot de
 * 
 */

#include "Rf_data.h"
#include "config.h"
#include "diameter_rf.h"
#include "acr.h"

#ifdef WHARF
#define M_NAME "Client_Rf"
#endif

#include "config.h"
#include "charging.h"

extern client_rf_cfg cfg;
an_charg_info_list_t * an_charg_info;
ims_charg_info_list_t * ims_charg_info;

int init_an_charg_info(){

	int i;

	mem_new(an_charg_info, cfg.hash_table_size*sizeof(an_charg_info_list_t), shm);

	for(i=0; i< cfg.hash_table_size; i++){
		an_charg_info[i].lock = lock_alloc();
		if(!an_charg_info[i].lock) goto out_of_memory;
		an_charg_info[i].lock=lock_init(an_charg_info[i].lock);

	}

	return 1;
out_of_memory:
	LOG(L_ERR, "out of shm memory");
	return 0;
}

void destroy_an_charg_info(){

	int i;
	for(i=0; i< cfg.hash_table_size; i++){

		lock_get(an_charg_info[i].lock);
		lock_destroy(an_charg_info[i].lock);
		lock_dealloc(an_charg_info[i].lock);

		WL_FREE_ALL(an_charg_info+i, an_charg_info_list_t,shm);
	}
}

/**
 * Computes the hash for a string.
 * @param id - input string
 * @returns - the hash
 */
inline unsigned int an_charg_info_calc_hash(str id){
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

int add_new_an_charg_info_safe(int hash_index, str sip_uri, str an_charg_id/*, uint32_t expires*/){

	an_charg_info_list_slot_t * info = NULL;

	mem_new(info, sizeof(an_charg_info_list_slot_t), shm);
	str_dup(info->sip_uri, sip_uri, shm);
	str_dup(info->an_charg_id, an_charg_id, shm);
	WL_APPEND(an_charg_info+hash_index, info);

	return 1;

out_of_memory:
	LOG(L_ERR, "ERR: add_new_and_charg_info_safe: out of shm memory\n");
	return 0;
}

int Rf_add_an_chg_info(str sip_uri, str an_charg_id){

	int hash_index;
	an_charg_info_list_slot_t * info = NULL;

	hash_index = an_charg_info_calc_hash(sip_uri);

	lock_get(an_charg_info[hash_index].lock);
			WL_FOREACH(&(an_charg_info[hash_index]), info){
				if(str_equal(info->sip_uri, sip_uri) && 
					!(str_equal(info->an_charg_id, an_charg_id))){
					str_free(info->an_charg_id, shm);
					str_dup(info->an_charg_id, an_charg_id, shm);
					goto end;
				}
			}
			if(!add_new_an_charg_info_safe(hash_index, sip_uri, an_charg_id))
				goto error;

end:
	lock_release(an_charg_info[hash_index].lock);

	return 1;
out_of_memory:
	LOG(L_ERR, "out of memory");
error:
	lock_release(an_charg_info[hash_index].lock);
	return 0;
}

/**
 * Retrieve the charging id in pkg, stored at registration time
 * @param sip_uri - the SIP URI of the user
 * @returns an_charging_id assoctiated with the user SIP URI
 */
str get_an_charg_info(str sip_uri){

	str res = {0,0};

	int hash_index;
	an_charg_info_list_slot_t * info = NULL;

	hash_index = an_charg_info_calc_hash(sip_uri);

	lock_get(an_charg_info[hash_index].lock);
			WL_FOREACH(&(an_charg_info[hash_index]), info){
				if(str_equal(info->sip_uri, sip_uri)){
					str_dup(res, info->an_charg_id, pkg);
					goto end;
				}
			}
end:
	lock_release(an_charg_info[hash_index].lock);
	return res;
out_of_memory:
	LOG(L_ERR, "out of pkg memory while trying to retrieve the an charg id\n");
	lock_release(an_charg_info[hash_index].lock);
	return res;
}

int init_ims_charg_info(){

	int i;

	mem_new(ims_charg_info, cfg.hash_table_size*sizeof(ims_charg_info_list_t), shm);

	for(i=0; i< cfg.hash_table_size; i++){
		ims_charg_info[i].lock = lock_alloc();
		if(!ims_charg_info[i].lock) goto out_of_memory;
		ims_charg_info[i].lock=lock_init(ims_charg_info[i].lock);

	}

	return 1;
out_of_memory:
	LOG(L_ERR, "out of shm memory");
	return 0;
}

void destroy_ims_charg_info(){

	int i;
	for(i=0; i< cfg.hash_table_size; i++){

		lock_get(ims_charg_info[i].lock);
		lock_destroy(ims_charg_info[i].lock);
		lock_dealloc(ims_charg_info[i].lock);

		WL_FREE_ALL(ims_charg_info+i, ims_charg_info_list_t,shm);
	}
}


/**
 * Computes the hash for a string.
 * @param id - input string
 * @returns - the hash
 */
inline unsigned int ims_charg_info_calc_hash(str id){
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

int add_new_ims_charg_info_safe(int hash_index, str call_id, str ims_charg_id/*, uint32_t expires*/){

	ims_charg_info_list_slot_t * info = NULL;

	mem_new(info, sizeof(ims_charg_info_list_slot_t), shm);
	str_dup(info->call_id, call_id, shm);
	str_dup(info->ims_charg_id, ims_charg_id, shm);
	WL_APPEND(ims_charg_info+hash_index, info);

	return 1;

out_of_memory:
	LOG(L_ERR, "ERR: add_new_ims_charg_info_safe: out of shm memory\n");
	return 0;
}

int Rf_add_ims_chg_info(str call_id, str ims_charg_id){

	int hash_index;
	ims_charg_info_list_slot_t * info = NULL;

	//LOG(L_DBG, "adding new ims chg info for callid %.*s and ims charg id %.*s\n",
	//	call_id.len, call_id.s, ims_charg_id.len, ims_charg_id.s);

	if(!call_id.len || !call_id.s || !ims_charg_id.len || !ims_charg_id.s){
		LOG(L_WARN, "WARN: Client_Rf: Rf_add_ims_chg_info: empty argument\n");
		return 1;
	}

	hash_index = ims_charg_info_calc_hash(call_id);

	lock_get(ims_charg_info[hash_index].lock);
			WL_FOREACH(&(ims_charg_info[hash_index]), info){
				if(str_equal(info->call_id, call_id) &&
					!str_equal(info->ims_charg_id, ims_charg_id)){
					str_free(info->ims_charg_id, shm);
					str_dup(info->ims_charg_id, ims_charg_id, shm);
					goto end;
				}
			}
			if(!add_new_ims_charg_info_safe(hash_index, call_id, ims_charg_id))
				goto error;

end:
	lock_release(ims_charg_info[hash_index].lock);

	return 1;
out_of_memory:
	LOG(L_ERR, "out of memory");
error:
	lock_release(ims_charg_info[hash_index].lock);
	return 0;
}

/**
 * Retrieve the charging id in pkg, stored at registration time
 * @param call_id - the SIP call_id
 * @returns ims_charging_id assoctiated with the call id
 */
str get_ims_charg_info(str call_id){

	str res = {0,0};

	int hash_index;
	ims_charg_info_list_slot_t * info = NULL;

	hash_index = ims_charg_info_calc_hash(call_id);

	lock_get(ims_charg_info[hash_index].lock);
			WL_FOREACH(&(ims_charg_info[hash_index]), info){
				if(str_equal(info->call_id, call_id)){
					str_dup(res, info->ims_charg_id, pkg);
					goto end;
				}
			}
end:
	lock_release(ims_charg_info[hash_index].lock);
	return res;
out_of_memory:
	LOG(L_ERR, "out of pkg memory while trying to retrieve the ims charg id\n");
	lock_release(ims_charg_info[hash_index].lock);
	return res;
}

