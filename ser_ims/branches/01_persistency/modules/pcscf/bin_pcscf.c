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
 * Binary codec operations for the S-CSCF
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */

#include "bin_pcscf.h"

/* structures reprezentation functions */

extern int r_hash_size;						/**< Size of S-CSCF registrar hash table		*/


static inline int str_shm_dup(str *dest,str *src)
{
	dest->s = shm_malloc(src->len);
	if (!dest->s){
		LOG(L_ERR,"ERR:"M_NAME":str_shm_dup: Error allocating %d bytes\n",src->len);
		return 0;
	}
	dest->len = src->len;
	memcpy(dest->s,src->s,src->len);
	return 1;
}



/**
 * Encode an dialog into a binary form
 * @param x - binary data to append to
 * @param u - the dialog to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_p_dialog(bin_data *x,p_dialog *d)
{
	int i,k;
	
	if (!bin_encode_str(x,&(d->call_id))) goto error;
	
	if (!bin_encode_str(x,&(d->host))) goto error;
	k = d->port;
	if (!bin_encode_int2(x,k)) goto error;
	k = d->transport;
	if (!bin_encode_int1(x,k)) goto error;

	if (!bin_encode_int2(x,d->routes_cnt)) goto error;
	for(i=0;i<d->routes_cnt;i++)
		if (!bin_encode_str(x,d->routes+i)) goto error;

	k = d->method;
	if (!bin_encode_int1(x,k)) goto error;
	if (!bin_encode_str(x,&(d->method_str))) goto error;
	
	if (!bin_encode_int4(x,d->first_cseq)) goto error;	
	if (!bin_encode_int4(x,d->last_cseq)) goto error;	

	if (!bin_encode_int1(x,d->state)) goto error;	

	if (!bin_encode_int4(x,d->expires)) goto error;		
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_p_dialog: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a dialog from a binary data structure
 * @param x - binary data to decode from
 * @returns the p_dialog* where the data has been decoded
 */
p_dialog* bin_decode_p_dialog(bin_data *x)
{
	p_dialog *d=0;
	int len,k,i;
	str s;
	
	len = sizeof(p_dialog);
	d = (p_dialog*) shm_malloc(len);
	if (!d) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_p_dialog: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(d,0,len);

	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->call_id),&s)) goto error;

	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->host),&s)) goto error;
	if (!bin_decode_int2(x,	&k)) goto error;
	d->port = k;
	if (!bin_decode_int1(x,	&k)) goto error;
	d->transport = k;

	if (!bin_decode_int2(x,	&k)) goto error;
	d->routes_cnt = k;

	len = sizeof(str)*d->routes_cnt;
	d = (p_dialog*) shm_malloc(len);
	if (!d) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_p_dialog: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(d->routes,0,len);	
	for(i=0;i<d->routes_cnt;i++)
		if (!bin_decode_str(x,&s)||!str_shm_dup(d->routes+i,&s)) goto error;
	
	if (!bin_decode_int1(x,	&k)) goto error;
	d->method = k;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->method_str),&s)) goto error;
	
	if (!bin_decode_int4(x,	&k)) goto error;
	d->first_cseq = k;
	if (!bin_decode_int4(x,	&k)) goto error;
	d->last_cseq = k;

	if (!bin_decode_int1(x,	&k)) goto error;
	d->state = k;
	
	if (!bin_decode_int4(x,	&k)) goto error;
	d->expires = k;
	
	d->hash = get_p_dialog_hash(d->call_id);		
	
	return d;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_p_dialog: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (d) {
		if (d->call_id.s) shm_free(d->call_id.s);
		if (d->host.s) shm_free(d->host.s);
		if (d->routes_cnt){
			for(i=0;i<d->routes_cnt;i++)
				if (d->routes[i].s) shm_free(d->routes[i].s);
			shm_free(d->routes);
		}
		if (d->method_str.s) shm_free(d->method_str.s);
		shm_free(d);
	}
	return 0;
}










/**
 * Encode an ipsec into a binary form
 * @param x - binary data to append to
 * @param ipsec - the r_ipsec to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_ipsec(bin_data *x,r_ipsec *ipsec)
{
	if (!ipsec){
		if (!bin_encode_int1(x,0)) goto error;
		return 1;
	}
	
	if (!bin_encode_int1(x,1)) goto error;

	if (!bin_encode_int4(x,ipsec->spi_uc)) goto error;
	if (!bin_encode_int4(x,ipsec->spi_us)) goto error;
	if (!bin_encode_int4(x,ipsec->spi_pc)) goto error;
	if (!bin_encode_int4(x,ipsec->spi_ps)) goto error;
	if (!bin_encode_int2(x,ipsec->port_uc)) goto error;
	if (!bin_encode_int2(x,ipsec->port_us)) goto error;
	
	if (!bin_encode_str(x,&(ipsec->ealg))) goto error;
	if (!bin_encode_str(x,&(ipsec->ck))) goto error;
	if (!bin_encode_str(x,&(ipsec->alg))) goto error;
	if (!bin_encode_str(x,&(ipsec->ik))) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_ipsec: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a ipsec from a binary data structure
 * @param x - binary data to decode from
 * @param ipsec - ** to write into
 * @returns 1 on success or 0 on failure
 */
int bin_decode_ipsec(bin_data *x,r_ipsec **ipsec)
{
	int len,k;
	str s;

	if (!bin_decode_int1(x,	&k)) goto error;
	
	if (k==0) {
		*ipsec = 0;
		return 1;
	}
	
	len = sizeof(r_ipsec);
	*ipsec = (r_ipsec*) shm_malloc(len);
	if (!*ipsec) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_ipsec: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(*ipsec,0,len);

	if (!bin_decode_int4(x,	&k)) goto error;
	(*ipsec)->spi_uc = k;
	if (!bin_decode_int4(x,	&k)) goto error;
	(*ipsec)->spi_us = k;
	if (!bin_decode_int4(x,	&k)) goto error;
	(*ipsec)->spi_pc = k;
	if (!bin_decode_int4(x,	&k)) goto error;
	(*ipsec)->spi_ps = k;
	if (!bin_decode_int2(x,	&k)) goto error;
	(*ipsec)->port_uc = k;
	if (!bin_decode_int2(x,	&k)) goto error;
	(*ipsec)->port_us = k;

	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->ealg),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->ck),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->alg),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&((*ipsec)->ik),&s)) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_ipsec: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (*ipsec) {
		if ((*ipsec)->ealg.s) shm_free((*ipsec)->ealg.s);
		if ((*ipsec)->ck.s) shm_free((*ipsec)->ck.s);
		if ((*ipsec)->alg.s) shm_free((*ipsec)->alg.s);
		if ((*ipsec)->ik.s) shm_free((*ipsec)->ik.s);
		shm_free(*ipsec);
		*ipsec = 0;
	}
	return 0;
}



/**
 * Encode a pinhole into a binary form
 * @param x - binary data to append to
 * @param u - the dialog to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_pinhole(bin_data *x,r_nat_dest *pinhole)
{
	if (!pinhole){
		if (!bin_encode_int1(x,0)) goto error;
		return 1;
	}
	
	if (!bin_encode_int1(x,1)) goto error;

	if (!bin_encode_int4(x,pinhole->nat_addr.af)) goto error;
	if (!bin_encode_int1(x,pinhole->nat_addr.len)) goto error;
	if (!bin_encode_int4(x,pinhole->nat_addr.u.addr32[0])) goto error;
	if (!bin_encode_int4(x,pinhole->nat_addr.u.addr32[1])) goto error;
	if (!bin_encode_int4(x,pinhole->nat_addr.u.addr32[2])) goto error;
	if (!bin_encode_int4(x,pinhole->nat_addr.u.addr32[3])) goto error;
	
	if (!bin_encode_int1(x,pinhole->nat_port)) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_pinhole: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a pinhole from a binary data structure
 * @param x - binary data to decode from
 * @param pinhole - ** to write into
 * @returns 1 on success or 0 on failure
 */
int bin_decode_pinhole(bin_data *x,r_nat_dest **pinhole)
{
	int len,k;
	
	if (!bin_decode_int1(x,	&k)) goto error;
	
	if (k==0) {
		*pinhole = 0;
		return 1;
	}
	
	len = sizeof(r_nat_dest);
	*pinhole = (r_nat_dest*) shm_malloc(len);
	if (!*pinhole) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_pinhole: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(*pinhole,0,len);

	if (!bin_decode_int4(x,	&k)) goto error;
	(*pinhole)->nat_addr.af = k;
	if (!bin_decode_int1(x,	&k)) goto error;
	(*pinhole)->nat_addr.len = k;
	if (!bin_decode_int4(x,	&k)) goto error;
	(*pinhole)->nat_addr.u.addr32[0] = k;
	if (!bin_decode_int4(x,	&k)) goto error;
	(*pinhole)->nat_addr.u.addr32[1] = k;
	if (!bin_decode_int4(x,	&k)) goto error;
	(*pinhole)->nat_addr.u.addr32[2] = k;
	if (!bin_decode_int4(x,	&k)) goto error;
	(*pinhole)->nat_addr.u.addr32[3] = k;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_pinhole: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (*pinhole) {
		shm_free(*pinhole);
		*pinhole = 0;
	}
	return 0;
}



/**
 * Encode a r_public into a binary form
 * @param x - binary data to append to
 * @param p - the r_public to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_r_public(bin_data *x,r_public *p)
{
	if (!bin_encode_str(x,&(p->aor))) goto error;
	if (!bin_encode_int1(x,p->is_default)) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_r_public: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a r_public from a binary data structure
 * @param x - binary data to decode from
 * @returns the new *r_public or 0 on error
 */
r_public* bin_decode_r_public(bin_data *x)
{
	str s;
	int k,len;
	r_public *p;

	len = sizeof(r_public);
	p = (r_public*) shm_malloc(len);
	if (!p) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_public: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(p,0,len);
	
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(p->aor),&s)) goto error;
	if (!bin_decode_int1(x,	&k)) goto error;
	p->is_default = k;
	
	return p;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_public: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (p) {
		if (p->aor.s) shm_free(p->aor.s);
		shm_free(p);
	}
	return 0;
}











/**
 * Encode a r_contact into a binary form
 * @param x - binary data to append to
 * @param p - the r_contact to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_r_contact(bin_data *x,r_contact *c)
{
	int k,i;
	r_public *p=0;
	
	if (!bin_encode_str(x,&(c->host))) goto error;
	k = c->port;
	if (!bin_encode_int2(x,k)) goto error;
	k = c->transport;
	if (!bin_encode_int1(x,k)) goto error;
	
	if (!bin_encode_ipsec(x,c->ipsec)) goto error;
	
	if (!bin_encode_str(x,&(c->uri))) goto error;
	
	k = c->reg_state+5;
	if (!bin_encode_int1(x,k)) goto error;

	if (!bin_encode_int4(x,c->expires)) goto error;

	k = c->service_route_cnt;
	if (!bin_encode_int2(x,k)) goto error;
	for(i=0;i<c->service_route_cnt;i++)
		if (!bin_encode_str(x,c->service_route+i)) goto error;
		
	if (!bin_encode_pinhole(x,c->pinhole)) goto error;
	
	k=0;
	for(p=c->head;p;p=p->next)
		k++;
	if (!bin_encode_int2(x,k)) goto error;
	for(p=c->head;p;p=p->next)
		if (!bin_encode_r_public(x,p)) goto error;	
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_r_contact: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a r_contact from a binary data structure
 * @param x - binary data to decode from
 * @returns the r_contact* where the data has been decoded
 */
r_contact* bin_decode_r_contact(bin_data *x)
{
	r_contact *c=0;
	r_public *p=0,*pn=0;
	int len,k,i;
	str st;
	
	len = sizeof(r_contact);
	c = (r_contact*) shm_malloc(len);
	if (!c) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(c,0,len);
	
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(c->host),&st)) goto error;
	if (!bin_decode_int2(x,&k)) goto error;
	c->port = k;
	if (!bin_decode_int1(x,&k)) goto error;
	c->transport = k;

	c->hash = get_contact_hash(c->host,c->port,c->transport,r_hash_size);
	
	if (!bin_decode_ipsec(x,&(c->ipsec ))) goto error;
	
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(c->uri),&st)) goto error;
	
	if (!bin_decode_int1(x,&k)) goto error;
	c->reg_state = k-5;

	if (!bin_decode_int4(x,&k)) goto error;
	c->expires = k;
	
	if (!bin_decode_int2(x,	&k)) goto error;
	c->service_route_cnt = k;

	len = sizeof(str)*c->service_route_cnt;
	c->service_route = (str*) shm_malloc(len);
	if (!c->service_route) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(c->service_route,0,len);	
	for(i=0;i<c->service_route_cnt;i++)
		if (!bin_decode_str(x,&st)||!str_shm_dup(c->service_route+i,&st)) goto error;
	
	if (!bin_decode_pinhole(x,&(c->pinhole ))) goto error;
		
	if (!bin_decode_int2(x,&k)) goto error;
	for(i=0;i<k;i++){
		p = bin_decode_r_public(x);
		if (!p) goto error;
		p->prev = c->tail;
		p->next = 0;
		if (c->tail) c->tail->next = p;
		c->tail = p;
		if (!c->head) c->head = p;
	}
	return c;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (c) {
		if (c->host.s) shm_free(c->host.s);		
		if (c->ipsec) shm_free(c->ipsec);
		if (c->uri.s) shm_free(c->uri.s);
		if (c->pinhole) shm_free(c->pinhole);
		while(c->head){
			p = c->head;
			pn = p->next;
			free_r_public(p);
			c->head = pn;
		}
		shm_free(c);
	}
	return 0;
}





