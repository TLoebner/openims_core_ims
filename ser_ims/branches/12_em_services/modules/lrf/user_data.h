#ifndef LRF_USER_DATA_H
#define LRF_USER_DATA_H

/*
 *  file user_data.h
 *  author Ancuta Onofrei ancuta_onofrei -at- yahoo dot com
 */

#include "../../str.h"
#include "../../locking.h"
#include "../../ut.h"

#include <lost/client.h>
#include <lost/parsing.h>
#include <lost/pidf_loc.h>


/** Structure for a LRF user data cell */
typedef struct user_d_cell{
	unsigned int hash;
	str esqk;
	str user_uri;
	str service; /*e.g urn:service:sos*/
	str psap_uri;
	str loc_str;
	xmlNode * loc;
	loc_fmt l_fmt;
	struct user_d_cell * next;
	struct user_d_cell * prev;
}user_d;

/** Structure for a LRF user data hash slot */
typedef struct {
	user_d *head;						/**< first dialog in this dialog hash slot 		*/
	user_d *tail;						/**< last dialog in this dialog hash slot 		*/
	gen_lock_t *lock;					/**< slot lock 									*/	
} user_d_hash_slot;

int init_lrf_user_data(int hash_size);
void destroy_lrf_user_data();

user_d * add_user_data(str user_uri, str service);
/*get_user_data acquires the lock of the d->hash also. Do not forget to unlock it*/
user_d * get_user_data(str user_uri, str service);
void lrf_unlock(unsigned int i);
void del_user_data(user_d *d);

void print_lrf_user_data();
void user_data_timer(unsigned int ticks, void* param);

int LRF_alloc_user_data(struct sip_msg* msg, char*str1, char*str2);
int LRF_del_user_data(struct sip_msg* msg, char* str1, char* str2);
int LRF_call_query_resp(struct sip_msg* msg, char*str1, char*str2);

#endif
