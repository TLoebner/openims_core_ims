/*
 * $Id$
 *
 * Copyright (C) 2004-2007 FhG Fokus
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
 * S-CSCF persistency operations
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 * Database persistency implemented by Mário Ferreira (PT INOVAÇÃO)
 * est-m-aferreira at ptinovacao.pt
 *
 */

#include "s_persistency.h"

extern persistency_mode_t scscf_persistency_mode;/**< the type of persistency					*/
extern char* scscf_persistency_location;		/**< where to dump the persistency data 		*/ 


extern auth_hash_slot_t *auth_data;				/**< Authentication vector hash table 			*/
extern int auth_data_hash_size;					/**< authentication vector hash table size 		*/

extern db_con_t* scscf_db; /**< Database connection handle */
extern db_func_t scscf_dbf;	/**< Structure with pointers to db functions */
extern int* auth_snapshot_version;
extern int* auth_step_version;
extern int* dialogs_snapshot_version;
extern int* dialogs_step_version;
extern int* registrar_snapshot_version;
extern int* registrar_step_version;
extern gen_lock_t* db_lock; 

int s_dump(bin_data* x, persistency_mode_t mode, char* location, char* prepend_fname);
int s_load(bin_data *x,int mode,char *location,char* prepend_fname);

/**
 * Creates a snapshots of the authorization data and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_authdata()
{
	bin_data x;
	auth_userdata *aud;
	int i;	
	if (!bin_alloc(&x,256)) goto error;		
	for(i=0;i<auth_data_hash_size;i++){
		auth_data_lock(i);
		aud = auth_data[i].head;
		while(aud){
			if (!bin_encode_auth_userdata(&x,aud)) goto error;
			aud = aud->next;
		}
		auth_data_unlock(i);
	}
	bin_print(&x);
	i=s_dump(&x,scscf_persistency_mode,scscf_persistency_location,"authdata");
	//i = bin_dump(&x,scscf_persistency_mode,scscf_persistency_location,"authdata");		
	bin_free(&x);
	return i;
error:
	return 0;
}

int bin_dump_to_db(bin_data *x, char* prepend_fname);

int s_dump(bin_data* x, persistency_mode_t mode, char* location, char* prepend_fname){
	
	switch (mode){
		case NO_PERSISTENCY:
			LOG(L_ERR,"ERR:"M_NAME":s_dump: Snapshot done but persistency was disabled...\n");
			return 0;
		case WITH_FILES:
			return bin_dump_to_file(x,location,prepend_fname);
		case WITH_DATABASE_BULK:
			return bin_dump_to_db(x, prepend_fname);
		case WITH_DATABASE_CACHE:
			LOG(L_ERR,"ERR:"M_NAME":s_dump: Snapshot done but WITH_DATABASE_CACHE not implemented...\n");
			return 0;
		default:
			LOG(L_ERR,"ERR:"M_NAME":s_dump: Snapshot done but no such mode %d\n",mode);
			return 0;
	}
}  

int bin_dump_to_table(bin_data *x, char* table, int snapshot_version, int step_version);

/**
 * Writes the binary data to a snapshot on the DB.
 * @param x - the binary data to write
 * @param prepend_fname - dump auth, dialog or registrar information
 * @returns 1 on success or 0 on failure
 */
int bin_dump_to_db(bin_data *x, char* prepend_fname){
	
	char* table;
	int snapshot_version;
	int step_version;
	
	if(!strncmp(prepend_fname,"sregistrar",10)){
		table="registrar_bulk";
		snapshot_version=*registrar_snapshot_version;//registrar snapshot timer must increment it
		step_version=*registrar_step_version;//registrar step timer must increment it
	}
	else{
		if(!strncmp(prepend_fname,"sdialogs",8)){
			table="dialogs_bulk";
			snapshot_version=*dialogs_snapshot_version;//dialogs snapshot timer must increment it
			step_version=*dialogs_step_version;//dialogs step timer must increment it
		}
		else{
			if(!strncmp(prepend_fname,"authdata",8)){
				table="auth_data_bulk";
				snapshot_version=*auth_snapshot_version;//auth snapshot timer must increment it
				step_version=*auth_step_version;//auth step timer must increment it
			}
			else{
				LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_db: No such information to dump %s\n", prepend_fname);
				return 0;
			}
		}
	}
	
	return bin_dump_to_table(x, table, snapshot_version, step_version);
}

int bin_db_keep_count=1; /**< how many old snapshots to keep */

/**
 * Drops older auth/dialogs/registrar snapshots,
 * keeping the 'bin_db_keep_count' most recent. 
 * @param table - where to drop.
 * @param current_snapshot_version - version of the current snapshot
 * @returns 1 on success or 0 on failure
 */
int delete_older_snapshots(char* table, int current_snapshot){

	db_key_t query_cols[1];
	db_op_t  query_ops[1];
	db_val_t query_vals[1];

	query_cols[0] = "snapshot_version";
	query_ops[0] = OP_LEQ;
	query_vals[0].type = DB_INT;
	query_vals[0].nul = 0;
	query_vals[0].val.int_val = current_snapshot - bin_db_keep_count;

	if (scscf_dbf.use_table(scscf_db, table) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":delete_older_snapshots(): Error in use_table\n");
		return 0;
	}

	if (scscf_dbf.delete(scscf_db, query_cols, query_ops, query_vals, 1) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":delete_older_snapshots(): Error while deleting older snapshots\n");
		return 0;
	}
	
	return 1;
}

/**
 * Writes auth/dialogs/registrar data to a snapshot on the DB.
 * @param x - the binary data to write
 * @param table - where to dump.
 * @param snapshot_version - version of the current snapshot
 * @param step_version - the step in the current snapshot
 * @returns 1 on success or 0 on failure
 */
int bin_dump_to_table(bin_data *x, char* table, int snapshot_version, int step_version){
	db_key_t keys[3];
	db_val_t vals[3];

	keys[0] = "snapshot_version";
	keys[1] = "step_version";
	keys[2] = "data";

	vals[0].type = DB_INT;
	vals[0].nul = 0;
	vals[0].val.int_val=snapshot_version;
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val=step_version;

	str d = {x->s, x->len};
	vals[2].type = DB_BLOB;
	vals[2].nul = 0;
	vals[2].val.blob_val = d;

	//lock
	lock_get(db_lock);

	if (scscf_dbf.use_table(scscf_db, table) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_dump_to_table(): Error in use_table\n");
		lock_release(db_lock);//unlock
		return 0;
	}

	if (scscf_dbf.insert(scscf_db, keys, vals, 3) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_dump_to_table(): Error while inserting on %s\n", table);
		lock_release(db_lock);//unlock
		return 0;
	}
	
	//delete older snapshots
	if (delete_older_snapshots(table, snapshot_version)!=1){
		LOG(L_ERR, "ERR:"M_NAME":bin_dump_to_table(): Error while deleting older snapshots from %s\n", table);
		lock_release(db_lock);//unlock
		return 0;
	}
	
	//unlock
	lock_release(db_lock);
	
	return 1;
}


/**
 * Loads the authorization data from the last snapshot.
 * @returns 1 on success or 0 on failure
 */
int load_snapshot_authdata()
{
	bin_data x;
	auth_userdata *aud;
	if (!s_load(&x,scscf_persistency_mode,scscf_persistency_location,"authdata")) goto error;
	//if (!bin_load(&x,scscf_persistency_mode,scscf_persistency_location,"authdata")) goto error;
	bin_print(&x);
	x.max=0;
	LOG(L_INFO,"INFO:"M_NAME":load_snapshot_auth: max %d len %d\n",x.max,x.len);
	while(x.max<x.len){
		aud = bin_decode_auth_userdata(&x);		
		if (!aud) return 0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_auth: Loaded auth_userdata for <%.*s>\n",aud->private_identity.len,aud->private_identity.s);
		auth_data_lock(aud->hash);
		aud->prev = auth_data[aud->hash].tail;
		aud->next = 0;
		if (auth_data[aud->hash].tail) auth_data[aud->hash].tail->next = aud;
		auth_data[aud->hash].tail = aud;
		if (!auth_data[aud->hash].head) auth_data[aud->hash].head = aud;
		auth_data_unlock(aud->hash);
	}
	bin_free(&x);
	return 1;
error:
	return 0;
	
}

int bin_load_from_db(bin_data *x, char* prepend_fname);

int s_load(bin_data *x,int mode,char *location,char* prepend_fname){
	
	switch (mode){
		case NO_PERSISTENCY:
			LOG(L_ERR,"ERR:"M_NAME":s_load: Persistency support was disabled\n");
			return 0;
		case WITH_FILES:
			return bin_load_from_file(x,location,prepend_fname);		
		case WITH_DATABASE_BULK:
			return bin_load_from_db(x, prepend_fname);
		case WITH_DATABASE_CACHE:
			LOG(L_ERR,"ERR:"M_NAME":s_load: WITH_DATABASE_CACHE not implemented...\n");
			return 0;
		default:
			LOG(L_ERR,"ERR:"M_NAME":s_load: Can't resume because no such mode %d\n",mode);
			return 0;
	}
}

int bin_load_from_table(bin_data *x, char* table);

/**
 * Reloads the last snapshot dumped to db.
 * @param x - where to load
 * @param prepend_fname - load auth, dialog or registrar information
 * @returns 1 on success, 0 on error
 */
int bin_load_from_db(bin_data *x, char* prepend_fname){
	
	char* table;
	
	if(!strncmp(prepend_fname,"sregistrar",10)){
		table="registrar_bulk";
	}
	else{
		if(!strncmp(prepend_fname,"sdialogs",8)){
			table="dialogs_bulk";
		}
		else{
			if(!strncmp(prepend_fname,"authdata",8)){
				table="auth_data_bulk";
			}
			else{
				LOG(L_ERR,"ERR:"M_NAME":bin_load_from_db: No such information to load %s\n", prepend_fname);
				return 0;
			}
		}
	}
	
	return bin_load_from_table(x, table);
}

int db_get_last_snapshot_version(char* table, int* version);
int set_versions(char* table, int snapshot_version, int step_version);

/**
 * Reloads the last snapshot dumped to db.
 * @param x - where to load
 * @param table - load auth, dialog or registrar information
 * @returns 1 on success, 0 on error
 */
int bin_load_from_table(bin_data *x, char* table){
	
	int snapshot_version;
	int r;
	bin_alloc(x,1024);//due to bin_free() on load_snapshot_...()
	
	//lock
	lock_get(db_lock);
	
	if((r=db_get_last_snapshot_version(table, &snapshot_version))==0){
		LOG(L_ERR,"ERR:"M_NAME":bin_load_from_table: Error while getting snapshot version\n");
		lock_release(db_lock);//unlock
		return 0;
	}
	else if(r==-1){//empty table, nothing to load
			LOG(L_INFO,"INFO:"M_NAME":bin_load_from_table: %s empty\n", table);
			lock_release(db_lock);//unlock
			return 1;
		}
		
	//set snap/step versions
	set_versions(table, snapshot_version+1, 0);
		
	db_key_t keys[1];
	db_val_t vals[1];
	db_op_t ops[1];
	db_key_t result_cols[1];
	
	db_res_t *res = NULL;
	
	keys[0] = "snapshot_version";
	
	ops[0] = OP_EQ;

	vals[0].type = DB_INT;
	vals[0].nul = 0;
	vals[0].val.int_val = snapshot_version;
	
	result_cols[0]="data";
	
	if (scscf_dbf.use_table(scscf_db, table) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_load_from_table(): Error in use_table\n");
		lock_release(db_lock);//unlock
		return -1;
	}
	
	if (scscf_dbf.query(scscf_db, keys, ops, vals, result_cols, 1, 1, 0, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_load_from_table(): Error while querying %s\n", table);
		lock_release(db_lock);//unlock
		return -1;
	}
	
	if (!res) {
		lock_release(db_lock);//unlock
		return 0;
	}
	
	if(res->n==0){
		scscf_dbf.free_result(scscf_db, res);
		lock_release(db_lock);//unlock
		return 1;
	}
	
	db_row_t* rows = RES_ROWS(res);
	db_val_t *row_vals = ROW_VALUES(rows);
	
	int len=row_vals[0].val.blob_val.len;
	LOG(L_INFO,"INFO:"M_NAME":%s row length -> %d\n", table, len);
	bin_expand(x, len);
	
	memcpy(x->s, row_vals[0].val.blob_val.s, len);
	x->len+=len;
	
	lock_release(db_lock);//unlock
	
	return 1;
}


/**
 * Gets the version of the last snapshot dumped to db.
 * Used to load information form db on startup.
 * @param table - the table to query
 * @param version - where to load
 * @returns 1 on success, -1 if empty table, 0 on error
 */
int db_get_last_snapshot_version(char* table, int* version){
	
	db_res_t *res = NULL;
	char sql[50];
	
	sprintf(sql, "SELECT max(snapshot_version) from %s", table);
	
	if (scscf_dbf.raw_query(scscf_db, sql, &res) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":db_get_last_snapshot_version(): Error while getting last snapshot version from %s\n", table);
		return 0;
	}
	
	if (!res) return 0;
	
	/*if(res->n==0) { //never happens with MAX function
		LOG(L_ERR, "ERR:"M_NAME":db_get_last_snapshot_version(): %s empty\n", table);
		scscf_dbf.free_result(scscf_db, res);
		return -1;
	}*/
	
	db_row_t* rows = RES_ROWS(res);
	db_val_t *row_vals = ROW_VALUES(rows);
	
	//MAX/MIN of an empty table will return a row of value NULL
	if(VAL_NULL(&row_vals[0])) {
		LOG(L_INFO, "INFO:"M_NAME":db_get_last_snapshot_version(): %s empty\n", table);
		scscf_dbf.free_result(scscf_db, res);
		return -1;
	}
	
	*version=row_vals[0].val.int_val;
	
	LOG(L_INFO, "INFO:"M_NAME":db_get_last_snapshot_version(): %s -> %d\n", table, *version);
	
	scscf_dbf.free_result(scscf_db, res);
	
	return 1;
}

/**
 * @param table - auth, dialogs or registrar versions?
 * @param snapshot_version - it must continue from the last snapshot version + 1
 * @param step_version - it must continue from the last step version + 1 
 * @returns 1 on success, 0 on error
 */
int set_versions(char* table, int snapshot_version, int step_version){

	if(!strncmp(table,"registrar_bulk",14)){
		*registrar_snapshot_version=snapshot_version;
		*registrar_step_version=step_version;
	}
	else{
		if(!strncmp(table,"dialogs_bulk",12)){
			*dialogs_snapshot_version=snapshot_version;
			*dialogs_step_version=step_version;
		}
		else{
			if(!strncmp(table,"auth_data_bulk",14)){
				*auth_snapshot_version=snapshot_version;
				*auth_step_version=step_version;
			}
			else{
				LOG(L_ERR,"ERR:"M_NAME"set_versions: No such information %s\n", table);
				return 0;
			}
		}
	}
	return 1;
}

/**
 * Timer callback for persistency dumps
 * @param ticks - what's the time
 * @param param - a given parameter to be called with
 */
void persistency_timer_authdata(unsigned int ticks, void* param)
{
	make_snapshot_authdata();
	
	(*auth_snapshot_version)++; 	
}





extern int s_dialogs_hash_size;						/**< size of the dialog hash table 					*/
extern s_dialog_hash_slot *s_dialogs;				/**< the hash table									*/

/**
 * Creates a snapshots of the dialogs data and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_dialogs()
{
	bin_data x;
	s_dialog *d;
	int i;	
	if (!bin_alloc(&x,256)) goto error;		
	for(i=0;i<s_dialogs_hash_size;i++){
		d_lock(i);
		d = s_dialogs[i].head;
		while(d){
			if (!bin_encode_s_dialog(&x,d)) goto error;
			d = d->next;
		}
		d_unlock(i);
	}
	bin_print(&x);
	i=s_dump(&x,scscf_persistency_mode,scscf_persistency_location,"sdialogs");
	//i = bin_dump(&x,scscf_persistency_mode,scscf_persistency_location,"sdialogs");		
	bin_free(&x);
	return i;
error:
	return 0;
}  

/**
 * Loads the dialogs data from the last snapshot.
 * @returns 1 on success or 0 on failure
 */
int load_snapshot_dialogs()
{
	bin_data x;
	s_dialog *d;
	if (!s_load(&x,scscf_persistency_mode,scscf_persistency_location,"sdialogs")) goto error;
	//if (!bin_load(&x,scscf_persistency_mode,scscf_persistency_location,"sdialogs")) goto error;
	bin_print(&x);
	x.max=0;
	LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dlg: max %d len %d\n",x.max,x.len);
	while(x.max<x.len){
		d = bin_decode_s_dialog(&x);
		if (!d) return 0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dlg: Loaded s_dialog for <%.*s>\n",d->aor.len,d->aor.s);
		d_lock(d->hash);
		d->prev = s_dialogs[d->hash].tail;
		d->next = 0;
		if (s_dialogs[d->hash].tail) s_dialogs[d->hash].tail->next = d;
		s_dialogs[d->hash].tail = d;
		if (!s_dialogs[d->hash].head) s_dialogs[d->hash].head = d;
		d_unlock(d->hash);
	}
	bin_free(&x);
	return 1;
error:
	return 0;
	
}


/**
 * Timer callback for persistency dumps
 * @param ticks - what's the time
 * @param param - a given parameter to be called with
 */
void persistency_timer_dialogs(unsigned int ticks, void* param)
{
	make_snapshot_dialogs();
	
	(*dialogs_snapshot_version)++;	 	
}



extern int r_hash_size;						/**< Size of S-CSCF registrar hash table		*/
extern r_hash_slot *registrar;				/**< The S-CSCF registrar 						*/

/**
 * Creates a snapshots of the registrar and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_registrar()
{
	bin_data x;
	r_public *p;
	int i;	
	if (!bin_alloc(&x,256)) goto error;		
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
		p = registrar[i].head;
		while(p){
			if (!bin_encode_r_public(&x,p)) goto error;
			p = p->next;
		}
		r_unlock(i);
	}
	bin_print(&x);
	i = s_dump(&x,scscf_persistency_mode,scscf_persistency_location,"sregistrar");
	//i = bin_dump(&x,scscf_persistency_mode,scscf_persistency_location,"sregistrar");		
	bin_free(&x);
	return i;
error:
	return 0;
}  

/**
 * Loads the registrar data from the last snapshot.
 * @returns 1 on success or 0 on failure
 */
int load_snapshot_registrar()
{
	bin_data x;
	r_public *p;
	if (!s_load(&x,scscf_persistency_mode,scscf_persistency_location,"sregistrar")) goto error;
	//if (!bin_load(&x,scscf_persistency_mode,scscf_persistency_location,"sregistrar")) goto error;
	bin_print(&x);
	x.max=0;
	LOG(L_INFO,"INFO:"M_NAME":load_snapshot_registrar: max %d len %d\n",x.max,x.len);
	while(x.max<x.len){
		p = bin_decode_r_public(&x);
		if (!p) return 0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_registrar: Loaded r_public for <%.*s>\n",p->aor.len,p->aor.s);
		r_lock(p->hash);
		p->prev = registrar[p->hash].tail;
		p->next = 0;
		if (registrar[p->hash].tail) registrar[p->hash].tail->next = p;
		registrar[p->hash].tail = p;
		if (!registrar[p->hash].head) registrar[p->hash].head = p;
		r_unlock(p->hash);
	}
	bin_free(&x);
	return 1;
error:
	return 0;
	
}


/**
 * Timer callback for persistency dumps
 * @param ticks - what's the time
 * @param param - a given parameter to be called with
 */
void persistency_timer_registrar(unsigned int ticks, void* param)
{
	make_snapshot_registrar();
	
	(*registrar_snapshot_version)++;
}

