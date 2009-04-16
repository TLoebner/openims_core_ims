/**
 * \file
 * 
 * Location Routing Function - SER module interface
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
#include "../cdp/cdp_load.h"
#include <lost/client.h>

#include "lost.h"
#include "user_data.h"

MODULE_VERSION

static int mod_init(void);
static int mod_child_init(int rank);
static void mod_destroy(void);

extern user_d_hash_slot * user_datas;

/* parameters storage */
int user_d_hash_size=1024;
char* lrf_name="sip:lrf.open-ims.test:8060";	/**< SIP URI of this LRF */
char* local_psap_uri="sip:bob@open-ims.test";	/**< SIP URI of the local default PSAP */
char * lost_server_url= "http://lost.open-ims.test:8180/lost/LoSTServlet";
str lrf_name_str={0,0};				
lost_server_info lost_server;			/**< info from the http URL of the lost server>*/
/*prefix of this LRF server, used to forward the SUBSCRIBE from the PSAP 
  to the appropriate LRF, using the ESQK including this prefix*/
char * esqk_prefix = "123";
str esqk_prefix_str = {0,0};
str local_psap_uri_str = {0,0};
int * shutdown_singleton;				/**< Shutdown singleton 	*/

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
	LOG(L_DBG, "DBG:"M_NAME":fix_parameters: lost server host: %.*s port: %u\n", 
			lost_server.host.len, lost_server.host.s, lost_server.port);
	esqk_prefix_str.s = esqk_prefix;
	esqk_prefix_str.len = strlen(esqk_prefix);

	local_psap_uri_str.s = local_psap_uri;
	local_psap_uri_str.len = strlen(local_psap_uri);

	return 1;
}


/**
 * Initializes the module.
 */
static int mod_init(void)
{
	load_tm_f load_tm;
			
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

	/* register the user datas timer */
	if (register_timer(user_data_timer, user_datas,60)<0) goto error;

	if (init_lost_lib()){
		LOG(L_ERR, "ERR:"M_NAME":mod_init: Error initializing the LoST library\n");
		goto error;
	}
	
	if(!init_lrf_user_data(user_d_hash_size)){
		LOG(L_ERR, "ERR:"M_NAME":mod_init: Error initializing the user data\n");
		goto error;
	}
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

	//TODO:destroy the map
	if (do_destroy){
		/* Then nuke it all */	
		destroy_lrf_user_data();	
		end_lost_lib();
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

