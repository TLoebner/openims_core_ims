/**
 * $Id$ cx_avp.h $Date$ $Author$ dvi Dragos Vingarzan
 *
 * I/S-CSCF Module - Cx AVP Operations
 * 
 * Copyright (C) 2005 FhG Fokus
 * 		
 */
 
#ifndef IS_CSCF_CX_AVP_H
#define IS_CSCF_CX_AVP_H

#include "../../sr_module.h"
#include "mod.h"
#include "../cdp/cdp_load.h"


/** NO DATA WILL BE DUPLICATED OR FREED - DO THAT AFTER SENDING THE MESSAGE!!! */

int Cx_add_user_name(AAAMessage *uar,str private_identity);
int Cx_add_public_identity(AAAMessage *msg,str data);
int Cx_add_visited_network_id(AAAMessage *msg,str data);
int Cx_add_authorization_type(AAAMessage *msg,unsigned int data);

int Cx_add_server_name(AAAMessage *msg,str data);
int Cx_add_sip_number_auth_items(AAAMessage *msg,unsigned int data);
int Cx_add_sip_auth_data_item_request(AAAMessage *msg,str auth_scheme,str auth);
int Cx_add_server_assignment_type(AAAMessage *msg,unsigned int data);
int Cx_add_userdata_available(AAAMessage *msg,unsigned int data);
int Cx_add_result_code(AAAMessage *msg,unsigned int data);
int Cx_add_experimental_result_code(AAAMessage *msg,unsigned int data);
int Cx_add_vendor_specific_appid(AAAMessage *msg,unsigned int vendor_id,
	unsigned int auth_id,unsigned int acct_id);
int Cx_add_auth_session_state(AAAMessage *msg,unsigned int data);	
int Cx_add_destination_realm(AAAMessage *msg,str data);

/* GET AVPS */

str Cx_get_session_id(AAAMessage *msg);
str Cx_get_user_name(AAAMessage *msg);
str Cx_get_public_identity(AAAMessage *msg);
AAA_AVP* Cx_get_next_public_identity(AAAMessage *msg,AAA_AVP* pos,int avp_code,int vendor_id,const char *func);		
str Cx_get_visited_network_id(AAAMessage *msg);
int Cx_get_authorization_type(AAAMessage *msg, int *data);
int Cx_get_server_assignment_type(AAAMessage *msg, int *data);
int Cx_get_userdata_available(AAAMessage *msg, int *data);
int Cx_get_result_code(AAAMessage *msg,int *data);
int Cx_get_experimental_result_code(AAAMessage *msg, int *data);
str Cx_get_server_name(AAAMessage *msg);
int Cx_get_capabilities(AAAMessage *msg,int **m,int *m_cnt,int **o,int *o_cnt);
int Cx_get_sip_number_auth_items(AAAMessage *msg,int *data);
int Cx_get_auth_data_item_request(AAAMessage *msg,
		 str *auth_scheme, str *authorization);
int Cx_get_auth_data_item_answer(AAAMessage *msg, AAA_AVP **auth_data,
	int *item_number,str *auth_scheme,str *authenticate,str *authorization,
	str *ck,str *ik);
	
str Cx_get_destination_host(AAAMessage *msg);	
str Cx_get_user_data(AAAMessage *msg);	
str Cx_get_charging_info(AAAMessage *msg);

#endif /* IS_CSCF_CX_AVP_H */
