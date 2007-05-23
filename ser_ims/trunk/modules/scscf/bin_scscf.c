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

#include "bin_scscf.h"

extern struct tm_binds tmb;   		/**< Structure with pointers to tm funcs 		*/

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
 *	Encode and append a Public Indentity
 * @param x - binary data to append to
 * @param pi - the public identity to encode
 * @returns 1 on succcess or 0 on error
 */
static int bin_encode_public_identity(bin_data *x,ims_public_identity *pi)
{
	if (!bin_encode_char(x,pi->barring)) goto error;
	if (!bin_encode_str(x,&(pi->public_identity))) goto error;	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_public_identity: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a binary string from a binary data structure
 * @param x - binary data to decode from
 * @param pi - the public identity to decode into
 * @returns 1 on succcess or 0 on error
 */
static int bin_decode_public_identity(bin_data *x,ims_public_identity *pi)
{
	str s;
	if (!bin_decode_char(x,	&(pi->barring))) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(pi->public_identity),&s))	goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_public_identity: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (pi) {
		if (pi->public_identity.s) shm_free(pi->public_identity.s);
	}
	return 0;
}






/**
 *	Encode and append a SPT
 * @param x - binary data to append to
 * @param spt - the service point trigger to encode
 * @returns 1 on succcess or 0 on error
 */
static int bin_encode_spt(bin_data *x, ims_spt *spt)
{
	unsigned char c = spt->condition_negated<<7 | spt->registration_type<<4 | spt->type;
	// cond negated, reg type, spt type
	if (!bin_encode_uchar(x,c)) goto error;

	//group
	if (!bin_encode_int(x,spt->group)) goto error;

	//spt
	switch(spt->type){
		case 1:
			if (!bin_encode_str(x,&(spt->request_uri))) goto error; 
			break;
		case 2:
			if (!bin_encode_str(x,&(spt->method))) goto error; 
			break;
		case 3:
			if (!bin_encode_short(x,spt->sip_header.type)) goto error;
			if (!bin_encode_str(x,&(spt->sip_header.header))) goto error; 
			if (!bin_encode_str(x,&(spt->sip_header.content))) goto error; 
			break;
		case 4:
			if (!bin_encode_char(x,spt->session_case)) goto error;
			break;
		case 5:
			if (!bin_encode_str(x,&(spt->session_desc.line))) goto error; 
			if (!bin_encode_str(x,&(spt->session_desc.content))) goto error; 
			break;
	}
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_spt: Error while encoding.\n");
	return 0;		
}


/**
 *	Decode an SPT
 * @param x - binary data to decode from
 * @param spt - the service point trigger to decode into
 * @returns 1 on succcess or 0 on error
 */
static int bin_decode_spt(bin_data *x, ims_spt *spt)
{
	unsigned char k;
	str s;
	
	if (!bin_decode_uchar(x,&k)) goto error;
	
	spt->type = k & 0x0F;
	spt->condition_negated = ((k & 0x80)!=0);
	spt->registration_type = ((k & 0x70)>>4);
	
	if (!bin_decode_int(x,&(spt->group))) goto error;

	switch (spt->type){
		case 1:
			if (!bin_decode_str(x,&s)||!str_shm_dup(&(spt->request_uri),&s)) goto error;
			break;
		case 2:
			if (!bin_decode_str(x,&s)||!str_shm_dup(&(spt->method),&s)) goto error;
			break;
		case 3:
			if (!bin_decode_short(x,&(spt->sip_header.type))) goto error;
			if (!bin_decode_str(x,&s)||!str_shm_dup(&(spt->sip_header.header),&s)) goto error;
			if (!bin_decode_str(x,&s)||!str_shm_dup(&(spt->sip_header.content),&s)) goto error;
			break;
		case 4:
			if (!bin_decode_char(x,&(spt->session_case))) goto error;
			break;
		case 5:
			if (!bin_decode_str(x,&s)||!str_shm_dup(&(spt->session_desc.line),&s)) goto error;
			if (!bin_decode_str(x,&s)||!str_shm_dup(&(spt->session_desc.content),&s)) goto error;
			break;

	}
	return 1;
	
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_spt: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (spt){
		switch (spt->type){
			case 1:
				if (spt->request_uri.s) shm_free(spt->request_uri.s);
				break;
			case 2:
				if (spt->method.s) shm_free(spt->method.s);
				break;
			case 3:
				if (spt->sip_header.header.s) shm_free(spt->sip_header.header.s);
				if (spt->sip_header.header.s) shm_free(spt->sip_header.content.s);
				break;
			case 4:
				break;
			case 5:
				if (spt->sip_header.header.s) shm_free(spt->session_desc.line.s);
				if (spt->sip_header.header.s) shm_free(spt->session_desc.content.s);
				break;
		}
	}
	return 0;
}	



/**
 *	Encode and Append a Filter Criteria
 * @param x - binary data to append to
 * @param spt - the service point trigger to encode
 * @returns 1 on succcess or 0 on error
 */
static int bin_encode_filter_criteria(bin_data *x, ims_filter_criteria *fc)
{
	int i;
	char ppindicator;

	//priority
	if (!bin_encode_int(x,fc->priority)) goto error;
	
	//profile part indicator
	if (fc->profile_part_indicator) ppindicator = (*fc->profile_part_indicator)+1;
	else ppindicator = 0;
	if (!bin_encode_char(x,ppindicator)) goto error;
			
	// trigger point 
	if (fc->trigger_point) {
		if (!bin_encode_char(x,fc->trigger_point->condition_type_cnf)) goto error;
		
		if (!bin_encode_ushort(x,fc->trigger_point->spt_cnt)) goto error;
		
		for(i=0;i<fc->trigger_point->spt_cnt;i++)
			if (!bin_encode_spt(x,fc->trigger_point->spt+i)) goto error;
	} else {
		if (!bin_encode_char(x,100)) goto error;
	}
	
	//app server
	if (!bin_encode_str(x,&(fc->application_server.server_name))) goto error;
	if (!bin_encode_char(x,fc->application_server.default_handling)) goto error;
	if (!bin_encode_str(x,&(fc->application_server.service_info))) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_filter_criteria: Error while encoding.\n");
	return 0;		
}


/**
 *	Decode a Filter Criteria
 * @param x - binary data to decode from
 * @param fc - filter criteria to decode into
 * @returns 1 on succcess or 0 on error
 */
static int bin_decode_filter_criteria(bin_data *x, ims_filter_criteria *fc)
{
	int i,len;
	str s;
	char ppindicator,cnf;
	
	//priority
	if (!bin_decode_int(x,&(fc->priority))) goto error;
	
	// profile part indicator
	if (!bin_decode_char(x,&ppindicator)) goto error;
	if (!ppindicator){
		fc->profile_part_indicator = 0;
	}
	else {
		fc->profile_part_indicator = (char*)shm_malloc(sizeof(char));
		if (!fc->profile_part_indicator) {
			LOG(L_ERR,"ERR:"M_NAME":bin_decode_filter_criteria: Error allocating %d bytes.\n",sizeof(int));
			goto error;
		}
		*(fc->profile_part_indicator) = ppindicator-1;
	}
	
	//cnf 
	if (!bin_decode_char(x,&cnf)) goto error;

	if (cnf==100)
		fc->trigger_point=NULL;
	else {
		ims_trigger_point *tp=0;
		//trigger point
		len = sizeof(ims_trigger_point);
		tp = (ims_trigger_point*)shm_malloc(len);
		fc->trigger_point = tp;
		if (!tp) {
			LOG(L_ERR,"ERR:"M_NAME":bin_decode_filter_criteria: Error allocating %d bytes.\n",len);
			goto error;
		}
		memset(tp,0,len);
		tp->condition_type_cnf=cnf;
		
		if (!bin_decode_ushort(x,&tp->spt_cnt)) goto error;
		len = sizeof(ims_spt)*tp->spt_cnt;
		tp->spt = (ims_spt*)shm_malloc(len);
		if (!tp->spt) {
			LOG(L_ERR,"ERR:"M_NAME":bin_decode_filter_criteria: Error allocating %d bytes.\n",len);
			goto error;
		}
		memset(tp->spt,0,len);
		for(i=0;i<tp->spt_cnt;i++)
			if (!bin_decode_spt(x,tp->spt+i)) goto error;
	}
	//app server uri
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(fc->application_server.server_name),&s)) goto error;
	// app server default handling
	if (!bin_decode_char(x,&(fc->application_server.default_handling)))goto error;
	// app server service info
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(fc->application_server.service_info),&s)) goto error;

	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_filter_criteria: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (fc){
		if (fc->trigger_point){
			if (fc->trigger_point){
				if (fc->trigger_point->spt) shm_free(fc->trigger_point->spt);
			}
			shm_free(fc->trigger_point);
		}
		if (fc->application_server.server_name.s) shm_free(fc->application_server.server_name.s);
		if (fc->application_server.service_info.s) shm_free(fc->application_server.service_info.s);
	}
	return 0;		
}






/**
 *	Encode and append a Service Profile
 * @param x - binary data to append to
 * @param sp - the service profile to encode
 * @returns 1 on succcess or 0 on error
 */
static int bin_encode_service_profile(bin_data *x,ims_service_profile *sp)
{
	int i;
		
	//public identity
	if (!bin_encode_ushort(x,sp->public_identities_cnt)) return 0;
	for(i=0;i<sp->public_identities_cnt;i++)
		if (!bin_encode_public_identity(x,sp->public_identities+i)) goto error;
	
	//filter criteria
	if (!bin_encode_ushort(x,sp->filter_criteria_cnt)) return 0;
	for(i=0;i<sp->filter_criteria_cnt;i++)
		if (!bin_encode_filter_criteria(x,sp->filter_criteria+i)) goto error;
		
	//cn service_auth
	if (sp->cn_service_auth)
		i = sp->cn_service_auth->subscribed_media_profile_id;
	else i = 0xFFFFFFFF;
	if (!bin_encode_int(x,i)) goto error;

	//shared_ifc
	if (!bin_encode_ushort(x,sp->shared_ifc_set_cnt)) return 0;
	for(i=0;i<sp->shared_ifc_set_cnt;i++)
		if (!bin_encode_int(x,sp->shared_ifc_set[i])) goto error;

	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_service_profile: Error while encoding.\n");
	return 0;		
}


/**
 *	Decode a service profile
 * @param x - binary data to decode from
 * @param sp - service profile to decode into
 * @returns 1 on succcess or 0 on error
 */
static int bin_decode_service_profile(bin_data *x, ims_service_profile *sp)
{
	int i,len;

	//public identities
	if (!bin_decode_ushort(x,&(sp->public_identities_cnt))) goto error;
	len = sizeof(ims_public_identity)*sp->public_identities_cnt;
	sp->public_identities = (ims_public_identity*)shm_malloc(len);
	if (!sp->public_identities) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_service_profile: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(sp->public_identities,0,len);
	for(i=0;i<sp->public_identities_cnt;i++)
		if (!bin_decode_public_identity(x,sp->public_identities+i)) goto error;
	
	// filter criteria
	if (!bin_decode_ushort(x,&(sp->filter_criteria_cnt))) goto error;	
	len = sizeof(ims_filter_criteria)*sp->filter_criteria_cnt;
	sp->filter_criteria = (ims_filter_criteria*)shm_malloc(len);
	if (!sp->filter_criteria) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_service_profile: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(sp->filter_criteria,0,len);
	for(i=0;i<sp->filter_criteria_cnt;i++)
		if (!bin_decode_filter_criteria(x,sp->filter_criteria+i)) goto error;

	// cn service auth
	if (!bin_decode_int(x,&i)) goto error;
	if (i==0xFFFFFFFF)
		sp->cn_service_auth = 0;
	else {
		len = sizeof(ims_cn_service_auth);
		sp->cn_service_auth = (ims_cn_service_auth*)shm_malloc(len);
		if (!sp->cn_service_auth) {
			LOG(L_ERR,"ERR:"M_NAME":bin_decode_service_profile: Error allocating %d bytes.\n",len);
			goto error;
		}
		sp->cn_service_auth->subscribed_media_profile_id=i;
	}
	
	//shared ifc
	if (!bin_decode_ushort(x,&(sp->shared_ifc_set_cnt))) goto error;	
	len = sizeof(int)*sp->shared_ifc_set_cnt;
	sp->shared_ifc_set = (int*)shm_malloc(len);
	if (!sp->shared_ifc_set) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_service_profile: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(sp->shared_ifc_set,0,len);
	for(i=0;i<sp->shared_ifc_set_cnt;i++)
		if (!bin_decode_int(x,sp->shared_ifc_set+i)) goto error;

	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_service_profile: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (sp) {
		if (sp->public_identities) shm_free(sp->public_identities);
		if (sp->filter_criteria) shm_free(sp->filter_criteria);
		if (sp->cn_service_auth) shm_free(sp->cn_service_auth);
		if (sp->shared_ifc_set) shm_free(sp->shared_ifc_set);
	}
	return 0;
}













/**
 *	Encode the entire user profile and append it to the binary data
 * @param x - binary data to append to
 * @param s - the ims subscription to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_ims_subscription(bin_data *x, ims_subscription *s)
{
	int i;
	if (!bin_encode_str(x,&(s->private_identity))) goto error;
	if (!bin_encode_ushort(x,s->service_profiles_cnt)) goto error;

	for(i=0;i<s->service_profiles_cnt;i++)
		if (!bin_encode_service_profile(x,s->service_profiles+i)) goto error;
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_ims_subscription: Error while encoding.\n");
	return 0;	
}


/**
 *	Decode a binary string from a binary data structure
 * @param x - binary data to decode from
 * @returns the ims_subscription* where the data has been decoded
 */
ims_subscription *bin_decode_ims_subscription(bin_data *x)
{
	ims_subscription *imss=0;
	int i,len;
	str s;
	
	imss = (ims_subscription*) shm_malloc(sizeof(ims_subscription));
	if (!imss) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_ims_subscription: Error allocating %d bytes.\n",sizeof(ims_subscription));
		goto error;
	}
	memset(imss,0,sizeof(ims_subscription));
	
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(imss->private_identity),&s)) goto error;
	if (!bin_decode_ushort(x,	&(imss->service_profiles_cnt))) goto error;
	
	len = sizeof(ims_service_profile)*imss->service_profiles_cnt;
	imss->service_profiles = (ims_service_profile*)shm_malloc(len);
	if (!imss->service_profiles) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_ims_subscription: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(imss->service_profiles,0,len);

	for(i=0;i<imss->service_profiles_cnt;i++)
		if (!bin_decode_service_profile(x,imss->service_profiles+i)) goto error;

	imss->lock = lock_alloc();
	imss->lock = lock_init(imss->lock);
	imss->ref_count = 1;

	return imss;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_ims_subscription: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (imss) {
		if (imss->private_identity.s) shm_free(imss->private_identity.s);
		if (imss->service_profiles) shm_free(imss->service_profiles);
		shm_free(imss);
	}
	return 0;
}





/**
 * Encode a r_public into a binary form
 * @param x - binary data to append to
 * @param c - the r_contact to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_r_contact(bin_data *x,r_contact *c)
{
	if (!bin_encode_str(x,&(c->uri))) goto error;
	if (!bin_encode_time_t(x,c->expires)) goto error;
	if (!bin_encode_str(x,&(c->ua))) goto error;
	if (!bin_encode_str(x,&(c->path))) goto error;

	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_r_contact: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a contact from a binary data structure
 * @param x - binary data to decode from
 * @returns the r_contact* where the data has been decoded
 */
r_contact* bin_decode_r_contact(bin_data *x)
{
	r_contact *c=0;
	int len;
	str s;
	
	len = sizeof(r_contact);
	c = (r_contact*) shm_malloc(len);
	if (!c) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(c,0,len);
	
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(c->uri),&s)) goto error;
	if (!bin_decode_time_t(x,&c->expires)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(c->ua),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(c->path),&s)) goto error;
	
	return c;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (c) {
		if (c->uri.s) shm_free(c->uri.s);
		if (c->ua.s) shm_free(c->ua.s);
		if (c->path.s) shm_free(c->path.s);
		
		shm_free(c);
	}
	return 0;
}


/**
 * Encode a r_subscriber into a binary form
 * @param x - binary data to append to
 * @param s - the r_subscriber to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_r_subscriber(bin_data *x,r_subscriber *s)
{
	if (!bin_encode_str(x,&(s->subscriber))) goto error;
	if (!bin_encode_char(x,s->event)) goto error;
	if (!bin_encode_time_t(x,s->expires)) goto error;
	if (!bin_encode_dlg_t(x,s->dialog)) goto error;
	if (!bin_encode_int(x,s->version)) goto error;

	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_r_subscriber: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a subscriber from a binary data structure
 * @param x - binary data to decode from
 * @returns the r_subscriber* where the data has been decoded
 */
r_subscriber* bin_decode_r_subscriber(bin_data *x)
{
	r_subscriber *s=0;
	int len;
	str st;
	
	len = sizeof(r_subscriber);
	s = (r_subscriber*) shm_malloc(len);
	if (!s) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(s,0,len);
	
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(s->subscriber),&st)) goto error;
	if (!bin_decode_char(x,&(s->event))) goto error;
	if (!bin_decode_time_t(x,&(s->expires))) goto error;
	if (!bin_decode_dlg_t(x,&(s->dialog))) goto error;
	if (!bin_decode_int(x,&(s->version))) goto error;	
	
	return s;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_contact: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (s) {
		if (s->subscriber.s) shm_free(s->subscriber.s);
		if (s->dialog) tmb.free_dlg(s->dialog);
		shm_free(s);
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
	unsigned short k;
	char ch;
	r_contact *c=0;
	r_subscriber *s=0;
	
	if (!bin_encode_str(x,&(p->aor))) goto error;
	if (!bin_encode_str(x,&(p->early_ims_ip))) goto error;	
	ch = p->reg_state;
	if (!bin_encode_char(x,ch)) goto error;
	if (!bin_encode_ims_subscription(x,p->s)) goto error;
	if (!bin_encode_str(x,&(p->ccf1))) goto error;
	if (!bin_encode_str(x,&(p->ccf2))) goto error;
	if (!bin_encode_str(x,&(p->ecf1))) goto error;
	if (!bin_encode_str(x,&(p->ecf2))) goto error;
	
	k=0;
	for(c=p->head;c;c=c->next)
		k++;
	if (!bin_encode_ushort(x,k)) goto error;
	for(c=p->head;c;c=c->next)
		if (!bin_encode_r_contact(x,c)) goto error;	
	
	k=0;
	for(s=p->shead;s;s=s->next)
		k++;
	if (!bin_encode_ushort(x,k)) goto error;
	for(s=p->shead;s;s=s->next)
		if (!bin_encode_r_subscriber(x,s)) goto error;	
	
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_r_public: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a r_public from a binary data structure
 * @param x - binary data to decode from
 * @returns the r_public* where the data has been decoded
 */
r_public* bin_decode_r_public(bin_data *x)
{
	r_public *p=0;
	r_contact *c=0,*cn=0;
	r_subscriber *s,*sn=0;
	int len,i;
	unsigned short k;
	char ch;
	str st;
	
	len = sizeof(r_public);
	p = (r_public*) shm_malloc(len);
	if (!p) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_public: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(p,0,len);
	
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(p->aor),&st)) goto error;
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(p->early_ims_ip),&st)) goto error;
	p->hash = get_aor_hash(p->aor,r_hash_size);
	if (!bin_decode_char(x,&ch)) goto error;
	p->reg_state = ch;
	
	p->s = bin_decode_ims_subscription(x);
	if (!p->s) goto error;

	if (!bin_decode_str(x,&st)||!str_shm_dup(&(p->ccf1),&st)) goto error;
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(p->ccf2),&st)) goto error;
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(p->ecf1),&st)) goto error;
	if (!bin_decode_str(x,&st)||!str_shm_dup(&(p->ecf2),&st)) goto error;
	
	
	if (!bin_decode_ushort(x,&k)) goto error;
	for(i=0;i<k;i++){
		c = bin_decode_r_contact(x);
		if (!c) goto error;
		c->prev = p->tail;
		c->next = 0;
		if (!p->head) p->head = c;
		if (p->tail) p->tail->next = c;
		p->tail = c;
	}

	if (!bin_decode_ushort(x,&k)) goto error;
	for(i=0;i<k;i++){
		s = bin_decode_r_subscriber(x);
		if (!s) goto error;
		s->prev = p->stail;
		s->next = 0;
		if (!p->shead) p->shead = s;
		if (p->stail) p->stail->next = s;
		p->stail = s;
	}
	
	return p;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_r_public: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (p) {
		if (p->aor.s) shm_free(p->aor.s);
		while(p->head){
			c = p->head;
			cn = c->next;
			free_r_contact(c);
			p->head = cn;
		}
		while(p->shead){
			s = p->shead;
			sn = s->next;
			free_r_subscriber(s);
			p->shead = sn;
		}
		shm_free(p);
	}
	return 0;
}








/**
 * Encode an authentication vector into a binary form
 * @param x - binary data to append to
 * @param v - the authentication vector to encode
 * @returns 1 on succcess or 0 on error
 */
 int bin_encode_auth_vector(bin_data *x,auth_vector *v)
{
	char ch;
	if (!bin_encode_int(x,v->item_number)) goto error;
	if (!bin_encode_uchar(x,v->type)) goto error;
	if (!bin_encode_str(x,&(v->authenticate))) goto error;
	if (!bin_encode_str(x,&(v->authorization))) goto error;
	if (!bin_encode_str(x,&(v->ck))) goto error;
	if (!bin_encode_str(x,&(v->ik))) goto error;
	if (!bin_encode_time_t(x,v->expires)) goto error;
	ch = v->status;
	if (!bin_encode_char(x,ch)) goto error;

	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_auth_vector: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode an authentication vector from a binary data structure
 * @param x - binary data to decode from
 * @returns the auth_vector* where the data has been decoded
 */
auth_vector* bin_decode_auth_vector(bin_data *x)
{
	auth_vector *v=0;
	int len;
	char ch;
	str s;
	
	len = sizeof(auth_vector);
	v = (auth_vector*) shm_malloc(len);
	if (!v) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_auth_vector: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(v,0,len);
	
	if (!bin_decode_int(x,&(v->item_number))) goto error;
	if (!bin_decode_uchar(x,&(v->type))) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(v->authenticate),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(v->authorization),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(v->ck),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(v->ik),&s)) goto error;

	if (!bin_decode_time_t(x,	&(v->expires))) goto error;
	if (!bin_decode_char(x,	&ch)) goto error;
	v->status=ch;
	
	return v;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_auth_vector: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (v) {
		if (v->authenticate.s) shm_free(v->authenticate.s);
		if (v->authorization.s) shm_free(v->authorization.s);
		if (v->ck.s) shm_free(v->ck.s);
		if (v->ik.s) shm_free(v->ik.s);
		shm_free(v);
	}
	return 0;
}





/**
 * Encode an authentication userdata into a binary form
 * @param x - binary data to append to
 * @param u - the authentication usedata to encode
 * @returns 1 on succcess or 0 on error
 */
 int bin_encode_auth_userdata(bin_data *x,auth_userdata *u)
{
	unsigned short k=0;
	auth_vector *v;
	
	if (!bin_encode_str(x,&(u->private_identity))) goto error;
	if (!bin_encode_str(x,&(u->public_identity))) goto error;
	if (!bin_encode_time_t(x,u->expires)) goto error;
	
	for(v=u->head;v;v=v->next)
		k++;

	if (!bin_encode_ushort(x,k)) goto error;
	for(v=u->head;v;v=v->next)
		if (!bin_encode_auth_vector(x,v)) goto error;

	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_auth_vector: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode an authentication userdata from a binary data structure
 * @param x - binary data to decode from
 * @returns the auth_userdata* where the data has been decoded
 */
auth_userdata* bin_decode_auth_userdata(bin_data *x)
{
	auth_userdata *u=0;
	auth_vector *v=0,*vn=0;
	int i,len;
	unsigned short k;
	str s;
	
	len = sizeof(auth_userdata);
	u = (auth_userdata*) shm_malloc(len);
	if (!u) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_auth_userdata: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(u,0,len);
	
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(u->private_identity),&s)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(u->public_identity),&s)) goto error;
	u->hash = get_hash_auth(u->private_identity,u->public_identity);	
	if (!bin_decode_time_t(x,	&(u->expires))) goto error;
	
	if (!bin_decode_ushort(x,	&k)) goto error;
	
	for(i=0;i<k;i++){
		v = bin_decode_auth_vector(x);
		if (!v) goto error;
		v->prev = u->tail;
		v->next = 0;
		if (!u->head) u->head = v;
		if (u->tail) u->tail->next = v;
		u->tail = v;
	}
	
	return u;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_auth_userdata: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (u) {
		if (u->private_identity.s) shm_free(u->private_identity.s);
		if (u->public_identity.s) shm_free(u->public_identity.s);
		while(u->head){
			v = u->head;
			vn = v->next;
			if (v->authenticate.s) shm_free(v->authenticate.s);
			if (v->authorization.s) shm_free(v->authorization.s);
			if (v->ck.s) shm_free(v->ck.s);
			if (v->ik.s) shm_free(v->ik.s);
			shm_free(v);
			u->head = v;
		}
		shm_free(u);
	}
	return 0;
}








/**
 * Encode a dialog into a binary form
 * @param x - binary data to append to
 * @param u - the dialog to encode
 * @returns 1 on succcess or 0 on error
 */
int bin_encode_s_dialog(bin_data *x,s_dialog *d)
{
	char ch;
	if (!bin_encode_str(x,&(d->call_id))) goto error;
	ch = d->direction;
	if (!bin_encode_char(x,ch)) goto error;
	
	if (!bin_encode_str(x,&(d->aor))) goto error;
	
	ch = d->method;
	if (!bin_encode_char(x,ch)) goto error;	
	if (!bin_encode_str(x,&(d->method_str))) goto error;
	
	if (!bin_encode_int(x,d->first_cseq)) goto error;	
	if (!bin_encode_int(x,d->last_cseq)) goto error;	

	ch = d->state;
	if (!bin_encode_char(x,ch)) goto error;	

	if (!bin_encode_time_t(x,d->expires)) goto error;
	if (!bin_encode_time_t(x,d->lr_session_expires)) goto error;
	if (!bin_encode_str(x,&(d->refresher))) goto error;
	if (!bin_encode_uchar(x,d->uac_supp_timer)) goto error;

	if (!bin_encode_uchar(x,d->is_releasing)) goto error;
	if (!bin_encode_dlg_t(x,d->dialog_c)) goto error;	
	if (!bin_encode_dlg_t(x,d->dialog_s)) goto error;
	return 1;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_encode_s_dialog: Error while encoding.\n");
	return 0;		
}

/**
 *	Decode a dialog userdata from a binary data structure
 * @param x - binary data to decode from
 * @returns the s_dialog* where the data has been decoded
 */
s_dialog* bin_decode_s_dialog(bin_data *x)
{
	s_dialog *d=0;
	int len;
	str s;
	char ch;
	
	len = sizeof(s_dialog);
	d = (s_dialog*) shm_malloc(len);
	if (!d) {
		LOG(L_ERR,"ERR:"M_NAME":bin_decode_s_dialog: Error allocating %d bytes.\n",len);
		goto error;
	}
	memset(d,0,len);

	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->call_id),&s)) goto error;

	if (!bin_decode_char(x,	&ch)) goto error;
	d->direction = ch;
	
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->aor),&s)) goto error;
		
	if (!bin_decode_char(x,	&ch)) goto error;
	d->method = ch;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->method_str),&s)) goto error;
	
	if (!bin_decode_int(x,	&d->first_cseq)) goto error;
	if (!bin_decode_int(x,	&d->last_cseq)) goto error;

	if (!bin_decode_char(x,	&ch)) goto error;
	d->state = ch;
	
	if (!bin_decode_time_t(x, &d->expires)) goto error;

	if (!bin_decode_time_t(x, &d->lr_session_expires)) goto error;
	if (!bin_decode_str(x,&s)||!str_shm_dup(&(d->refresher),&s)) goto error;
	if (!bin_decode_uchar(x,&d->uac_supp_timer)) goto error;
		
	if (!bin_decode_uchar(x, &d->is_releasing)) goto error;	
	if (!bin_decode_dlg_t(x,&(d->dialog_c))) goto error;
	if (!bin_decode_dlg_t(x,&(d->dialog_s))) goto error;
	
	d->hash = get_s_dialog_hash(d->call_id);		
	
	return d;
error:
	LOG(L_ERR,"ERR:"M_NAME":bin_decode_s_dialog: Error while decoding (at %d (%04x)).\n",x->max,x->max);
	if (d) {
		if (d->call_id.s) shm_free(d->call_id.s);
		if (d->aor.s) shm_free(d->aor.s);
		if (d->method_str.s) shm_free(d->method_str.s);
		if (d->refresher.s) shm_free(d->refresher.s);
		shm_free(d);
	}
	return 0;
}


