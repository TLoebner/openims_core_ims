/*
 * $Id$
 *
 * Copyright (C) 2011 FhG Fokus
 *
 * This file is part of the Wharf project.
 */

#ifndef RF_SESSION_H_
#define RF_SESSION_H_

#include "../cdp/cdp_load.h"
#include "../cdp_avp/mod_export.h"

typedef struct _rf_session_list_t_slot{

	str hash_id;
	str session_id;
	int32_t ref_count;
	struct _rf_session_list_t_slot * next, * prev;
} rf_session_list_slot_t;

typedef struct _rf_session_list_t{
	gen_lock_t * lock;
	struct _rf_session_list_t_slot * head, * tail;
} rf_session_list_t;

#define rf_session_list_t_free(x,mem) \
		do{\
			if (x) {\
				str_free((x)->hash_id,mem);\
				str_free((x)->session_id,mem);\
				mem##_free(x);\
				(x)=0;\
			}\
		}while(0)

int init_rf_session_hash();
void destroy_rf_session_hash();

AAASession * create_rf_session();

str get_AAA_Session (str id);
void decr_ref_cnt_AAA_Session (str sessionid);

str get_rf_session(str id);
int add_rf_session(str id, str session_id);
void del_rf_session(str id);

#endif /* RF_SESSION_H_ */
