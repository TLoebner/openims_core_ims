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
 * Wharf CDF_Rf module -  
 * 
 * 
 *  \author Andreea Onofrei Corici andreea dot ancuta dot corici -at- fokus dot fraunhofer dot de
 * 
 */
 
#ifndef _Client_Rf_CHARGING_H
#define _Client_Rf_CHARGING_H

#ifdef WHARF
#include "../../utils/utils.h"
#endif /*WHARF*/

#include "../cdp/diameter.h"
#include "Rf_data.h"

typedef struct _charg_info_list_t_slot{
	str sip_uri;
	str an_charg_id;
	//time_t expires;
	struct _charg_info_list_t_slot * next, * prev;
} charg_info_list_slot_t;

typedef struct _charg_info_list_t{
	gen_lock_t * lock;
	struct _charg_info_list_t_slot * head, * tail;
} charg_info_list_t;

#define charg_info_list_t_free(x,mem) \
do{\
	if (x) {\
		str_free((x)->sip_uri,mem);\
		str_free((x)->an_charg_id,mem);\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

int init_charg_info();
void destroy_charg_info();


int Rf_add_chg_info(str sip_uri, str an_charg_id);

str get_charg_info(str sip_uri);

#endif
