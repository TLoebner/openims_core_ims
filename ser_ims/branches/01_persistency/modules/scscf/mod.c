/*
 * $Id$
 *  
 * Copyright (C) 2004-2006 FhG Fokus
 *
 * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
 * implementation
 *
 * Open IMS Core is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the Open IMS Core software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact Fraunhofer FOKUS by e-mail at the following
 * addresses:
 *     info@open-ims.org
 *
 * Open IMS Core is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * It has to be noted that this Open Source IMS Core System is not 
 * intended to become or act as a product in a commercial context! Its 
 * sole purpose is to provide an IMS core reference implementation for 
 * IMS technology testing and IMS application prototyping for research 
 * purposes, typically performed in IMS test-beds.
 * 
 * Users of the Open Source IMS Core System have to be aware that IMS
 * technology may be subject of patents and licence terms, as being 
 * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
 * standards. Thus all Open IMS Core users have to take notice of this 
 * fact and have to agree to check out carefully before installing, 
 * using and extending the Open Source IMS Core System, if related 
 * patents and licences may become applicable to the intended usage 
 * context.  
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
 
/**
 * \file
 * 
 * Serving-CSCF - SER module interface
 * 
 * Scope:
 * - Exports parameters and functions
 * - Initialization functions
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "mod.h"

#include "../../sr_module.h"
#include "../../timer.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"

#include "registration.h"
#include "registrar.h"
#include "registrar_storage.h"
#include "registrar_notify.h"
#include "sip.h"
#include "cx.h"
#include "scscf_load.h"
#include "dlg_state.h"
#include "s_persistency.h"

MODULE_VERSION

static int mod_init(void);
static int mod_child_init(int rank);
static void mod_destroy(void);


/* parameters storage */
char* scscf_name="sip:scscf.open-ims.test:6060";	/**< name of the S-CSCF */

char* scscf_aaa_peer="hss.open-ims.test";/**< FQDN of the Diameter Peer (HSS) */

char *scscf_user_data_dtd=0; 			/* Path to "CxDataType.dtd" 	 							*/
char *scscf_user_data_xsd=0; 			/* Path to "CxDataType_Rel6.xsd" or "CxDataType_Rel7.xsd"	*/

int auth_data_hash_size=1024;			/**< the size of the hash table 							*/
int auth_vector_timeout=60;				/**< timeout for a sent auth vector to expire in sec 		*/
int auth_data_timeout=60;				/**< timeout for a hash entry to expire when empty in sec 	*/
int av_request_at_once=1;				/**< how many auth vectors to request in a MAR 				*/
int av_request_at_sync=1;				/**< how many auth vectors to request in a sync MAR 		*/	

int server_assignment_store_data=0; 	/**< whether to ask to keep the data in SAR 	*/

int registrar_hash_size=1024;			/**< the size of the hash table					*/
int registration_default_expires=3600;	/**< the default value for expires if none found*/
int registration_min_expires=10;		/**< minimum registration expiration time 		*/
int registration_max_expires=1000000;	/**< maximum registration expiration time 		*/
char* registration_default_algorithm="AKAv1-MD5";	/**< default algorithm for registration (if none present)*/
str registration_default_algorithm_s={0,0};	/**< fixed default algorithm for registration (if none present)	 */

int subscription_default_expires=3600;	/**< the default value for expires if none found*/
int subscription_min_expires=10;		/**< minimum subscription expiration time 		*/
int subscription_max_expires=1000000;	/**< maximum subscription expiration time 		*/


int append_branches=1;					/**< if to append branches						*/

int scscf_dialogs_hash_size=256;		/**< size of the dialog hash table 				*/
int scscf_dialogs_expiration_time=3600;	/**< default expiration time for dialogs		*/

persistency_mode_t scscf_persistency_mode=NO_PERSISTENCY;			/**< the type of persistency				*/
char* scscf_persistency_location="/opt/OpenIMSCore/persistency";	/**< where to dump the persistency data 	*/
int scscf_persistency_auth_data_timer=60;							/**< interval to snapshot authorization data*/ 

/* fixed parameter storage */
str scscf_name_str;						/**< fixed name of the S-CSCF 							*/
str scscf_record_route_mo;				/**< the record route header for Mobile Originating 	*/
str scscf_record_route_mt;				/**< the record route header for Mobile Terminating 	*/
str scscf_record_route_mo_uri;			/**< just the record route uri for Mobile Originating 	*/
str scscf_record_route_mt_uri;			/**< just the record route uri for Mobile Terminating 	*/
str scscf_service_route;				/**< the service route header							*/
str scscf_service_route_uri;			/**< just the service route uri 						*/
str scscf_registration_min_expires;		/**< fixed minimum registration expiration time 		*/
str scscf_subscription_min_expires;		/**< fixed minimum subscription expiration time 		*/
str scscf_aaa_peer_str;					/**< fixed FQDN of the Diameter Peer (HSS) 				*/

int * callback_singleton;				/**< Cx callback singleton 								*/

/** 
 * Exported functions.
 * - load_scscf() - load the S-CSCF bindings
 * - get_r_public() - find a r_public structure in the registrar and return it (locks the registrar)
 * - r_unlock() - unlock a registrar slot
 * <p>
 * - S_is_integrity_protected() - checks if there is a integrity-protected="yes" in the Authorization header
 * - S_challenge() - challenge a REGISTER
 * - S_is_authorized() - check if the Authorization header is correct in a REGISTER
 * - S_add_path_service_routes() - Add Path, Service-Route to a REGISTER response
 * - S_add_service_route() - Add Service-Route to a REGISTER response
 * - S_check_visited_network_id() - check if the P-Visited-Network-Id header matches a regexp
 * <p>
 * - S_assign_server() - trigger the Server Assignment Operation for registered/de-registered users
 * - S_assign_server_unreg() - trigger the Server Assignment Operation for unregistered user
 * - S_update_contacts() - update the registrar with the contacts found in a REGISTER
 * - S_lookup() - performa a registrar lookup for the Request-URI and replace with contact
 * - S_term_registered() - check if the terminated user is registered here
 * - S_term_unregistered() - check if the terminated user is unregistered here
 * - S_term_not_registered() - check if the terminated user is not-registered here
 * - S_is_not_registered() - check if the originating user is not-registered here
 * <p>
 * - S_can_subscribe() - check if the originator of a SUBCRIBE to reg is authorized to do that here
 * - S_subscribe() - accept a SUBCRIBE for reg here
 * <p>
 * - S_trans_in_processing() - check if the transaction is already in processing
 * <p>
 * - S_mobile_originating() - check if this message is in the Mobile Originating case
 * <p>
 * - S_originating_barred() - check if the originating user is barred
 * - S_terminating_barred() - check if the terminating user is barred
 * <p>
 * - S_is_in_dialog() - check if this message is a subsequent one for a dialog
 * - S_save_dialog() - save a dialog
 * - S_update_dialog() - update a dialog - drops the dialogs if terminated
 * - S_record_route() - record routes
 * - S_is_record_routed() - check if we already record-routed
 */
static cmd_export_t scscf_cmds[]={
	{"load_scscf",					(cmd_function)load_scscf, 	NO_SCRIPT, 0, 0},
	{"get_r_public",				(cmd_function)get_r_public, NO_SCRIPT, 0, 0},
	{"get_r_public_expires",		(cmd_function)get_r_public_expires, NO_SCRIPT, 0, 0},
	{"r_unlock",					(cmd_function)r_unlock, 	NO_SCRIPT, 0, 0},
	
	{"S_is_integrity_protected",	S_is_integrity_protected,	1,0,REQUEST_ROUTE},
	{"S_challenge",					S_challenge,				1,0,REQUEST_ROUTE},
	{"S_is_authorized",				S_is_authorized,			1,0,REQUEST_ROUTE},
	{"S_add_path_service_routes",	S_add_path_service_routes,	0,0,REQUEST_ROUTE},
	{"S_add_service_route",			S_add_service_route,		1,0,REQUEST_ROUTE},
	{"S_check_visited_network_id",	S_check_visited_network_id,	1,0,REQUEST_ROUTE},

	{"S_assign_server",				S_assign_server,			1,0,REQUEST_ROUTE},
	{"S_assign_server_unreg",		S_assign_server_unreg,		1,0,REQUEST_ROUTE},
	{"S_update_contacts",			S_update_contacts,			0,0,REQUEST_ROUTE},
	{"S_lookup",					S_lookup,					0,0,REQUEST_ROUTE},

	{"S_term_registered",			S_term_registered,			0,0,REQUEST_ROUTE},
	{"S_term_not_registered",		S_term_not_registered,		0,0,REQUEST_ROUTE},
	{"S_term_unregistered",			S_term_unregistered,		0,0,REQUEST_ROUTE},

	{"S_orig_registered",			S_orig_registered,			0,0,REQUEST_ROUTE},
	{"S_orig_not_registered",		S_orig_not_registered,		0,0,REQUEST_ROUTE},
	{"S_orig_unregistered",			S_orig_unregistered,		0,0,REQUEST_ROUTE},

	{"S_can_subscribe",				S_can_subscribe,			0,0,REQUEST_ROUTE},
	{"S_subscribe",					S_subscribe,				0,0,REQUEST_ROUTE},

	{"S_trans_in_processing",		S_trans_in_processing,				0,0,REQUEST_ROUTE},
	
	
	{"S_mobile_originating",		S_mobile_originating,		0,0,REQUEST_ROUTE},
	
	{"S_originating_barred",		S_originating_barred,		0,0,REQUEST_ROUTE},
	{"S_terminating_barred",		S_terminating_barred,		0,0,REQUEST_ROUTE},
	
	{"S_is_in_dialog",				S_is_in_dialog,				1,0,REQUEST_ROUTE},
	{"S_save_dialog",				S_save_dialog,				1,0,REQUEST_ROUTE},
	{"S_update_dialog",				S_update_dialog,			1,0,REQUEST_ROUTE|ONREPLY_ROUTE},
	{"S_record_route",				S_record_route,				1,0,REQUEST_ROUTE},	
	{"S_is_record_routed",			S_is_record_routed,			1,0,REQUEST_ROUTE},	
	
	{0, 0, 0, 0, 0}
};

/** 
 * Exported parameters. 
 * - name - name fo the S-CSCF
 * <p>
 * - aaa_peer - FQDN of the Diameter Peer (HSS)
 * <p>
 * - auth_data_hash_size - size of the authentication vectors hash table
 * - auth_vector_timeout - time-out for a used vector to expire if no answer received
 * - auth_data_timeout - time-out for a auth vector to be removed when empty
 * - av_request_at_once - how many auth vectors to request at once through MAR
 * - av_request_at_sync - how many auth vectors to request at once through MAR after synchronization
 * <p>
 * - server_assignment_store_data - if to store data on de-registration
 * <p>
 * - user_data_dtd - DTD to check the user data received in SAA (if one from DTD or XSD is specified, it is enough)
 * - user_data_xsd - XSD to check the user data received in SAA (if one from DTD or XSD is specified, it is enough)
 * <p>
 * - registrar_hash_size - size of the registrar hash table
 * - registration_default_expires - default expires interval for registration, if not specified
 * - registration_min_expires - minimum expires interval
 * - registration_max_expires - maximim expires interval
 * - registration_default_algorithm - default algorithm to use for authentication
 * <p>
 * - subscription_default_expires - default expires interval for reg subscriptions, if not specified
 * - subscription_min_expires - minimum expires interval
 * - subscription_max_expires - maximim expires interval
 * <p>
 * - append-branches - if to fork the requests on multiple contacts
 * <p>
 * - dialogs_hash_size - size of the dialogs hash table 
 * - dialogs_expiration_time - default dialogs expiration time
 * <p>
 * - persistency_mode - how to do persistency - 0 none; 1 with files; 2 with db	
 * - persistency_location - where to dump/load the persistency data to/from
 */	
static param_export_t scscf_params[]={ 
	{"name", 							STR_PARAM, &scscf_name},

	{"aaa_peer", 						STR_PARAM, &scscf_aaa_peer},

	{"auth_data_hash_size", 			INT_PARAM, &auth_data_hash_size},
	{"auth_vector_timeout", 			INT_PARAM, &auth_vector_timeout},
	{"auth_data_timeout", 				INT_PARAM, &auth_data_timeout},
	{"av_request_at_once", 				INT_PARAM, &av_request_at_once},
	{"av_request_at_sync", 				INT_PARAM, &av_request_at_sync},

	{"server_assignment_store_data", 	INT_PARAM, &server_assignment_store_data},

	{"user_data_dtd",					STR_PARAM, &scscf_user_data_dtd},
	{"user_data_xsd", 					STR_PARAM, &scscf_user_data_xsd},

	{"registrar_hash_size", 			INT_PARAM, &registrar_hash_size},
	{"registration_default_expires", 	INT_PARAM, &registration_default_expires},
	{"registration_min_expires", 		INT_PARAM, &registration_min_expires},
	{"registration_max_expires", 		INT_PARAM, &registration_max_expires},
	{"registration_default_algorithm",	STR_PARAM, &registration_default_algorithm},

	{"subscription_default_expires",	INT_PARAM, &subscription_default_expires},
	{"subscription_min_expires", 		INT_PARAM, &subscription_min_expires},
	{"subscription_max_expires", 		INT_PARAM, &subscription_max_expires},

	{"append_branches",					INT_PARAM, &append_branches},

	{"dialogs_hash_size", 				INT_PARAM, &scscf_dialogs_hash_size},
	{"dialogs_expiration_time", 		INT_PARAM, &scscf_dialogs_expiration_time},
	
	{"persistency_mode",	 			INT_PARAM, &scscf_persistency_mode},	
	{"persistency_location", 			STR_PARAM, &scscf_persistency_location},
	{"persistency_auth_data_timer",		INT_PARAM, &scscf_persistency_auth_data_timer},
	{0,0,0} 
};

/** module exports */
struct module_exports exports = {
	"scscf", 
	scscf_cmds,
	0,
	scscf_params,
	
	mod_init,		/* module initialization function */
	0,				/* response function*/
	mod_destroy,	/* destroy function */
	0,				/* onbreak function */
	mod_child_init	/* per-child init function */
};


/** Global variables and imported functions */
int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 
												/**< link to the stateless reply function in sl module	*/

struct tm_binds tmb;							/**< Structure with pointers to tm funcs 				*/
struct cdp_binds cdpb;							/**< Structure with pointers to cdp funcs				*/

extern auth_hash_slot_t *auth_data;					/**< authentication vectors hast table 					*/
extern r_hash_slot *registrar;					/**< the S-CSCF registrar								*/
extern r_notification_list *notification_list; 	/**< list of notifications for reg to be sent			*/

extern s_dialog_hash_slot *s_dialogs;			/**< the dialogs hash table								*/


static str s_service_route = {"Service-Route: <",16};
static str s_orig = {"sip:orig@",9};
static str s_sr_end = {";lr>\r\n",6};
static str s_min_expires_s={"Min-Expires: ",13};
static str s_min_expires_e={"\r\n",2};

static str s_record_route_s={"Record-Route: <",15};
static str s_mo = {"sip:mo@",7};
static str s_mt = {"sip:mt@",7};
static str s_record_route_e={";lr>\r\n",6};
/**
 * Builds the Service-Route header from the scscf_name_str
 */
static inline int build_record_service_route()
{
	scscf_service_route.s = pkg_malloc(s_service_route.len+s_orig.len+scscf_name_str.len+s_sr_end.len);
	if (!scscf_service_route.s){
		LOG(L_ERR, "ERR"M_NAME":mod_init: Error allocating %d bytes\n",
			s_service_route.len+s_orig.len+scscf_name_str.len+s_sr_end.len);
		return 0;
	}
	scscf_registration_min_expires.s = pkg_malloc(s_min_expires_s.len+10+s_min_expires_e.len);
	if (!scscf_registration_min_expires.s){
		LOG(L_ERR, "ERR"M_NAME":mod_init: Error allocating %d bytes\n",
			s_min_expires_s.len+10+s_min_expires_e.len);
		return 0;
	}
	scscf_subscription_min_expires.s = pkg_malloc(s_min_expires_s.len+10+s_min_expires_e.len);
	if (!scscf_subscription_min_expires.s){
		LOG(L_ERR, "ERR"M_NAME":mod_init: Error allocating %d bytes\n",
			s_min_expires_s.len+10+s_min_expires_e.len);
		return 0;
	}
	scscf_record_route_mo.s = pkg_malloc(s_record_route_s.len+s_mo.len+scscf_name_str.len+s_record_route_e.len);
	if (!scscf_record_route_mo.s){
		LOG(L_ERR, "ERR"M_NAME":mod_init: Error allocating %d bytes\n",
			s_record_route_s.len+s_mo.len+scscf_name_str.len+s_record_route_e.len);
		return 0;
	}
	scscf_record_route_mt.s = pkg_malloc(s_record_route_s.len+s_mt.len+scscf_name_str.len+s_record_route_e.len);
	if (!scscf_record_route_mt.s){
		LOG(L_ERR, "ERR"M_NAME":mod_init: Error allocating %d bytes\n",
			s_record_route_s.len+s_mt.len+scscf_name_str.len+s_record_route_e.len);
		return 0;
	}
	
	STR_APPEND(scscf_service_route,s_service_route);
	if (scscf_name_str.len>4 && strncasecmp(scscf_name_str.s,"sip:",4)==0){
		STR_APPEND(scscf_service_route,s_orig);
		memcpy(scscf_service_route.s+scscf_service_route.len,scscf_name_str.s+4,
			scscf_name_str.len-4);
		scscf_service_route.len += scscf_name_str.len-4;
	} else {
		STR_APPEND(scscf_service_route,s_orig);
		STR_APPEND(scscf_service_route,scscf_name_str);
	}
	STR_APPEND(scscf_service_route,s_sr_end);
	scscf_service_route_uri.s = scscf_service_route.s + s_service_route.len;
	scscf_service_route_uri.len = scscf_service_route.len - s_service_route.len - s_sr_end.len;

	sprintf(scscf_registration_min_expires.s,"%.*s%d%.*s",
		s_min_expires_s.len,s_min_expires_s.s,
		registration_min_expires,
		s_min_expires_e.len,s_min_expires_e.s);
	scscf_registration_min_expires.len = strlen(scscf_registration_min_expires.s);

	sprintf(scscf_subscription_min_expires.s,"%.*s%d%.*s",
		s_min_expires_s.len,s_min_expires_s.s,
		subscription_min_expires,
		s_min_expires_e.len,s_min_expires_e.s);
	scscf_subscription_min_expires.len = strlen(scscf_subscription_min_expires.s);

	scscf_record_route_mo.len=0;
	STR_APPEND(scscf_record_route_mo,s_record_route_s);
	if (scscf_name_str.len>4 && strncasecmp(scscf_name_str.s,"sip:",4)==0){
		STR_APPEND(scscf_record_route_mo,s_mo);
		memcpy(scscf_record_route_mo.s+scscf_record_route_mo.len,scscf_name_str.s+4,
			scscf_name_str.len-4);
		scscf_record_route_mo.len += scscf_name_str.len-4;
	} else {
		STR_APPEND(scscf_record_route_mo,s_mo);
		STR_APPEND(scscf_record_route_mo,scscf_name_str);
	}
	STR_APPEND(scscf_record_route_mo,s_record_route_e);
	scscf_record_route_mo_uri.s = scscf_record_route_mo.s + s_record_route_s.len;
	scscf_record_route_mo_uri.len = scscf_record_route_mo.len - s_record_route_s.len - s_record_route_e.len;

	scscf_record_route_mt.len=0;
	STR_APPEND(scscf_record_route_mt,s_record_route_s);
	if (scscf_name_str.len>4 && strncasecmp(scscf_name_str.s,"sip:",4)==0){
		STR_APPEND(scscf_record_route_mt,s_mt);
		memcpy(scscf_record_route_mt.s+scscf_record_route_mt.len,scscf_name_str.s+4,
			scscf_name_str.len-4);
		scscf_record_route_mt.len += scscf_name_str.len-4;
	} else {
		STR_APPEND(scscf_record_route_mt,s_mt);
		STR_APPEND(scscf_record_route_mt,scscf_name_str);
	}
	STR_APPEND(scscf_record_route_mt,s_record_route_e);
	scscf_record_route_mt_uri.s = scscf_record_route_mt.s + s_record_route_s.len;
	scscf_record_route_mt_uri.len = scscf_record_route_mt.len - s_record_route_s.len - s_record_route_e.len;


	return 1;
}

/**
 * Initializes the module.
 */
static int mod_init(void)
{
	load_tm_f load_tm;
	load_cdp_f load_cdp;
	callback_singleton=shm_malloc(sizeof(int));
	*callback_singleton=0;
		
	LOG(L_INFO,"INFO:"M_NAME":mod_init: Initialization of module\n");
	/* fix the parameters */
	scscf_name_str.s = scscf_name;
	scscf_name_str.len = strlen(scscf_name);
	if (!build_record_service_route()) goto error;
	scscf_aaa_peer_str.s = scscf_aaa_peer;
	scscf_aaa_peer_str.len = strlen(scscf_aaa_peer);
	
	registration_default_algorithm_s.s = registration_default_algorithm;
	registration_default_algorithm_s.len = strlen(registration_default_algorithm);
	
	/* load the send_reply function from sl module */
    sl_reply = find_export("sl_send_reply", 2, 0);
	if (!sl_reply) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: This module requires sl module\n");
		goto error;
	}
	
//	/* bind to the db module */
//	if ( cscf_db_bind( scscf_db_url ) < 0 ) goto error;
	
	/* bind to the tm module */
	if (!(load_tm = (load_tm_f)find_export("load_tm",NO_SCRIPT,0))) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: Can not import load_tm. This module requires tm module\n");
		goto error;
	}
	if (load_tm(&tmb) == -1)
		goto error;
	/* bind to the cdp module */
	if (!(load_cdp = (load_cdp_f)find_export("load_cdp",NO_SCRIPT,0))) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: Can not import load_cdp. This module requires cdp module\n");
		goto error;
	}
	if (load_cdp(&cdpb) == -1)
		goto error;
	
	
	/* Init the authorization data storage */
	if (!auth_data_init(auth_data_hash_size)) goto error;
	
	if (scscf_persistency_mode!=NO_PERSISTENCY){
		load_snapshot_auth(auth_data);
		if (register_timer(auth_persistency_timer,0,scscf_persistency_auth_data_timer)<0) goto error;
	}
	

	/* register the authentication vectors timer */
	if (register_timer(reg_await_timer,auth_data,10)<0) goto error;
	
	/* init the registrar storage */
	if (!r_storage_init(registrar_hash_size)) goto error;

	/* register the registrar timer */
	if (register_timer(registrar_timer,registrar,10)<0) goto error;

	/* init the registrar notifications */
	if (!r_notify_init()) goto error;

	/* register the registrar notifications timer */
	if (register_timer(notification_timer,notification_list,5)<0) goto error;

	/* init the dialog storage */
	if (!s_dialogs_init(scscf_dialogs_hash_size)){
		LOG(L_ERR, "ERR"M_NAME":mod_init: Error initializing the Hash Table for stored dialogs\n");
		goto error;
	}		

	/* register the dialog timer */
	if (register_timer(dialog_timer,s_dialogs,60)<0) goto error;


	/* don't register response callback as we always set callback per transactions 
	 *  and we're not interested in other responses */
	/*AAAAddResponseHandler(&CxAnswerHandler,0);*/

	
	return 0;
error:
	return -1;
}

extern gen_lock_t* process_lock;		/* lock on the process table */

/**
 * Initializes the module in child.
 */
static int mod_child_init(int rank)
{
	LOG(L_INFO,"INFO:"M_NAME":mod_child_init: Initialization of module in child [%d] \n",
		rank);
	/* don't do anything for main process and TCP manager process */
	if ( rank == PROC_MAIN || rank == PROC_TCP_MAIN )
		return 0;
	
//	/* db child init */
//	cscf_db_init( scscf_db_url, 
//		scscf_db_nds_table,
//		scscf_db_scscf_table,
//		scscf_db_capabilities_table);
	/* init the diameter callback - must be done just once */
	lock_get(process_lock);
	if((*callback_singleton)==0){
		*callback_singleton=1;
		cdpb.AAAAddRequestHandler(CxRequestHandler,NULL);
	}
	lock_release(process_lock);
	/* Init the user data parser */
	if (!parser_init(scscf_user_data_dtd,scscf_user_data_xsd)) return -1;
		
	return 0;
}

/**
 * Destroys the modules 
 */
static void mod_destroy(void)
{
	LOG(L_INFO,"INFO:"M_NAME":mod_destroy: child exit\n");
	auth_data_destroy();
	parser_destroy();
	r_notify_destroy();	
	r_storage_destroy();
	s_dialogs_destroy();	
	pkg_free(scscf_service_route.s);
}




/**
 * Checks if this transaction is already in processing.
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if in processing or #CSCF_RETURN_FALSE if not
 */
int S_trans_in_processing(struct sip_msg* msg, char* str1, char* str2)
{
	unsigned int hash, label;
	if (tmb.t_get_trans_ident(msg,&hash,&label)<0)
		return CSCF_RETURN_FALSE;
	return CSCF_RETURN_TRUE;	
}
