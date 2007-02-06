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
	i = bin_dump(&x,scscf_persistency_mode,scscf_persistency_location,"authdata");		
	bin_free(&x);
	return i;
error:
	return 0;
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
	i = bin_dump(&x,scscf_persistency_mode,scscf_persistency_location,"sdialogs");		
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
	i = bin_dump(&x,scscf_persistency_mode,scscf_persistency_location,"sregistrar");		
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
}

