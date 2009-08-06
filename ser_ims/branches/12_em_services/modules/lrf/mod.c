/*
 * Copyright (C) 2008-2009 FhG Fokus
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
 * author Ancuta Onofrei, 
 * 	email andreea dot ancuta dot onofrei -at- fokus dot fraunhofer dot de
 */
/**
 * Location Routing Function - SER module interface
 * 
 * Scope:
 * - Exports parameters and functions
 * - Initialization functions
 * 
 */
 

#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mod.h"

#include "../../db/db.h"
#include "../../sr_module.h"
#include "../../socket_info.h"
#include "../../timer.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"
#include "../dialog/dlg_mod.h"
#include <lost/client.h>

#include "lost.h"
#include "user_data.h"
#include "dlg_state.h"

MODULE_VERSION

static int mod_init(void);
static int mod_child_init(int rank);
static void mod_destroy(void);

extern user_d_hash_slot * user_datas;

/* parameters storage */
int user_d_hash_size=1024;
char* lrf_name="sip:lrf.open-ims.test:8060";	/**< SIP URI of this LRF */
char* local_psap_uri="sip:bob@open-ims.test";	/**< SIP URI of the local default PSAP */
int using_lost_srv = 1;				/* by default, a LoST server will be contacted*/
char * lost_server_url= "http://lost.open-ims.test:8180/lost/LoSTServlet";
str lrf_name_str={0,0};				
lost_server_info lost_server;			/**< info from the http URL of the lost server>*/
/*prefix of this LRF server, used to forward the SUBSCRIBE from the PSAP 
  to the appropriate LRF, using the ESQK including this prefix*/
char * esqk_prefix = "123";
str esqk_prefix_str = {0,0};
str local_psap_uri_str = {0,0};
int * shutdown_singleton;				/**< Shutdown singleton 	*/
dlg_func_t dialogb;								/**< Structure with pointers to dialog funcs			*/
extern lrf_dialog_hash_slot *lrf_dialogs;	/**< the dialogs hash table				*/
int lrf_dialogs_hash_size=1024;			/**< the size of the hash table for dialogs			*/
int lrf_dialogs_expiration_time=3600;		/**< expiration time for a dialog					*/
//int lrf_dialogs_enable_release=1;			/**< if to enable dialog release					*/
int* lrf_dialog_count = 0;				/**< Counter for saved dialogs						*/
int lrf_max_dialog_count=20000;			/**< Maximum number of dialogs						*/ 
gen_lock_t* l_dialog_count_lock=0; 		/**< Lock for the dialog counter					*/


int LRF_trans_in_processing(struct sip_msg* msg, char* str1, char* str2);

int LRF_get_psap(struct sip_msg* msg, char*str1, char*str2);

/** 
 * Exported functions.
 * LRF_get_psap - query the lost server about the appropriate PSAP URI
 */
static cmd_export_t lrf_cmds[]={
	{"LRF_alloc_user_data",			LRF_alloc_user_data,			0, 0, REQUEST_ROUTE},
	{"LRF_del_user_data",			LRF_del_user_data,			0, 0, REQUEST_ROUTE},
	{"LRF_has_loc",				LRF_has_loc,				0, 0, REQUEST_ROUTE},
	{"LRF_save_user_loc",			LRF_save_user_loc,			0, 0, REQUEST_ROUTE},
	{"LRF_get_psap",			LRF_get_psap,				0, 0, REQUEST_ROUTE},
	{"LRF_call_query_resp",			LRF_call_query_resp,			0, 0, REQUEST_ROUTE},
	{"LRF_trans_in_processing",		LRF_trans_in_processing,		0, 0, REQUEST_ROUTE},
	{"LRF_is_in_dialog",			LRF_is_in_dialog, 			1, 0, REQUEST_ROUTE},
	{"LRF_save_dialog",			LRF_save_dialog, 				1, 0, REQUEST_ROUTE},
	{"LRF_update_dialog",			LRF_update_dialog, 			1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"LRF_drop_dialog",			LRF_drop_dialog, 				1, 0, REQUEST_ROUTE|ONREPLY_ROUTE|FAILURE_ROUTE},
	{0, 0, 0, 0, 0}
}; 

/** 
 * Exported parameters.
 * - name - name of the LRF node
 */	
static param_export_t lrf_params[]={ 
	{"name", 			STR_PARAM, 		&lrf_name},
	{"lost_server",			STR_PARAM, 		&lost_server_url},
	{"esqk_prefix",			STR_PARAM, 		&esqk_prefix},
	{"default_psap",		STR_PARAM, 		&local_psap_uri},
	{"using_lost_srv",		INT_PARAM, 		&using_lost_srv},
	{"dialogs_hash_size",		INT_PARAM,		&lrf_dialogs_hash_size},
	{"dialogs_expiration_time",	INT_PARAM,		&lrf_dialogs_expiration_time},
	//{"dialogs_enable_release",	INT_PARAM,		&lrf_dialogs_enable_release},
	{"max_dialog_count",		INT_PARAM,		&lrf_max_dialog_count},
	{0,0,0} 
};

/** module exports */
struct module_exports exports = {
	"lrf", 
	lrf_cmds,
	0,
	lrf_params,
	mod_init,			/* module initialization function */
	0,				/* response function*/
	mod_destroy,			/* destroy function */
	0,				/* onbreak function */
	mod_child_init			/* per-child init function */
};


/* Global variables and imported functions */
int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 
										/**< link to the stateless reply function in sl module */

struct tm_binds tmb;            						/**< Structure with pointers to tm funcs 		*/

/**
 * Fix the configuration parameters.
 */
int fix_parameters()
{
	char * car, *last_car;
	str port;
	
	lrf_name_str.s = lrf_name;
	lrf_name_str.len = strlen(lrf_name);

	esqk_prefix_str.s = esqk_prefix;
	esqk_prefix_str.len = strlen(esqk_prefix);

	local_psap_uri_str.s = local_psap_uri;
	local_psap_uri_str.len = strlen(local_psap_uri);


	if(!using_lost_srv)
		return 1;

	if(strncmp(lost_server_url, "http://", 7) == 0){
		lost_server.type   = HTTP_TYPE;
		lost_server.host.s = lost_server_url;
		lost_server.host.len = strlen(lost_server_url);
		car = lost_server_url+7;
		lost_server.port   = 80;
	}else if(strncmp(lost_server_url, "https://", 8) == 0){
		lost_server.type   = HTTPS_TYPE;
		lost_server.host.s = lost_server_url;
		lost_server.host.len = strlen(lost_server_url);
		car = lost_server_url+8;
		lost_server.port   = 433;
	}else {
		LOG(L_ERR, "ERR"M_NAME":fix_parameters: invalid URL for the lost server %s\n",
				lost_server_url);
		return 0;
	}
			
	car = strchr(car, ':');
	if(car != NULL){
		port.s = car+1;

		if((last_car = strchr(car, '/')) == NULL)
			port.len = strlen(car+1);
		else
			port.len = last_car - car-1;
	
		LOG(L_DBG, "DBG:"M_NAME":fix_parameters: string of the lost server port: %.*s\n",port.len, port.s); 
	
		if(str2int(&port, &lost_server.port)){
			LOG(L_ERR, "ERR:"M_NAME":fix_parameters: invalid port number for the lost server %s\n",
					lost_server_url);
			return 0;
		}
	}

	LOG(L_DBG, "DBG:"M_NAME":fix_parameters: lost server host: %.*s port: %u\n", 
			lost_server.host.len, lost_server.host.s, lost_server.port);
	
	return 1;
}


/**
 * Initializes the module.
 */
static int mod_init(void)
{
	load_tm_f load_tm;
	bind_dlg_mod_f load_dlg;
			
	LOG(L_INFO,"INFO:"M_NAME":mod_init: Initialization of module\n");
	shutdown_singleton=shm_malloc(sizeof(int));
	*shutdown_singleton=0;
	
	
	/* fix the parameters */
	if (!fix_parameters()) goto error;

	/* load the send_reply function from sl module */
    	sl_reply = find_export("sl_send_reply", 2, 0);
	if (!sl_reply) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: This module requires sl module\n");
		goto error;
	}
	
	/* bind to the tm module */
	if (!(load_tm = (load_tm_f)find_export("load_tm",NO_SCRIPT,0))) {
		LOG(L_ERR, "ERR:"M_NAME":mod_init: Can not import load_tm. This module requires tm module\n");
		goto error;
	}
	if (load_tm(&tmb) == -1)
		goto error;

	/* bind to the dialog module */
	load_dlg = (bind_dlg_mod_f)find_export("bind_dlg_mod", -1, 0);
	if (!load_dlg) {
		LOG(L_ERR, "ERR"M_NAME":mod_init:  Can not import bind_dlg_mod. This module requires dialog module\n");
		return -1;
	}
	if (load_dlg(&dialogb) != 0) {
		return -1;
	}

	if (using_lost_srv && init_lost_lib()){
		LOG(L_ERR, "ERR:"M_NAME":mod_init: Error initializing the LoST library\n");
		goto error;
	}
	
	if(!init_lrf_user_data(user_d_hash_size)){
		LOG(L_ERR, "ERR:"M_NAME":mod_init: Error initializing the user data\n");
		goto error;
	}
	/* init the dialog storage */
	if (!lrf_dialogs_init(lrf_dialogs_hash_size)){
		LOG(L_ERR, "ERR"M_NAME":mod_init: Error initializing the Hash Table for stored dialogs\n");
		goto error;
	}		
	lrf_dialog_count = shm_malloc(sizeof(int));
	*lrf_dialog_count = 0;
	l_dialog_count_lock = lock_alloc();
	l_dialog_count_lock = lock_init(l_dialog_count_lock);

	/* register the user datas timer */
	if (register_timer(user_data_timer, user_datas,60)<0) goto error;

	/* register the dialog timer */
	if (register_timer(dialog_timer,lrf_dialogs,60)<0) goto error;


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
	LOG(L_INFO,"INFO:"M_NAME":mod_init: Initialization of module in child [%d] \n",
		rank);
	/* don't do anything for main process and TCP manager process */
	if ( rank == PROC_MAIN || rank == PROC_TCP_MAIN )
		return 0;
			
	return 0;
}

extern gen_lock_t* process_lock;		/* lock on the process table */

/**
 * Destroys the module.
 */
static void mod_destroy(void)
{
	int do_destroy=0;
	LOG(L_INFO,"INFO:"M_NAME":mod_destroy: child exit\n");
	
	lock_get(process_lock);
	if((*shutdown_singleton)==0){
		*shutdown_singleton=1;
		do_destroy=1;
	}
	lock_release(process_lock);

	if (do_destroy){
		/* Then nuke it all */	
		destroy_lrf_user_data();	
		end_lost_lib();
		lrf_dialogs_destroy();
	        lock_get(l_dialog_count_lock);
        	shm_free(lrf_dialog_count);
	        lock_destroy(l_dialog_count_lock);
	}
	
}


/**
 * Checks if the transaction is in processing.
 * @param msg - the SIP message to check
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if the transaction is already in processing, #CSCF_RETURN_FALSE if not
 */
int LRF_trans_in_processing(struct sip_msg* msg, char* str1, char* str2)
{
	unsigned int hash, label;
	if (tmb.t_get_trans_ident(msg,&hash,&label)<0)
		return CSCF_RETURN_FALSE;
	return CSCF_RETURN_TRUE;	
}

