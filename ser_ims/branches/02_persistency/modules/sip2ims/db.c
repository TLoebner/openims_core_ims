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
 * SIP-to-IMS Gateway - Database Operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "db.h"

#include "../../db/db.h"
#include "../../mem/mem.h"

#include "mod.h"

static db_func_t dbf;		/**< DB bindings */
extern char * db_url;		/**< URL of the DB connection */
extern char * db_table;		/**< name of the credentials table */

static db_con_t *hdl=0;		/**< local database handle */



/** 
 * Bind to the database module.
 * @param db_url - URL of the DB. The first part (like "mysql://") is used to identify
 * the right module to load for database access.
 * @returns 0 on success, -1 on failure
 */
int sip2ims_db_bind(char* db_url)
{
	if (bind_dbmod(db_url, &dbf)) {
		LOG(L_CRIT, "CRIT:"M_NAME":sip2ims_db_bind: cannot bind to database module! "
		"Did you forget to load a database module ?\n");
		return -1;
	}
	return 0;
}


/**
 *  Init the database connection.
 * @param db_url - URL of the DB
 * @param db_table - name of the credentials table
 * @returns 0 on success, -1 on error
 */
int sip2ims_db_init(char* db_url,
	char* db_table)
{
	if (dbf.init==0){
		LOG(L_CRIT, "BUG:"M_NAME":sip2ims_db_init: unbound database module\n");
		return -1;
	}
	hdl=dbf.init(db_url);
	if (hdl==0){
		LOG(L_CRIT,"ERR:"M_NAME":sip2ims_db_init: cannot initialize database "
			"connection\n");
		goto error;
	}	
	if (dbf.use_table(hdl, db_table)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":sip2ims_db_init: cannot select table \"%s\"\n",db_table);
		goto error;
	}
	return 0;

error:
	if (hdl){
		dbf.close(hdl);
		hdl=0;
	}
	return -1;
}
/**
 *  Close the database connection.
 */
void sip2ims_db_close()
{
	if (hdl&& dbf.close){
		dbf.close(hdl);
		hdl=0;
	}
}
/**
 *  Simply check if the database connection was initialized.
 * If not, sip2ims_db_init() is called to attempt reconnection.
 * @param  db_hdl - the database handle to check
 * @returns 1 if connected, 0 if not and connection failed
 */
static inline int sip2ims_db_check_init(db_con_t *db_hdl)
{
	if (db_hdl) return 1;
	return (sip2ims_db_init( db_url,	db_table)==0);		
}

/** 
 * Get the Password from the database for a specific user.
 * @param username - username to look for in the auth_username field
 * @param domain - realm to look for in the realm field
 * @param pass - will be filled with the password value
 * @returns 1 on success, 0 on error
 */
int sip2ims_db_get_password(str username,str domain,str *pass)
{
	db_key_t   keys_cmp[] = {"auth_username","realm"};
	db_op_t    keys_op[] = {OP_EQ,OP_EQ};
	db_val_t   keys_val[2];
	
	db_key_t   keys_ret[] = {"password"};
	db_res_t   * res = 0 ;	


	pass->s=0;
	pass->len=0;

	if (!sip2ims_db_check_init(hdl))
		goto error;

	DBG("DBG:"M_NAME":sip2ims_db_get_password: fetching password\n");
	keys_val[0].nul=0;
	keys_val[0].type = DB_STR;
	keys_val[0].val.str_val = username;
	keys_val[1].nul=0;
	keys_val[1].type = DB_STR;
	keys_val[1].val.str_val = domain;
	

	if (dbf.query(hdl, keys_cmp, keys_op, keys_val, keys_ret, 2, 1, NULL, & res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_db_get_password: db_query failed\n");
		goto error;
	}

	if (res->n == 0) {
		DBG("DBG:"M_NAME":sip2ims_db_get_password: No such user in db\n");
		goto error;
	}
	else {
		pass->len = strlen(res->rows[0].values[0].val.string_val);
		pass->s = pkg_malloc(pass->len);
		if (!pass->s){
				LOG(L_ERR, "ERR:"M_NAME":sip2ims_db_get_password: failed pkg_malloc for %d bytes\n",
					pass->len);
				pass->len=0;
				goto error;	
		}
		memcpy(pass->s,res->rows[0].values[0].val.string_val,pass->len);
	}

	dbf.free_result( hdl, res);
	return 1;
error:
	if (res)
		dbf.free_result( hdl, res);
	return 0;
}


