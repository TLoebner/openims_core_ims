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

#include <time.h>
#include <errno.h>

#include "bin_scscf.h"

extern auth_hash_slot_t *auth_data;			/**< Authentication vector hash table */
extern int auth_data_hash_size;						/**< authentication vector hash table size */

int bin_dump_to_file(bin_data *x,char *prep_fname)
{
	char c[256];
	time_t now;
	FILE *f;
	int k;
	
	now = time(0);
	sprintf(c,"%s_last.bin",prep_fname/*,(unsigned int)now*/);
	f = fopen(c,"w");
	k = fwrite(x->s,1,x->len,f);
	LOG(L_INFO,"INFO:"M_NAME":bin_dump_to_file: Dumped %d bytes into %s.\n",k,c);
	fclose(f);	
	if (k==x->len) return 1;
	else return 0;
}

int bin_load_from_file(bin_data *x,char *prep_fname)
{
	char c[256];
	time_t now;
	FILE *f;
	int k;
	
	now = time(0);
	sprintf(c,"%s_last.bin",prep_fname/*,(unsigned int)now*/);
	f = fopen(c,"r");
	if (!f) {
		LOG(L_ERR,"error opening %s : %s\n",c,strerror(errno));
		return 0;
	}
	bin_alloc(x,1024);
	while(!feof(f)){
		bin_expand(x,1024);
		k = fread(x->s+x->len,1,1024,f);
		x->len+=k;
	}
	LOG(L_INFO,"INFO:"M_NAME":bin_load_from_file: Read %d bytes from %s.\n",x->len,c);
	fclose(f);	
	return 1;
}


int make_snapshot_auth(auth_hash_slot_t *ad)
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
	i = bin_dump_to_file(&x,"/opt/OpenIMSCore/authdata");
	bin_free(&x);
	return i;
error:
	return 0;
}  

int load_snapshot_auth(auth_hash_slot_t *ad)
{
	bin_data x;
	auth_userdata *aud;
	if (!bin_load_from_file(&x,"/opt/OpenIMSCore/authdata")) goto error;
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
