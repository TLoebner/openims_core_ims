/*
* $Id$
 *
 * Copyright (C) 2011 FhG Fokus
 *
 * This file is part of the Wharf project.
 */

#include "Rf_data.h"
#include "config.h"
#include "rf_session.h"

#ifdef WHARF
#define M_NAME "Client_Rf"
#endif

extern cdp_avp_bind_t *cavpb;
extern client_rf_cfg cfg;
rf_session_list_t * rf_session_hash;

AAASession * create_rf_session(){

	AAASession * auth = NULL;

	LOG(L_INFO,"INFO:"M_NAME":create_rf_session: creating Rf Session\n");
        auth = cavpb->cdp->AAACreateSession(NULL);
        if (!auth) {
                LOG(L_ERR,"ERR:"M_NAME":create_rf_session: unable to create the Rf Session\n");
                goto error;
        }
error:
	return auth;
}

str get_AAA_Session(str id){

	str sessionid = {0,0};
	AAASession * auth = NULL;

	sessionid= get_rf_session(id);
	if(!sessionid.len || !sessionid.s){
		sessionid.len =0; sessionid.s=0;
		auth = create_rf_session();
		if(!auth)
			return sessionid;
		if(!add_rf_session(id, auth->id)){
			cavpb->cdp->AAADropSession(auth);
			return sessionid;
		}
		str_dup(sessionid, auth->id, shm);
	}

	return sessionid;
out_of_memory:
	LOG(L_ERR, "out of shm memory");
	if(auth)
		cavpb->cdp->AAADropSession(auth);
	del_rf_session(id);
	return sessionid;
}

void decr_ref_cnt_AAA_Session(str sessionid){

}

int init_rf_session_hash(){

	int i;

	mem_new(rf_session_hash, cfg.hash_table_size*sizeof(rf_session_list_t), shm);

	for(i=0; i< cfg.hash_table_size; i++){
		rf_session_hash[i].lock = lock_alloc();
		if(!rf_session_hash[i].lock) goto out_of_memory;
		rf_session_hash[i].lock=lock_init(rf_session_hash[i].lock);
	}

	return 1;
out_of_memory:
	LOG(L_ERR, "out of shm memory");
	return 0;
}

void destroy_rf_session_hash(){

	int i;
	for(i=0; i< cfg.hash_table_size; i++){

		lock_get(rf_session_hash[i].lock);
		lock_destroy(rf_session_hash[i].lock);
		lock_dealloc(rf_session_hash[i].lock);

		WL_FREE_ALL(rf_session_hash+i, rf_session_list_t,shm);
	}
}

/**
 * Computes the hash for a string.
 * @param id - input string
 * @returns - the hash
 */
inline unsigned int rf_session_calc_hash(str id){
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

int add_new_rf_session_safe(int hash_index, str id, str session_id/*, uint32_t expires*/){

	rf_session_list_slot_t * info = NULL;

	mem_new(info, sizeof(rf_session_list_slot_t), shm);
	str_dup(info->hash_id, id, shm);
	str_dup(info->session_id, session_id, shm);
	WL_APPEND(rf_session_hash+hash_index, info);

	return 1;

out_of_memory:
	LOG(L_ERR, "ERR: add_new_rf_session_safe: out of shm memory\n");
	return 0;
}

int add_rf_session(str id, str session_id){

	int hash_index;
//	rf_session_list_slot_t * info = NULL;

	hash_index = rf_session_calc_hash(id);

	lock_get(rf_session_hash[hash_index].lock);
			/*WL_FOREACH(&(rf_session_hash[hash_index]), info){
				if(str_equal(info->hash_id, hash_id) &&
					!(str_equal(info->session_id, session_id))){
					str_free(info->session_id, shm);
					str_dup(info->session_id, session_id, shm);
					goto end;
				}
			}*/
			if(!add_new_rf_session_safe(hash_index, id, session_id))
				goto error;

//end:
	lock_release(rf_session_hash[hash_index].lock);

	return 1;
//out_of_memory:
	//LOG(L_ERR, "out of memory");
error:
	lock_release(rf_session_hash[hash_index].lock);
	return 0;
}

void del_rf_session(str id){

	int hash_index;
	rf_session_list_slot_t * info = NULL;

	hash_index = rf_session_calc_hash(id);

	lock_get(rf_session_hash[hash_index].lock);
			WL_FOREACH(&(rf_session_hash[hash_index]), info){
				if(str_equal(info->hash_id, id)){
					WL_DELETE(rf_session_hash+hash_index, info);
					WL_FREE(info,rf_session_list_t,shm);
					break;
				}
			}

	lock_release(rf_session_hash[hash_index].lock);
}

/**
 * Retrieve the session id in pkg, stored at registration time
 * @param id - the hash id of the entry to be searched for
 * @returns session_id associated with the id
 */
str get_rf_session(str id){

	str res = {0,0};

	int hash_index;
	rf_session_list_slot_t * info = NULL;

	hash_index = rf_session_calc_hash(id);

	lock_get(rf_session_hash[hash_index].lock);
			WL_FOREACH(&(rf_session_hash[hash_index]), info){
				if(str_equal(info->hash_id, id)){
					str_dup(res, info->session_id, pkg);
					goto end;
				}
			}
end:
	lock_release(rf_session_hash[hash_index].lock);
	return res;
out_of_memory:
	LOG(L_ERR, "out of pkg memory while trying to retrieve the an charg id\n");
	lock_release(rf_session_hash[hash_index].lock);
	return res;
}
