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
 * P-CSCF persistency operations
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */

#include "p_persistency.h"

extern persistency_mode_t pcscf_persistency_mode;/**< the type of persistency					*/
extern char* pcscf_persistency_location;		/**< where to dump the persistency data 		*/ 


extern int p_dialogs_hash_size;						/**< size of the dialog hash table 					*/
extern p_dialog_hash_slot *p_dialogs;				/**< the hash table									*/

/**
 * Creates a snapshots of the dialogs data and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_dialogs()
{
	bin_data x;
	p_dialog *d;
	int i;	
	if (!bin_alloc(&x,256)) goto error;		
	for(i=0;i<p_dialogs_hash_size;i++){
		d_lock(i);
		d = p_dialogs[i].head;
		while(d){
			if (!bin_encode_p_dialog(&x,d)) goto error;
			d = d->next;
		}
		d_unlock(i);
	}
	//bin_print(&x);
	i = bin_dump(&x,pcscf_persistency_mode,pcscf_persistency_location,"pdialogs");		
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
	p_dialog *d;
	if (!bin_load(&x,pcscf_persistency_mode,pcscf_persistency_location,"pdialogs")) goto error;
	//bin_print(&x);
	x.max=0;
	LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dlg: max %d len %d\n",x.max,x.len);
	while(x.max<x.len){
		d = bin_decode_p_dialog(&x);
		if (!d) return 0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_dlg: Loaded p_dialog for <%.*s>\n",d->host.len,d->host.s);
		d_lock(d->hash);
		d->prev = p_dialogs[d->hash].tail;
		d->next = 0;
		if (p_dialogs[d->hash].tail) p_dialogs[d->hash].tail->next = d;
		p_dialogs[d->hash].tail = d;
		if (!p_dialogs[d->hash].head) p_dialogs[d->hash].head = d;
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



extern int r_hash_size;						/**< Size of P-CSCF registrar hash table		*/
extern r_hash_slot *registrar;				/**< The P-CSCF registrar 						*/

/**
 * Creates a snapshots of the registrar and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_registrar()
{
	bin_data x;
	r_contact *c;
	int i;	
	if (!bin_alloc(&x,256)) goto error;		
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
		c = registrar[i].head;
		while(c){
			if (!bin_encode_r_contact(&x,c)) goto error;
			c = c->next;
		}
		r_unlock(i);
	}
	//bin_print(&x);
	i = bin_dump(&x,pcscf_persistency_mode,pcscf_persistency_location,"pregistrar");		
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
	r_contact *c;
	if (!bin_load(&x,pcscf_persistency_mode,pcscf_persistency_location,"pregistrar")) goto error;
	//bin_print(&x);
	x.max=0;
	LOG(L_INFO,"INFO:"M_NAME":load_snapshot_registrar: max %d len %d\n",x.max,x.len);
	while(x.max<x.len){
		c = bin_decode_r_contact(&x);
		if (!c) return 0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_registrar: Loaded r_contact for <%.*s>\n",c->uri.len,c->uri.s);
		r_lock(c->hash);
		c->prev = registrar[c->hash].tail;
		c->next = 0;
		if (registrar[c->hash].tail) registrar[c->hash].tail->next = c;
		registrar[c->hash].tail = c;
		if (!registrar[c->hash].head) registrar[c->hash].head = c;
		r_unlock(c->hash);
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





extern int subscriptions_hash_size;						/**< Size of P-CSCF subscriptions hash table		*/
extern r_subscription_hash_slot *subscriptions;				/**< The P-CSCF subscriptions 						*/

/**
 * Creates a snapshots of the subscriptions and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_subscriptions()
{
	bin_data x;
	r_subscription *s;
	int i;	
	if (!bin_alloc(&x,256)) goto error;		
	for(i=0;i<subscriptions_hash_size;i++){
		subs_lock(i);
		s = subscriptions[i].head;
		while(s){
			if (!bin_encode_r_subscription(&x,s)) goto error;
			s = s->next;
		}
		subs_unlock(i);
	}
	//bin_print(&x);
	i = bin_dump(&x,pcscf_persistency_mode,pcscf_persistency_location,"psubscriptions");		
	bin_free(&x);
	return i;
error:
	return 0;
}  

/**
 * Loads the subscriptions data from the last snapshot.
 * @returns 1 on success or 0 on failure
 */
int load_snapshot_subscriptions()
{
	bin_data x;
	r_subscription *s;
	if (!bin_load(&x,pcscf_persistency_mode,pcscf_persistency_location,"psubscriptions")) goto error;
	//bin_print(&x);
	x.max=0;
	LOG(L_INFO,"INFO:"M_NAME":load_snapshot_subscriptions: max %d len %d\n",x.max,x.len);
	while(x.max<x.len){
		s = bin_decode_r_subscription(&x);
		if (!s) return 0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_subscriptions: Loaded r_subscription for <%.*s>\n",s->req_uri.len,s->req_uri.s);
		subs_lock(s->hash);
		s->prev = subscriptions[s->hash].tail;
		s->next = 0;
		if (subscriptions[s->hash].tail) subscriptions[s->hash].tail->next = s;
		subscriptions[s->hash].tail = s;
		if (!subscriptions[s->hash].head) subscriptions[s->hash].head = s;
		subs_unlock(s->hash);
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
void persistency_timer_subscriptions(unsigned int ticks, void* param)
{
	make_snapshot_subscriptions();	 	
}

