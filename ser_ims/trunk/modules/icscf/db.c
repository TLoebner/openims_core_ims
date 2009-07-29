/**
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
 * Interrogating-CSCF - Database operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "db.h"

#ifdef SER_MOD_INTERFACE
	#include "../../lib/srdb1/db.h"
#else
	#include "../../db/db.h"
#endif
#include "../../mem/shm_mem.h"

#include "mod.h"

static db_func_t dbf;						/**< db function bindings*/
extern char * icscf_db_url;					/**< DB URL */
extern char * icscf_db_nds_table;			/**< NDS table in DB */
extern char * icscf_db_scscf_table;			/**< S-CSCF table in db */
extern char * icscf_db_capabilities_table;	/**< S-CSCF capabilities table in db */

#ifdef SER_MOD_INTERFACE
	static db1_con_t *hdl_db=0;				/**< handle for the database queries */	
#else
	static db_con_t *hdl_db=0;				/**< handle for the database queries */	
#endif	



/**
 *  Bind to the database module.
 * @param db_url - URL of the database
 * @returns 0 on success, -1 on error
 */
int icscf_db_bind(char* db_url)
{
#ifdef SER_MOD_INTERFACE	
	str db_url_str={db_url,strlen(db_url)};
#endif
	
	if (
#ifdef SER_MOD_INTERFACE	
		db_bind_mod(&db_url_str, &dbf) < 0
#else	
		bind_dbmod(db_url, &dbf)
#endif	
		) {
		LOG(L_CRIT, "CRIT:"M_NAME":icscf_db_bind: cannot bind to database module! "
		"Did you forget to load a database module ?\n");
		return -1;
	}
	return 0;
}


/**
 *  Init the database connection 
 * @param db_url - URL of the database
 * @param db_table_nds - name of the NDS table
 * @param db_table_scscf - name of the S-CSCF table
 * @param db_table_capabilities - name of the S-CSCF capabilities table
 * @returns 0 on success, -1 on error
 */
int icscf_db_init(char* db_url,
	char* db_table_nds,
	char* db_table_scscf,
	char* db_table_capabilities)
{
#ifdef SER_MOD_INTERFACE	
	str db_url_str={db_url,strlen(db_url)};
#endif
	
	if (dbf.init==0){
		LOG(L_CRIT, "BUG:"M_NAME":icscf_db_init: unbound database module\n");
		return -1;
	}

#ifdef SER_MOD_INTERFACE	
	hdl_db=dbf.init(&db_url_str);
#else
	hdl_db=dbf.init(db_url);
#endif	
	if (hdl_db==0){
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot initialize database connection\n");
		goto error;
	}	

	return 0;

error:
	if (hdl_db){
		dbf.close(hdl_db);
		hdl_db=0;
	}
	return -1;
}

/**
 *  Close the database connection.
 */
void icscf_db_close()
{
	if (!dbf.close) return;
	if (hdl_db){
		dbf.close(hdl_db);
		hdl_db=0;
	}
}

/**
 * Simply check if the database connection was initialized and connect if not.
 * @param db_hdl - database handle to test
 * @returns 1 if connected, 0 if not
 */
static inline int icscf_db_check_init(
#ifdef SER_MOD_INTERFACE		
		db1_con_t *db_hdl
#else
		db_con_t *db_hdl
#endif		
		)
{
	if (db_hdl) return 1;
	return (icscf_db_init( icscf_db_url,
		icscf_db_nds_table,
		icscf_db_scscf_table,
		icscf_db_capabilities_table)==0);		
}


static str s_trusted_domain={"trusted_domain",14};
/**
 *  Get the NDS list from the database.
 * @param d - array of string to fill with the db contents
 * @returns 1 on success, 0 on error 
 */
int icscf_db_get_nds(str *d[])
{	
#ifdef 	SER_MOD_INTERFACE	
	db_key_t   keys_ret[] = {&s_trusted_domain};
	db1_res_t   * res = 0 ;	
	str db_table_nds_str={icscf_db_nds_table,strlen(icscf_db_nds_table)};
#else		
	db_key_t   keys_ret[] = {s_trusted_domain.s};
	db_res_t   * res = 0 ;	
#endif	
	str s;
	int i;

	if (!icscf_db_check_init(hdl_db))
		goto error;

	DBG("DBG:"M_NAME":icscf_db_get_nds: fetching list of NDS for I-CSCF \n");
#ifdef 	SER_MOD_INTERFACE		
	if (dbf.use_table(hdl_db, &db_table_nds_str)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot select table \"%s\"\n",db_table_nds_str.s);
		goto error;
	}
#else
	if (dbf.use_table(hdl_db, icscf_db_nds_table)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot select table \"%s\"\n",icscf_db_nds_table);
		goto error;
	}	
#endif	
	if (dbf.query(hdl_db, 0, 0, 0, keys_ret, 0, 1, NULL, & res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: db_query failed\n");
		goto error;
	}

	if (res->n == 0) {
		DBG("DBG:"M_NAME":icscf_db_get_nds: I-CSCF has no NDS trusted domains in db\n");
		*d=shm_malloc(sizeof(str));
		if (*d==NULL){
			LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: failed shm_malloc for 0 domains\n");
			goto error;
		}	
		(*d)[0].s=0;
		(*d)[0].len=0;
	}
	else {
		*d=shm_malloc(sizeof(str)*(res->n+1));
		if (*d==NULL){
			LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: failed shm_malloc for %d domains\n",
				res->n);
			goto error;
		}	
		for(i=0;i<res->n;i++){
			s.s = (char*) res->rows[i].values[0].val.string_val;
			s.len = strlen(s.s);
			(*d)[i].s = shm_malloc(s.len);
			if ((*d)[i].s==NULL) {
				LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: failed shm_malloc for %d bytes\n",
					s.len);
				(*d)[i].len = 0;
			}else{
				(*d)[i].len = s.len;
				memcpy((*d)[i].s,s.s,s.len);
			}
		}
		(*d)[res->n].s=0;
		(*d)[res->n].len=0;
	}

	LOG(L_INFO, "INF:"M_NAME":icscf_db_get_nds: Loaded %d trusted domains\n",
		res->n);

	dbf.free_result( hdl_db, res);
	return 1;
error:
	if (res)
		dbf.free_result( hdl_db, res);
	*d=shm_malloc(sizeof(str));
	if (*d==NULL)
		LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_nds: failed shm_malloc for 0 domains\n");
	else {
		(*d)[0].s=0;
		(*d)[0].len=0;
	}
	return 0;
}


static str s_id={"id",2};
static str s_s_cscf_uri={"s_cscf_uri",10};
/**
 *  Get the S-CSCF names from the database and create the S-CSCF set.
 * @param cap - array of scscf_capabilities to fill with the db contents for the S-CSCF names
 * @returns 1 on success, 0 on error 
 */
int icscf_db_get_scscf(scscf_capabilities *cap[])
{
#ifdef 	SER_MOD_INTERFACE	
	db_key_t   keys_ret[] = {&s_id,&s_s_cscf_uri};
	db_key_t   key_ord = &s_id;
	db1_res_t   * res = 0 ;	
	str db_table_scscf_str={icscf_db_scscf_table,strlen(icscf_db_scscf_table)};
#else		
	db_key_t   keys_ret[] = {s_id.s,s_s_cscf_uri.s};
	db_key_t   key_ord = s_id.s;
	db_res_t   * res = 0 ;	
#endif	
	int i;

	*cap = 0;
		
	if (!icscf_db_check_init(hdl_db))
		goto error;

	DBG("DBG:"M_NAME":icscf_db_get_scscf: fetching S-CSCFs \n");
#ifdef 	SER_MOD_INTERFACE		
	if (dbf.use_table(hdl_db, &db_table_scscf_str)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot select table \"%s\"\n",db_table_scscf_str.s);
		goto error;
	}
#else
	if (dbf.use_table(hdl_db, icscf_db_scscf_table)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot select table \"%s\"\n",icscf_db_scscf_table);
		goto error;
	}	
#endif	
	
	if (dbf.query(hdl_db, 0, 0, 0, keys_ret, 0, 2, key_ord, & res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_scscf: db_query failed\n");
		goto error;
	}

	if (res->n == 0) {
		LOG(L_ERR,"ERR:"M_NAME":icscf_db_get_scscf:  no S-CSCFs found\n");
		goto error;
	}
	else {
		*cap = shm_malloc(sizeof(scscf_capabilities)*res->n);
		if (!(*cap)) {
			LOG(L_ERR,"ERR:"M_NAME":icscf_db_get_scscf: Error allocating %d bytes\n",
				sizeof(scscf_capabilities)*res->n);
			goto error;
		}
		memset((*cap),0,sizeof(scscf_capabilities)*res->n);
		for(i=0;i<res->n;i++){
			(*cap)[i].id_s_cscf = res->rows[i].values[0].val.int_val;
			(*cap)[i].scscf_name.len = strlen(res->rows[i].values[1].val.string_val);
			(*cap)[i].scscf_name.s = shm_malloc((*cap)[i].scscf_name.len);
			if (!(*cap)[i].scscf_name.s){
				LOG(L_ERR,"ERR:"M_NAME":icscf_db_get_scscf: Error allocating %d bytes\n",
					(*cap)[i].scscf_name.len);
				(*cap)[i].scscf_name.len=0;
				goto error;
			}
			memcpy((*cap)[i].scscf_name.s,res->rows[i].values[1].val.string_val,
				(*cap)[i].scscf_name.len);
		}
	}

	dbf.free_result( hdl_db, res);
	
	// return the size of scscf set  
	return i;
	
error:
	if (res)
		dbf.free_result( hdl_db, res);
	return 0;
}

static str s_id_s_cscf={"id_s_cscf",9};
static str s_capability={"capability",10};
/**
 *  Get the S-CSCF capabilities from the database and fill the S-CSCF set.
 * @param cap - array of scscf_capabilities to fill with capabilities
 * @returns 1 on success, 0 on error 
 */
int icscf_db_get_capabilities(scscf_capabilities *cap[],int cap_cnt)
{
#ifdef 	SER_MOD_INTERFACE	
	db_key_t   keys_ret[] = {&s_id_s_cscf,&s_capability};
	db_key_t   key_ord = &s_id_s_cscf;
	db1_res_t   * res = 0 ;	
	str db_table_capabilities_str={icscf_db_capabilities_table,strlen(icscf_db_capabilities_table)};	
#else		
	db_key_t   keys_ret[] = {s_id_s_cscf.s,s_capability.s};
	db_key_t   key_ord = s_id_s_cscf.s;
	db_res_t   * res = 0 ;	
#endif		
	int i,j;
	int ccnt=0;
	int cnt;


	if (!icscf_db_check_init(hdl_db))
		goto error;

	DBG("DBG:"M_NAME":icscf_db_get_capabilities: fetching list of Capabilities for I-CSCF\n");

#ifdef 	SER_MOD_INTERFACE		
	if (dbf.use_table(hdl_db, &db_table_capabilities_str)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot select table \"%s\"\n",db_table_capabilities_str.s);
		goto error;
	}
#else
	if (dbf.use_table(hdl_db, icscf_db_capabilities_table)<0) {
		LOG(L_CRIT,"ERR:"M_NAME":icscf_db_init: cannot select table \"%s\"\n",icscf_db_capabilities_table);
		goto error;
	}	
#endif
	
	if (dbf.query(hdl_db, 0, 0, 0, keys_ret, 0, 2, key_ord, & res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":icscf_db_get_capabilities: db_query failed\n");
		goto error;
	}

	if (res->n == 0) {
		DBG("DBG:"M_NAME":icscf_db_get_capabilities: No Capabilites found... not critical...\n");
		return 1;
	}
	else {
		for(i=0;i<cap_cnt;i++){
			cnt = 0;
			for(j=0;j<res->n;j++)
				if (res->rows[j].values[0].val.int_val == (*cap)[i].id_s_cscf)
					cnt++;
			(*cap)[i].capabilities = shm_malloc(sizeof(int)*cnt);
			if (!(*cap)[i].capabilities) {
				LOG(L_ERR,"ERR:"M_NAME":icscf_db_get_capabilities: Error allocating %d bytes\n",
					sizeof(int)*cnt);
				(*cap)[i].cnt=0;
					goto error;
			}			
			cnt=0;
			for(j=0;j<res->n;j++)
				if (res->rows[j].values[0].val.int_val == (*cap)[i].id_s_cscf) {
					(*cap)[i].capabilities[cnt++]=res->rows[j].values[1].val.int_val;
					ccnt++;
				}
			(*cap)[i].cnt=cnt;					
		}
			
	} 
	LOG(L_INFO, "INF:"M_NAME":icscf_db_get_capabilities: Loaded %d capabilities for %d S-CSCFs (%d invalid entries in db)\n",
		ccnt,cap_cnt,res->n-ccnt);
	dbf.free_result( hdl_db, res);
	return 1;
	
error:
	if (res)
		dbf.free_result( hdl_db, res);
	return 0;
}
