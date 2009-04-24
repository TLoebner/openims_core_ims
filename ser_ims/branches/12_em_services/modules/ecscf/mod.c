/**
 * \file
 * 
 * emergency-CSCF - SER module interface
 * 
 * Scope:
 * - Exports parameters and functions
 * - Initialization functions
 * 
 *  \author Ancuta Onofrei ancuta_onofrei -at- yahoo dot com
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
#include "../dialog/dlg_mod.h"
#include "../cdp/cdp_load.h"
#include <lost/client.h>

#include "dlg_state.h"
#include "lrf.h"
#include "route.h"

MODULE_VERSION

static int mod_init(void);
static int mod_child_init(int rank);
static void mod_destroy(void);

/* parameters storage */
char* ecscf_name="sip:ecscf.open-ims.test:7060";	/**< SIP URI of this E-CSCF */
int ecscf_dialogs_hash_size=1024;			/**< the size of the hash table for dialogs			*/
int ecscf_dialogs_expiration_time=3600;		/**< expiration time for a dialog					*/
int ecscf_dialogs_enable_release=1;			/**< if to enable dialog release					*/
int ecscf_min_se=90;						/**< Minimum session-expires accepted value			*/
int* ecscf_dialog_count = 0;				/**< Counter for saved dialogs						*/
int ecscf_max_dialog_count=20000;			/**< Maximum number of dialogs						*/ 
gen_lock_t* ecscf_dialog_count_lock=0; 		/**< Lock for the dialog counter					*/
int * shutdown_singleton;				/**< Shutdown singleton 	*/
char* lrf_sip_uri = "sip:lrf.open-ims.test:8060";	/**< sip uri or the LRF>*/
str lrf_sip_uri_str={NULL, 0};

str ecscf_name_str;					/**< SIP URI of the node>*/
str ecscf_record_route_mo;					/**< Record-route for originating case 				*/
str ecscf_record_route_mo_uri;				/**< URI for Record-route originating				*/ 
str ecscf_record_route_mt;					/**< Record-route for terminating case 				*/
str ecscf_record_route_mt_uri;				/**< URI for Record-route terminating				*/


int E_trans_in_processing(struct sip_msg* msg, char* str1, char* str2);

/** 
 * Exported functions.
 * int E_trans_in_processing(struct sip_msg* msg, char* str1, char* str2)
 * int E_is_in_dialog(struct sip_msg* msg, char* str1, char* str2)
 * E_save_dialog
 * E_update_dialog
 * E_drop_dialog
 * E_get_psap - checks if there is a location information 
 * and if so sends a LoST request and parses the LoST response, sets the psap_uri to the dialog information
 * E_fwd_to_psap - adds Route header containing the psap_uri, modifies the R-URI
 * E_add_record_route - add a Record-Route header containing the SIP URI of the ecscf for mobile orig
 */
static cmd_export_t ecscf_cmds[]={
	{"E_is_in_dialog",				E_is_in_dialog, 			1, 0, REQUEST_ROUTE},
	{"E_is_anonymous_user",				E_is_anonymous_user, 			0, 0, REQUEST_ROUTE},
	{"E_trans_in_processing",			E_trans_in_processing, 			0, 0, REQUEST_ROUTE},
	{"E_save_dialog",				E_save_dialog,				2, 0, REQUEST_ROUTE},
	{"E_update_dialog",				E_update_dialog, 			1, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{"E_drop_dialog",				E_drop_dialog, 				1, 0, REQUEST_ROUTE|ONREPLY_ROUTE|FAILURE_ROUTE},
	{"E_get_location",				E_get_location,				1, 0, REQUEST_ROUTE},
	{"E_query_LRF",					E_query_LRF,				1, 0, REQUEST_ROUTE},
	{"E_add_rr",					E_add_record_route,			0, 0, REQUEST_ROUTE},
	{"E_del_ESQK_info",				E_del_ESQK_info,			0, 0, REQUEST_ROUTE},
	{0, 0, 0, 0, 0}
}; 

/** 
 * Exported parameters.
 * - name - name of the E-CSCF
 */	
static param_export_t ecscf_params[]={ 
	{"name", 			STR_PARAM, 		&ecscf_name},
	{"dialogs_hash_size",		INT_PARAM,		&ecscf_dialogs_hash_size},
	{"dialogs_expiration_time",	INT_PARAM,		&ecscf_dialogs_expiration_time},
	{"dialogs_enable_release",	INT_PARAM,		&ecscf_dialogs_enable_release},
	{"max_dialog_count",		INT_PARAM,		&ecscf_max_dialog_count},
	{"min_se", 			INT_PARAM, 		&ecscf_min_se},
	{"lrf_sip_uri",			STR_PARAM, 		&lrf_sip_uri},
	{0,0,0} 
};

/** module exports */
struct module_exports exports = {
	"ecscf", 
	ecscf_cmds,
	0,
	ecscf_params,
	
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
dlg_func_t dialogb;								/**< Structure with pointers to dialog funcs			*/
extern e_dialog_hash_slot *e_dialogs;						/**< the dialogs hash table				*/


static str path_str_s={"Path: <",7};
static str path_str_1={"sip:term@",9};
static str path_str_e={";lr>\r\n",6};

static str s_record_route_s={"Record-Route: <",15};
static str s_mo = {"sip:mo@",7};
static str s_mt = {"sip:mt@",7};
static str s_record_route_e={";lr>\r\n",6};

/**
 * Fix the configuration parameters.
 */
int fix_parameters()
{
	ecscf_name_str.s = ecscf_name;
	ecscf_name_str.len = strlen(ecscf_name);

	lrf_sip_uri_str.s = lrf_sip_uri;
	lrf_sip_uri_str.len = strlen(lrf_sip_uri);
	
	LOG(L_DBG, "DBG:"M_NAME":fix_parameters: lrf uri: %.*s\n", lrf_sip_uri_str.len, lrf_sip_uri_str.s);

	/* Record-routes */
	ecscf_record_route_mo.s = pkg_malloc(s_record_route_s.len+s_mo.len+ecscf_name_str.len+s_record_route_e.len);
	if (!ecscf_record_route_mo.s){
		LOG(L_ERR, "ERR"M_NAME":fix_parameters: Error allocating %d bytes\n",
			s_record_route_s.len+s_mo.len+ecscf_name_str.len+s_record_route_e.len);
		return 0;
	}
	ecscf_record_route_mt.s = pkg_malloc(s_record_route_s.len+s_mt.len+ecscf_name_str.len+s_record_route_e.len);
	if (!ecscf_record_route_mt.s){
		LOG(L_ERR, "ERR"M_NAME":fix_parameters: Error allocating %d bytes\n",
			s_record_route_s.len+s_mt.len+ecscf_name_str.len+s_record_route_e.len);
		return 0;
	}
	
	ecscf_record_route_mo.len=0;
	STR_APPEND(ecscf_record_route_mo,s_record_route_s);
	if (ecscf_name_str.len>4 && strncasecmp(ecscf_name_str.s,"sip:",4)==0){
		STR_APPEND(ecscf_record_route_mo,s_mo);
		memcpy(ecscf_record_route_mo.s+ecscf_record_route_mo.len,ecscf_name_str.s+4,
			ecscf_name_str.len-4);
		ecscf_record_route_mo.len += ecscf_name_str.len-4;
	} else {
		STR_APPEND(ecscf_record_route_mo,s_mo);
		STR_APPEND(ecscf_record_route_mo,ecscf_name_str);
	}
	STR_APPEND(ecscf_record_route_mo,s_record_route_e);
	ecscf_record_route_mo_uri.s = ecscf_record_route_mo.s + s_record_route_s.len;
	ecscf_record_route_mo_uri.len = ecscf_record_route_mo.len - s_record_route_s.len - s_record_route_e.len;

	ecscf_record_route_mt.len=0;
	STR_APPEND(ecscf_record_route_mt,s_record_route_s);
	if (ecscf_name_str.len>4 && strncasecmp(ecscf_name_str.s,"sip:",4)==0){
		STR_APPEND(ecscf_record_route_mt,s_mt);
		memcpy(ecscf_record_route_mt.s+ecscf_record_route_mt.len,ecscf_name_str.s+4,
			ecscf_name_str.len-4);
		ecscf_record_route_mt.len += ecscf_name_str.len-4;
	} else {
		STR_APPEND(ecscf_record_route_mt,s_mt);
		STR_APPEND(ecscf_record_route_mt,ecscf_name_str);
	}
	STR_APPEND(ecscf_record_route_mt,s_record_route_e);
	ecscf_record_route_mt_uri.s = ecscf_record_route_mt.s + s_record_route_s.len;
	ecscf_record_route_mt_uri.len = ecscf_record_route_mt.len - s_record_route_s.len - s_record_route_e.len;

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
		LOG(L_ERR, "ERR:"M_NAME":mod_init:  Can not import bind_dlg_mod. This module requires dialog module\n");
		return -1;
	}
	if (load_dlg(&dialogb) != 0) {
		return -1;
	}

	/* init the dialog storage */
	if (!e_dialogs_init(ecscf_dialogs_hash_size)){
		LOG(L_ERR, "ERR"M_NAME":mod_init: Error initializing the Hash Table for stored dialogs\n");
		goto error;
	}		
	ecscf_dialog_count = shm_malloc(sizeof(int));
	*ecscf_dialog_count = 0;
	ecscf_dialog_count_lock = lock_alloc();
	ecscf_dialog_count_lock = lock_init(ecscf_dialog_count_lock);

	/* register the dialog timer */
	if (register_timer(dialog_timer,e_dialogs,60)<0) goto error;

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
		e_dialogs_destroy();
        	lock_get(ecscf_dialog_count_lock);
	        shm_free(ecscf_dialog_count);
        	lock_destroy(ecscf_dialog_count_lock);
	}
	
}


/**
 * Checks if the transaction is in processing.
 * @param msg - the SIP message to check
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if the transaction is already in processing, #CSCF_RETURN_FALSE if not
 */
int E_trans_in_processing(struct sip_msg* msg, char* str1, char* str2)
{
	unsigned int hash, label;
	if (tmb.t_get_trans_ident(msg,&hash,&label)<0)
		return CSCF_RETURN_FALSE;
	return CSCF_RETURN_TRUE;	
}

