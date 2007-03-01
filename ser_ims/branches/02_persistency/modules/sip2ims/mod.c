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
 * SIP-to-IMS Gateway - SER module interfaces
 * 
 * For now, only authentication translation
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "mod.h"

#include "../../sr_module.h"
#include "../../timer.h"
#include "../../locking.h"
#include "../tm/tm_load.h"

#include "gm.h"
#include "db.h"

MODULE_VERSION

static int mod_init(void);
static int mod_child_init(int rank);
static void mod_destroy(void);

/** length of the secret for MD5 */
#define RAND_SECRET_LEN 32
/** random secret for MD5 */
char* sec_rand = 0;

/* parameters storage */
char* db_url="mysql://sip2ims:heslo@localhost/sip2ims";/**< db URL */
char* db_table="credentials";	/**< db table for credentials */
char* sec_param=0;				/**< secret for MD5 */
int   nonce_expire = 300; 		/**< Nonce lifetime */

/** fixed parameter storage */
str secret;

/**
 *  Exported functions.
 * - Gw_MD5_to_AKA() - Translate from an MD5 authorization to a AKA one
 * - Gw_AKA_to_MD5() - Translate from a AKA challenge to a MD5 one
 */
static cmd_export_t sip2ims_cmds[]={
	{"Gw_MD5_to_AKA",	Gw_MD5_to_AKA,	1, 0, REQUEST_ROUTE},	
	{"Gw_AKA_to_MD5",	Gw_AKA_to_MD5,	0, 0, ONREPLY_ROUTE},	
	{0, 0, 0, 0, 0}
}; 

/**
 *  Exported parameters.
 * - db_url - URL of the credentials database 
 * - db_table - name of the credentials table
 * - secret - MD5 secret
 * - nonce_expire - MD5 nonce expiration period
 */	
static param_export_t sip2ims_params[]={ 
	{"db_url", STR_PARAM, &db_url},
	{"db_table", STR_PARAM, &db_table},
	{"secret", STR_PARAM, &sec_param},	
	{"nonce_expire",    INT_PARAM, &nonce_expire   },	
	{0,0,0} 
};

/** module exports */
struct module_exports exports = {
	"sip2ims", 
	sip2ims_cmds,
	0,
	sip2ims_params,
	
	mod_init,		/* module initialization function */
	0,				/* response function*/
	mod_destroy,	/* destroy function */
	0,				/* onbreak function */
	mod_child_init	/* per-child init function */
};


/* Global variables and imported functions */

int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 
										/**< link to the stateless reply function in sl module */

struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/


/*
 * Secret parameter was not used so we generate a random value here.
 */
static inline int generate_random_secret(void)
{
	int i;

	sec_rand = (char*)pkg_malloc(RAND_SECRET_LEN);
	if (!sec_rand) {
		LOG(L_ERR, "generate_random_secret(): No memory left\n");		
		return -1;
	}

	srandom(time(0));

	for(i = 0; i < RAND_SECRET_LEN; i++) {
		sec_rand[i] = 32 + (int)(95.0 * rand() / (RAND_MAX + 1.0));
	}

	secret.s = sec_rand;
	secret.len = RAND_SECRET_LEN;

	     /*	DBG("Generated secret: '%.*s'\n", secret.len, secret.s); */

	return 0;
}

/**
 * Initializes the module
 */
static int mod_init(void)
{
	load_tm_f load_tm;
	LOG(L_INFO,"INFO:"M_NAME":mod_init: Initialization of module\n");
	
	/* If the parameter was not used */
	if (sec_param == 0) {
		     /* Generate secret using random generator */
		if (generate_random_secret() < 0) {
			LOG(L_ERR, "mod_init(): Error while generating random secret\n");
			return -3;
		}
	} else {
		     /* Otherwise use the parameter's value */
		secret.s = sec_param;
		secret.len = strlen(secret.s);
	}	
	
	/* bind to db */
	if (sip2ims_db_bind(db_url)<0) goto error;
	
	/* load the send_reply function from sl module */
    sl_reply = find_export("sl_send_reply", 2, 0);
	if (!sl_reply) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: This module requires sl module\n");
		goto error;
	}
	
	/* bind to the db module */
	//if ( sip2ims_db_bind( db_url ) < 0 ) goto error;
	
	/* bind to the tm module */
	if (!(load_tm = (load_tm_f)find_export("load_tm",NO_SCRIPT,0))) {
		LOG(L_ERR, "ERR"M_NAME":mod_init: Can not import load_tm. This module requires tm module\n");
		goto error;
	}
	if (load_tm(&tmb) == -1)
		goto error;
	
	
	return 0;
error:
	return -1;
}

/**
 * Initializes the module in childs
 */
static int mod_child_init(int rank)
{
	LOG(L_INFO,"INFO:"M_NAME":mod_init: Initialization of module in child [%d] \n",
		rank);
	/* don't do anything for main process and TCP manager process */
	if ( rank == PROC_MAIN || rank == PROC_TCP_MAIN )
		return 0;
	
	/* db child init */
	sip2ims_db_init( db_url ,db_table);
				
	return 0;
}

/**
 * Destroys the module
 */
static void mod_destroy(void)
{
	LOG(L_INFO,"INFO:"M_NAME":mod_destroy: child exit\n");	
}





