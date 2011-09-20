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
 * Client_Rf Wharf module interface
 * 
 * 
 *  \author Andreea Ancuta Corici andreea dot ancuta dot corici -at- fokus dot fraunhofer dot de
 * 
 */


#ifdef WHARF
#ifndef _Client_Rf_EXPORT__H
#define _Client_Rf_EXPORT__H

#include "../cdp/session.h"
#include "Rf_data.h"

typedef int (*AAASendAccRequest_f)(str *session_id, Rf_ACR_t * rf_data);
typedef int (*Rf_add_an_chg_info_f)(str sip_uri, str an_charg_id);
typedef int (*Rf_add_ims_chg_info_icid_f)(str callid, int dir, str ims_charg_id);
typedef int (*Rf_add_ims_chg_ps_info_f) (str call_id, int dir, uint32_t rating_group);
typedef str (*Rf_get_AAA_Session_f) (str id);
typedef void (*Rf_decr_ref_cnt_AAA_Session_f) (str id);
typedef void (*Rf_incr_ref_cnt_AAA_Session_f) (str id);
typedef Rf_ACR_t* (*Rf_create_Rf_data_f) (str sessionid, int32_t acct_record_type);
typedef void (*Rf_free_Rf_data_f) (Rf_ACR_t *);


struct client_rf_bind_t{
	AAASendAccRequest_f	AAASendACR;
	Rf_add_an_chg_info_f	Rf_add_an_chg_info;
	Rf_add_ims_chg_info_icid_f	Rf_add_ims_chg_info_icid;
	Rf_add_ims_chg_ps_info_f	Rf_add_ims_chg_ps_info;
	Rf_get_AAA_Session_f		get_AAA_Session;
	Rf_decr_ref_cnt_AAA_Session_f	decr_ref_cnt_AAA_Session;
	Rf_incr_ref_cnt_AAA_Session_f	incr_ref_cnt_AAA_Session;
	Rf_create_Rf_data_f		create_Rf_data;
	Rf_free_Rf_data_f		free_Rf_data;
};

#endif
#endif /* WHARF */

