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

int s_dump(bin_data* x, persistency_mode_t mode, char* location, char* prepend_fname);

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
			LOG(L_ERR,"ERR:"M_NAME":bin_dump: Snapshot done but persistency was disabled...\n");
			return 0;
		case WITH_FILES:
			return bin_dump_to_file(x,location,prepend_fname);
		case WITH_DATABASE_BULK:
			return bin_dump_to_db(x, prepend_fname);
		case WITH_DATABASE_CACHE:
			LOG(L_ERR,"ERR:"M_NAME":bin_dump: Snapshot done but WITH_DATABASE_CACHE not implemented...\n");
			return 0;
		default:
			LOG(L_ERR,"ERR:"M_NAME":bin_dump: Snapshot done but no such mode %d\n",mode);
			return 0;
	}
}  

int bin_dump_auth_to_db(bin_data *x);
int bin_dump_dialogs_to_db(bin_data *x);
int bin_dump_registrar_to_db(bin_data *x);

/**
 * Writes the binary data to a snapshot on the DB.
 * @param x - the binary data to write
 * @param what - dump auth, dialog or registrar information
 * @returns 1 on success or 0 on failure
 */
int bin_dump_to_db(bin_data *x, char* prepend_fname){
	
	if(!strncmp(prepend_fname,"sregistrar",10)){
		return bin_dump_registrar_to_db(x);
	}
	else{
		if(!strncmp(prepend_fname,"sdialogs",8)){
			return bin_dump_dialogs_to_db(x);
		}
		else{
			if(!strncmp(prepend_fname,"authdata",8)){
				return bin_dump_auth_to_db(x);
			}
			else{
				LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_db: No such information to dump %s\n", prepend_fname);
				return 0;
			}
		}
	}	
}

/**
 * Writes auth data to a snapshot on the DB.
 * @param x - the binary data to write
 * @returns 1 on success or 0 on failure
 */
int bin_dump_auth_to_db(bin_data *x){
	db_key_t keys[3];
	db_val_t vals[3];

	keys[0] = "snapshot_version";
	keys[1] = "step_version";
	keys[2] = "data";

	vals[0].type = DB_INT;
	vals[0].nul = 0;
	vals[0].val.int_val=*auth_snapshot_version;//auth snapshot timer must increment it
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val=*auth_step_version;//auth step timer must increment it

	str d = {x->s, x->len};
	vals[2].type = DB_BLOB;
	vals[2].nul = 0;
	vals[2].val.blob_val = d;


	if (scscf_dbf.use_table(scscf_db, "auth_data_bulk") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_dump_auth_to_db(): Error in use_table\n");
		return 0;
	}

	if (scscf_dbf.insert(scscf_db, keys, vals, 3) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_dump_auth_to_db(): Error while inserting auth_userdata\n");
		return 0;
	}
	
	return 1;
}

/**
 * Writes s_dialogs data to a snapshot on the DB.
 * @param x - the binary data to write
 * @returns 1 on success or 0 on failure
 */
int bin_dump_dialogs_to_db(bin_data *x){
	db_key_t keys[3];
	db_val_t vals[3];

	keys[0] = "snapshot_version";
	keys[1] = "step_version";
	keys[2] = "data";

	vals[0].type = DB_INT;
	vals[0].nul = 0;
	vals[0].val.int_val=*dialogs_snapshot_version;//dialogs snapshot timer must increment it
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val=*dialogs_step_version;//dialogs step timer must increment it

	str d = {x->s, x->len};
	vals[2].type = DB_BLOB;
	vals[2].nul = 0;
	vals[2].val.blob_val = d;


	if (scscf_dbf.use_table(scscf_db, "dialogs_bulk") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_dump_dialogs_to_db(): Error in use_table\n");
		return 0;
	}

	if (scscf_dbf.insert(scscf_db, keys, vals, 3) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_dump_dialogs_to_db(): Error while inserting s_dialogs\n");
		return 0;
	}
	
	return 1;
}

/**
 * Writes registrar data to a snapshot on the DB.
 * @param x - the binary data to write
 * @returns 1 on success or 0 on failure
 */
int bin_dump_registrar_to_db(bin_data *x){
	
	db_key_t keys[3];
	db_val_t vals[3];

	keys[0] = "snapshot_version";
	keys[1] = "step_version";
	keys[2] = "data";

	vals[0].type = DB_INT;
	vals[0].nul = 0;
	vals[0].val.int_val=*registrar_snapshot_version;//registrar snapshot timer must increment it
	
	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val=*registrar_step_version;//registrar step timer must increment it

	str d = {x->s, x->len};
	vals[2].type = DB_BLOB;
	vals[2].nul = 0;
	vals[2].val.blob_val = d;


	if (scscf_dbf.use_table(scscf_db, "registrar_bulk") < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_dump_registrar_to_db(): Error in use_table\n");
		return 0;
	}

	if (scscf_dbf.insert(scscf_db, keys, vals, 3) < 0) {
		LOG(L_ERR, "ERR:"M_NAME":bin_dump_registrar_to_db(): Error while inserting registrar information\n");
		return 0;
	}
	
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
	if (!bin_load(&x,scscf_persistency_mode,scscf_persistency_location,"authdata")) goto error;
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
	if (!bin_load(&x,scscf_persistency_mode,scscf_persistency_location,"sdialogs")) goto error;
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
	if (!bin_load(&x,scscf_persistency_mode,scscf_persistency_location,"sregistrar")) goto error;
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

