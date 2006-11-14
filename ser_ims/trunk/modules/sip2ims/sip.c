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
 * SIP-to-IMS Gateway - SIP Operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "sip.h"

#include "../../mem/mem.h"
#include "../../data_lump.h"
#include "../../data_lump_rpl.h"
#include "../../parser/parse_to.h"
#include "../../parser/parse_expires.h"
#include "../../parser/digest/digest.h"
#include "../../parser/contact/contact.h"
#include "../../parser/parse_content.h"

#include "../tm/tm_load.h"

#include "mod.h"
#include "auth_api.h"

extern struct tm_binds tmb;            /**< Structure with pointers to tm funcs 		*/

/**
 * Adds a header to the message as the first one in the message
 * @param msg - the message to add a header to
 * @param hdr - the header to be filled with the result
 * @returns 1 on succes, 0 on failure
 */
int sip2ims_add_header_first(struct sip_msg *msg, str *hdr)
{
	struct hdr_field *first;
	struct lump* anchor,*l;

	first = msg->headers;
	anchor = anchor_lump(msg, first->name.s - msg->buf, 0 , 0 );

	if (anchor == NULL) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_add_header_first: anchor_lump failed\n");
		return 0;
	}

	if (!(l=insert_new_lump_before(anchor, hdr->s,hdr->len,HDR_ROUTE_T))){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_add_header_first: error creating lump for header\n" );
		return 0;
	}	
 	return 1;
}

/**
 * Adds a header to the message
 * @param msg - the message to add a header to
 * @param hdr - the header to be filled with the result
 * @param type - type of the new header
 * @returns 1 on succes, 0 on failure
 */
int sip2ims_add_header(struct sip_msg *msg, str *hdr,int type)
{
	struct hdr_field *last;
	struct lump* anchor;
	last = msg->headers;
	while(last->next) 
		last = last->next;
	if (!last){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_add_header: no headers found, aborting\n" );
		return 0;
	}
	anchor = anchor_lump(msg, last->name.s + last->len - msg->buf, 0 , 0);
	if (anchor == NULL) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_add_header: anchor_lump failed\n");
		return 0;
	}

	if (!insert_new_lump_after(anchor, hdr->s,hdr->len,type)){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_add_header: error creting lump for header\n" );
		return 0;
	}	
 	return 1;
}
/**
 * Adds a header to the reply message
 * @param msg - the message to add a header to its reply
 * @param hdr - the header to be filled with the result
 * @returns 1 on succes, 0 on failure
 */
int sip2ims_add_header_rpl(struct sip_msg *msg, str *hdr)
{
	if (add_lump_rpl( msg, hdr->s, hdr->len, LUMP_RPL_HDR)==0) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_add_header_rpl: Can't add header <%.*s>\n",
			hdr->len,hdr->s);
 		return 0;
 	}
 	return 1;
}



/**
 * Delete the given header from the given message
 * @param msg - the message to delete the header from
 * @param hdr - the header to delete. !!! Must be contained in the msg->headers
 * @returns 1 on success, 0 on failure
 */
int sip2ims_delete_header(struct sip_msg *msg, struct hdr_field *hdr)
{
	if (!del_lump(msg, hdr->name.s - msg->buf, hdr->len, 0)) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_delete_header: Can't remove header <%.*s>\n",
			hdr->name.len,hdr->name.s);
		return 0;
	}  
	return 1;
}


/**
 * Returns the Private Identity extracted from the Authorization header.
 * If none found there takes the SIP URI in To without the "sip:" prefix
 * TODO - remove the fallback case to the To header
 * @param msg - the SIP message
 * @param realm - the realm to match in an Authorization header
 * @returns the str containing the private id, no mem dup
 */
str sip2ims_get_private_identity(struct sip_msg *msg, str realm)
{
	str pi={0,0};
	struct hdr_field* h=0;
	int ret;

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_private_identity: Error parsing until header Authorization: \n");
		return pi;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_private_identity: Message does not contain Authorization header.\n");
		goto fallback;
	}

	ret = xfind_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_private_identity: Error while looking for credentials.\n");
		goto fallback;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_private_identity: No credentials for this realm found.\n");
			goto fallback;
		}
	
	if (h) pi=((auth_body_t*)h->parsed)->digest.username.whole;
	if (msg->authorization->parsed)
		free_credentials((auth_body_t**)&(msg->authorization->parsed));

	goto done;
		
fallback:
	LOG(L_INFO,"INF:"M_NAME":sip2ims_get_private_identity: Falling back to private_id=stripped(public_id)\n"
		"-> Message did not contain a valid Authorization Header!!! This fallback is deprecated!\n");	
	pi = sip2ims_get_public_identity(msg);
	if (pi.len>4&&strncasecmp(pi.s,"sip:",4)==0) {pi.s+=4;pi.len-=4;}

done:	
	LOG(L_INFO,"INF:"M_NAME":sip2ims_get_private_identity: <%.*s> \n",
		pi.len,pi.s);
	return pi;	
}

/**
 * Returns the Public Identity extracted from the To header
 * @param msg - the SIP message
 * @returns the str containing the private id, no mem dup
 */
str sip2ims_get_public_identity(struct sip_msg *msg)
{
	str pu={0,0};
	struct to_body *to;
	
	if (parse_headers(msg,HDR_TO_F,0)!=0) {
		LOG(L_INFO,"ERR:"M_NAME":sip2ims_get_public_identity: Error parsing until header To: \n");
		return pu;
	}
	
	if ( get_to(msg) == NULL ) {
		to = (struct to_body*) pkg_malloc(sizeof(struct to_body));
		parse_to( msg->to->body.s, msg->to->body.s + msg->to->body.len, to );
        msg->to->parsed = to;
	}
	else to=(struct to_body *) msg->to->parsed;

	pu = to->uri;
	
	LOG(L_INFO,"INF:"M_NAME":sip2ims_get_public_identity: <%.*s> \n",
		pu.len,pu.s);	
	return pu;
}

str sip2ims_p_visited_network_id={"P-Visited-Network-ID",20};
/**
 * Return the P-Visited-Network-ID header
 * @param msg - the SIP message
 * @param h - the header to be filled with the result
 * @returns the str with the header's body
 */
str sip2ims_get_visited_network_id(struct sip_msg *msg, struct hdr_field **h)
{
	str vnid={0,0};
	struct hdr_field *hdr;
	
	*h=0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_DBG,"DBG:"M_NAME":sip2ims_get_public_identity: Error parsing until header EOH: \n");
		return vnid;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len==sip2ims_p_visited_network_id.len &&
			strncasecmp(hdr->name.s,sip2ims_p_visited_network_id.s,hdr->name.len)==0)
		{
			*h = hdr;
			vnid = hdr->body;
			goto done;
		}
		hdr = hdr->next;
	}
	LOG(L_DBG,"DBG:"M_NAME":sip2ims_get_visited_network_id: P-Visited-Network-ID header not found \n");
	
done:	
	LOG(L_DBG,"DBG:"M_NAME":sip2ims_get_visited_network_id: <%.*s> \n",
		vnid.len,vnid.s);	
	return vnid;
}


/**
 * Returns the expires value from the Expires header in the message.
 * It searches into the Expires header and if not found returns -1
 * @param msg - the SIP message, if available
 * @returns the value of the expire or -1 if not found
 */
int sip2ims_get_expires_hdr(struct sip_msg *msg)
{
	exp_body_t *exp;
	int expires=-1;
	if (!msg) return -1;
	/*first search in Expires header */
	if (parse_headers(msg,HDR_EXPIRES_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_expires_hdr: Error parsing until header EXPIRES: \n");
		return -1;
	}
	
	if (msg->expires){
		parse_expires(msg->expires);
		msg->expires->type = HDR_EXPIRES_T;
		if (msg->expires->parsed) {
			exp = (exp_body_t*) msg->expires->parsed;
			if (exp->valid) {
				expires = exp->val;
				LOG(L_INFO,"INF:"M_NAME":sip2ims_get_expires_hdr: <%d> \n",expires);				
			}
			free_expires((exp_body_t**)&(msg->expires->parsed));
		}
	}
	return expires;
}

/**
 * Returns the expires value from the message.
 * First it searches into the Expires header and if not found it also looks 
 * into the expires parameter in the contact header
 * @param msg - the SIP message
 * @returns the value of the expire or the default 3600 if none found
 */
int sip2ims_get_expires(struct sip_msg *msg)
{
	exp_body_t *exp;
	int expires = 3600,i;
	struct to_body contact;
	struct to_param *param;
	/*first search in Expires header */
	if (parse_headers(msg,HDR_EXPIRES_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_expires: Error parsing until header EXPIRES: \n");
		//don'r return, look for contact
	}
	
	if (msg->expires){
		parse_expires(msg->expires);
		msg->expires->type = HDR_EXPIRES_T;
		if (msg->expires->parsed) {
			exp = (exp_body_t*) msg->expires->parsed;
			if (exp->valid) {
				expires = exp->val;
				LOG(L_INFO,"INF:"M_NAME":sip2ims_get_expires: <%d> \n",expires);
				return expires;
			}
		}
	}
	/* then search in contact header parameter expires */
	if (parse_headers(msg,HDR_CONTACT_F,0)!=0) {
		LOG(L_INFO,"ERR:"M_NAME":sip2ims_get_expires: Error parsing until header CONTACT: \n");
		LOG(L_INFO,"INF:"M_NAME":sip2ims_get_expires: <%d> \n",expires);
		return expires;
	}
	
	if (msg->contact) {
		parse_to(msg->contact->body.s,msg->contact->body.s+msg->contact->body.len,
			&contact);
		if (contact.param_lst){
			param = contact.param_lst;
			while(param){
				if (strncasecmp(param->name.s,"expires",7)==0){
					expires = 0;
					for(i=0;i<param->name.len;i++)
					 if (param->name.s[i]>='0' &&
					 	 param->name.s[i]<='9')
					 	expires=(expires*10)+param->name.s[i]-'0';
				}
				param = param->next;
			}
		}
		free_to(&contact);
	}	
	LOG(L_INFO,"INF:"M_NAME":sip2ims_get_expires: <%d> \n",expires);
	return expires;
}

/**
 * Get the Public Identity from the Request URI of the message
 * @param msg - the SIP message
 * @returns the public identity
 */
str sip2ims_get_public_identity_from_requri(struct sip_msg *msg)
{
	str pu={0,0};
	
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_ERR,"INF:"M_NAME":sip2ims_get_public_identity_from_requri: This ain't a request \n");	
		return pu;
	}
	if (parse_sip_msg_uri(msg)<0){
		LOG(L_ERR,"INF:"M_NAME":sip2ims_get_public_identity_from_requri: Error parsing requesturi \n");	
		return pu;
	}
	
	pu.len = 4 + msg->parsed_uri.user.len + 1 + msg->parsed_uri.host.len;
	pu.s = shm_malloc(pu.len+1);
	sprintf(pu.s,"sip:%.*s@%.*s",
		msg->parsed_uri.user.len,	
		msg->parsed_uri.user.s,	
		msg->parsed_uri.host.len,	
		msg->parsed_uri.host.s);	
	
	LOG(L_INFO,"INF:"M_NAME":sip2ims_get_public_identity_from_requri: <%.*s> \n",
		pu.len,pu.s);	
	return pu;
}

/**
 * Retrieves the SIP request that generated a diameter transaction
 * @param hash - tm module hash value
 * @param label - tm module label value
 * @return the SIP request
 */
struct sip_msg *sip2ims_get_request(unsigned int hash,unsigned int label)
{
	struct cell *c;
	if (tmb.t_lookup_ident(&c,hash,label)<0){
		LOG(L_INFO,"INF:"M_NAME":Cx_UAA_timeout: No transaction found for %u %u\n",
			hash,label);
		return 0;
	}
	return c->uas.request;
}

/**
 * Returns if the SIP message should be integrity protected.
 * The integrity-protected field in the in the Authorization header
 * TODO - optimize it by including the integrity protected parameter in the
 * digest parsed structures - will change the core though...
 * @param msg - the SIP message
 * @param realm - the realm
 * @returns 1 if integrity protected is set to yes, 0 if it is not set to yes or is
 * missing
 */
int sip2ims_get_integrity_protected(struct sip_msg *msg,str realm)
{
	str ip={"integrity-protected",19};
	struct hdr_field* h=0;
	int ret,i,j;

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_integrity_protected: Error parsing until header Authorization: \n");
		return 0;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_integrity_protected: Message does not contain Authorization header.\n");
		return 0;
	}
	msg->authorization->type = HDR_AUTHORIZATION_T;
	ret = xfind_credentials_noparse(msg, &realm, HDR_AUTHORIZATION_F, &h);
//	if (msg->authorization->parsed)
//		free_credentials((auth_body_t**)&(msg->authorization->parsed));
	
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_integrity_protected: Error while looking for credentials.\n");
		return 0;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_integrity_protected: No credentials for this realm found.\n");
			return 0;
		}
	
	if (h) {
		for(i=0;i<h->body.len-ip.len;i++)
			if (strncasecmp(h->body.s+i,ip.s,ip.len)==0)
				for(j=i+ip.len;j<h->body.len;j++){
					if (h->body.s[j]=='y' || h->body.s[j]=='Y') return 1;
					if (h->body.s[j]==' ') return 0;
				}			
	}
	
	return 0;
}


/**
 * Returns the transaction identifiers.
 * If no transaction, then creates one
 * @param msg - the SIP message
 * @param hash - where to write the hash
 * @param label - where to write the label
 * @returns 1 on success and creation of a new transaction, 0 if transaction existed,
 * -1 if failure
 */
int sip2ims_get_transaction(struct sip_msg *msg, unsigned int *hash,unsigned int *label)
{
	
	if (tmb.t_get_trans_ident(msg,hash,label)<0){	
		LOG(L_DBG,"DBG:"M_NAME":sip2ims_get_transaction: SIP message without transaction. OK - first request\n");
		if (tmb.t_newtran(msg)<0) 
			LOG(L_INFO,"INF:"M_NAME":sip2ims_get_transaction: Failed creating SIP transaction\n");
		if (tmb.t_get_trans_ident(msg,hash,label)<0){	
			LOG(L_INFO,"INF:"M_NAME":sip2ims_get_transaction: SIP message still without transaction\n");
			return -1;
		}else {
			LOG(L_DBG,"DBG:"M_NAME":sip2ims_get_transaction: New SIP message transaction %u %u\n",
				*hash,*label);
			return 1;
		}						
	}else {
		LOG(L_INFO,"INF:"M_NAME":sip2ims_get_transaction: Transaction %u %u exists."
		"Retransmission?\n",*hash,*label);
		return 0;
	}
}

/**
 * Looks for the auts parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @returns 1 the auts
 */
str sip2ims_get_auts(struct sip_msg *msg, str realm)
{
	str name={"auts=\"",6};
	struct hdr_field* h=0;
	int i,ret;
	str auts={0,0};

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_auts: Error parsing until header Authorization: \n");
		return auts;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_auts: Message does not contain Authorization header.\n");
		return auts;
	}

	ret = xfind_credentials_noparse(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (msg->authorization->parsed)
		free_credentials((auth_body_t**)&(msg->authorization->parsed));
	
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_auts: Error while looking for credentials.\n");
		return auts;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_auts: No credentials for this realm found.\n");
			return auts;
		}
	
	if (h) {
		for(i=0;i<h->body.len-name.len;i++)
			if (strncasecmp(h->body.s+i,name.s,name.len)==0){
				auts.s = h->body.s+i+name.len;
				while(i+auts.len<h->body.len && auts.s[auts.len]!='\"')
					auts.len++;
			}
	}
	
	return auts;	
}

static str www_authenticate={"WWW-Authenticate",16};
static str nonce_s={"nonce=\"",7};
/**
 * Looks for the nonce parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @returns the nonce
 */
str sip2ims_get_nonce(struct sip_msg *msg, str realm)
{
	struct hdr_field* h=0;
	int i;
	str nonce={0,0};

	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_nonce: Error parsing until header WWW-Authenticate: \n");
		return nonce;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==www_authenticate.len &&
			strncasecmp(h->name.s,www_authenticate.s,www_authenticate.len)==0){
			for(i=0;i<h->body.len-nonce_s.len;i++)
			if (strncasecmp(h->body.s+i,nonce_s.s,nonce_s.len)==0){
				nonce.s = h->body.s+i+nonce_s.len;
				while(i+nonce.len<h->body.len && nonce.s[nonce.len]!='\"')
					nonce.len++;
				return nonce;	
			}		
		}

		h = h->next;
	}
	LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_nonce: Message does not contain WWW-Authenticate header.\n");
		return nonce;
}


/**
 * Looks for the nonce and response parameters in the Authorization header and returns them
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @param nonce - nonce to fill with the findings
 * @param response - response to fill with the findings
 * @returns the nonce
 */
int sip2ims_get_nonce_response(struct sip_msg *msg, str realm,str *nonce,str *response)
{
	struct hdr_field* h=0;
	int ret;

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_nonce: Error parsing until header Authorization: \n");
		return 0;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_nonce: Message does not contain Authorization header.\n");
		return 0;
	}

	ret = xfind_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_nonce: Error while looking for credentials.\n");
		return 0;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_nonce: No credentials for this realm found.\n");
			return 0;
		}
	
	if (h&&h->parsed) {
		*nonce = ((auth_body_t*)h->parsed)->digest.nonce;
		*response = ((auth_body_t*)h->parsed)->digest.response;
	}
	if (msg->authorization->parsed)
		free_credentials((auth_body_t**)&(msg->authorization->parsed));
	
	
	return 1;	
}


str ua_dummy={"Unknown UA",10};
str ua_nomsg={"No Message",10};
/**
 * Looks for the UserAgent header and extracts its content
 * @param msg - the sip message
 * @returns the user agent string, or ua_dummy if not found, or ua_nomsg if msg was
 * NULL
 */
str sip2ims_get_user_agent(struct sip_msg *msg)
{
	str ua;
	if (!msg) return ua_nomsg;
	ua.len = 0;
	if (parse_headers(msg, HDR_USERAGENT_F, 0) != -1 && msg->user_agent &&
	    msg->user_agent->body.len > 0) 
	{
		ua.len = msg->user_agent->body.len;
		ua.s = msg->user_agent->body.s;
	}
	if (ua.len == 0) ua=ua_dummy;
	return ua;
}

/**
 * Parses all the contact headers
 * @param msg - the SIP message
 * @returns the first contact_body
 */
contact_body_t *sip2ims_parse_contacts(struct sip_msg *msg)
{
	struct hdr_field* ptr;
		
	if (!msg) return 0;
	if (msg->contact) {
		ptr = msg->contact;
		while(ptr) {
			if (ptr->type == HDR_CONTACT_T) {
				if (!ptr->parsed && (parse_contact(ptr) < 0)) {
					LOG(L_ERR,"ERR:"M_NAME":sip2ims_parse_contacts: error parsing contacts\n");
					return 0;
				}
				else {
//					contact_body_t *b
//					b = (contact_body_t*) ptr->parsed;
//					c = b->contacts;
//					while(c){
//						LOG(L_ERR,"DBG:"M_NAME":sip2ims_parse_contacts: Contact: <%.*s>\n",c->uri.len,c->uri.s);
//						c = c->next;
//					}
				}
					
			}
			ptr = ptr->next;
		}
	}
	if (!msg->contact) return 0;
	return msg->contact->parsed;
}

/**
 * Retrieve a list of concatenated contents of Path headers found in the message
 * @param msg - the SIP message containing the Path headers
 * @returns pkg mem allocated str with the headers values separated by ','
 */
str sip2ims_get_path(struct sip_msg *msg)
{
	str path={0,0};
	struct hdr_field *h;
	if (!msg) return path;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_path: error parsing headers\n");
		return path;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==4 &&
			strncasecmp(h->name.s,"Path",4)==0){
				path.len+=h->body.len+1;
			}
		h = h->next;
	}
	path.s = pkg_malloc(path.len);
	if (!path.s){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_path: error allocating %d bytes\n",
			path.len);
		path.len=0;
		return path;
	}
	h = msg->headers;
	path.len=0;
	while(h){
		if (h->name.len==4 &&
			strncasecmp(h->name.s,"Path",4)==0){
				if (path.len) path.s[path.len++]=',';
				memcpy(path.s+path.len,h->body.s,h->body.len);
				path.len+=h->body.len;
			}
		h = h->next;
	}

	return path;
}

/**
 * Looks for the Event header and extracts its content
 * @param msg - the sip message
 * @returns the event value
 */
str sip2ims_get_event(struct sip_msg *msg)
{
	str e={0,0};
	if (!msg) return e;
	if (parse_headers(msg, HDR_EVENT_F, 0) != -1 && msg->event &&
	    msg->event->body.len > 0) 
	{
		e.len = msg->event->body.len;
		e.s = msg->event->body.s;
	}
	return e;
}

/**
 * Looks for the P-Asserted-Identity header and extracts its content
 * @param msg - the sip message
 * @returns the event value
 */
str sip2ims_get_asserted_identity(struct sip_msg *msg)
{
	str id={0,0};
	struct hdr_field *h;
	
	if (!msg) return id;
	if (parse_headers(msg, HDR_EOH_F, 0)<0) {
		return id;
	}
	h = msg->headers;
	while(h)
	{
		if (h->name.len == 18  &&
			strncasecmp(h->name.s,"P-Asserted-Identity",18)==0)
		{
			id = h->body;
			while(id.len && (id.s[0]==' ' || id.s[0]=='\t' || id.s[0]=='<')){
				id.s = id.s+1;
				id.len --;
			}
			while(id.len && (id.s[id.len-1]==' ' || id.s[id.len-1]=='\t' || id.s[id.len-1]=='>')){
				id.len--;
			}	
			return id;
		}
		h = h->next;
	}
	return id;
}



/**
 * Looks for the Authorization header and returns its body.
 * @param msg - the SIP message
 * @param h - the header to be filled with the result
 * @returns the authorization body
 */
str sip2ims_get_authorization(struct sip_msg *msg,struct hdr_field **h)
{
	str auth={0,0};
	*h = 0;
	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_authorization: Error parsing until header Authorization: \n");
		return auth;
	}

	if (!msg->authorization){
		LOG(L_DBG, "DBG:"M_NAME":sip2ims_get_authorization: Message does not contain Authorization header.\n");
		return auth;
	}
//	msg->authorization->type = HDR_AUTHORIZATION_T;
//	if (msg->authorization->parsed)
//		free_credentials((auth_body_t**)&(msg->authorization->parsed));
	
	auth = msg->authorization->body;	
	*h = msg->authorization;
		
	return auth;	
}

/**
 * Looks for the WWW-Authenticate header and returns its body.
 * @param msg - the SIP message
 * @param h - the header to be filled with the result
 * @returns the authorization body
 */
str sip2ims_get_authenticate(struct sip_msg *msg,struct hdr_field **h)
{
	str auth={0,0};
	struct hdr_field *hdr;
	*h = 0;
	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_authorization: Error parsing until header WWW-Authenticate: \n");
		return auth;
	}
	hdr = msg->headers;
	while(hdr){
		if (hdr->name.len ==16  &&
			strncasecmp(hdr->name.s,"WWW-Authenticate",16)==0)
		{
			*h = hdr;
			auth = hdr->body;
			break;
		}
		hdr = hdr->next;
	}
	if (!hdr){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_authorization: Message does not contain WWW-Authenticate header.\n");
		return auth;
	}

	return auth;	
}


/**
 * Deletes the given header
 * @param msg - the SIP message
 * @param h - the header to be filled with the result
 * @returns 1 on success, 0 on error
 */
int sip2ims_del_header(struct sip_msg *msg,struct hdr_field *h)
{
	if (!h||!h->name.s){
		LOG(L_DBG, "DBG:"M_NAME":sip2ims_del_header: no header specified.\n");
		return 1;
	}

	if (!del_lump(msg,h->name.s-msg->buf,h->len,0)){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_del_header: Error adding del lump\n");
		return 0;		
	}		
	return 1;	
}



/**
 * Looks for the First Via header and returns its body.
 * @param msg - the SIP message
 * @param h - the header to be filled with the result
 * @returns the authorization body
 */
str sip2ims_get_first_via(struct sip_msg *msg,struct hdr_field **h)
{
	str uri={0,0};
	if (h) *h = 0;
	
	if (!msg->h_via1 && parse_headers(msg,HDR_VIA_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_first_via: Error parsing until header Via: \n");
		return uri;
	}

	if (!msg->via1){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_first_via: Message does not contain Via header.\n");
		return uri;
	}

	uri = msg->via1->host;
	//TODO - add protocol and port into identification
	if (h) *h = msg->h_via1;
		
	return uri;	
}


static str realm_p={"realm=\"",7};
/**
 * Looks for the realm parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @returns the nonce
 */
str sip2ims_get_realm(struct sip_msg *msg)
{
	struct hdr_field* h=0;
	int i;
	str realm={0,0};

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_realm: Error parsing until header Authenticate: \n");
		return realm;
	}
	h = msg->authorization;
	if (!h){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_realm: Message does not contain Authenticate header.\n");
			return realm;
	}	
	for(i=0;i<h->body.len-realm_p.len;i++)
		if (strncasecmp(h->body.s+i,realm_p.s,realm_p.len)==0){
			realm.s = h->body.s+i+realm_p.len;
			while(i+realm.len<h->body.len && realm.s[realm.len]!='\"')
				realm.len++;
			return realm;	
		}		
	

	return realm;
}

static str opaque_p={"opaque=\"",8};
/**
 * Looks for the realm parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @returns the nonce
 */
str sip2ims_get_opaque(struct sip_msg *msg)
{
	struct hdr_field* h=0;
	int i;
	str opaque={0,0};

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_opaque: Error parsing until header Authenticate: \n");
		return opaque;
	}
	h = msg->authorization;
	if (!h){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_opaque: Message does not contain Authenticate header.\n");
			return opaque;
	}	
	for(i=0;i<h->body.len-opaque_p.len;i++)
		if (strncasecmp(h->body.s+i,opaque_p.s,opaque_p.len)==0){
			opaque.s = h->body.s+i+opaque_p.len;
			while(i+opaque.len<h->body.len && opaque.s[opaque.len]!='\"')
				opaque.len++;
			return opaque;	
		}		
	return opaque;
}


/**
 * Returns the content of the P-Associated-URI header
 * public_id is pkg_alloced and should be later freed
 * inside values are not duplicated
 */
int sip2ims_get_p_associated_uri(struct sip_msg *msg,str **public_id,int *public_id_cnt)
{
	struct hdr_field *h;
	rr_t *r,*r2;
	*public_id = 0;
	*public_id_cnt = 0;
	
	if (!msg) return 0;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_p_associated_uri: error parsing headers\n");
		return 0;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==16 &&
			strncasecmp(h->name.s,"P-Associated-URI",16)==0){
				break;
			}
		h = h->next;
	}
	if (!h){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_p_associated_uri: Header P-Associated-URI not found\n");
		return 0;
	}
	if (parse_rr(h)<0){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_p_associated_uri: Error parsing as Route header\n");
		return 0;
	}
	r = (rr_t*)h->parsed;
	h->type = HDR_ROUTE_T;
	*public_id_cnt=0;
	r2 = r;
	while(r2){
		(*public_id_cnt) = (*public_id_cnt)+1;
		r2 = r2->next;
	}
	*public_id = pkg_malloc(sizeof(str)*(*public_id_cnt));
	if (!public_id){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_p_associated_uri: Error allocating %d bytes\n",
			sizeof(str)*(*public_id_cnt));
		return 0;
	}
	r2 = r;
	*public_id_cnt=0;
	while(r2){
		(*public_id)[(*public_id_cnt)]=r2->nameaddr.uri;
		(*public_id_cnt) = (*public_id_cnt)+1;
		r2 = r2->next;
	}
	
	return 1;
}

/**
 * Looks for the P-Preferred-Identity header and extracts its content
 * @param msg - the sip message
 * @param hr - the header to be filled with the result
 * @returns the event value
 */
str sip2ims_get_preferred_identity(struct sip_msg *msg,struct hdr_field **hr)
{
	str id={0,0};
	struct hdr_field *h;
	*hr=0;
	if (!msg) return id;
	if (parse_headers(msg, HDR_EOH_F, 0)<0) {
		return id;
	}
	h = msg->headers;
	while(h)
	{
		if (h->name.len == 20  &&
			strncasecmp(h->name.s,"P-Preferred-Identity",20)==0)
		{
			id = h->body;
			while(id.len && (id.s[0]==' ' || id.s[0]=='\t' || id.s[0]=='<')){
				id.s = id.s+1;
				id.len --;
			}
			while(id.len && (id.s[id.len-1]==' ' || id.s[id.len-1]=='\t' || id.s[id.len-1]=='>')){
				id.len--;
			}	
			*hr = h;
			return id;
		}
		h = h->next;
	}
	return id;
}

/**
 * Looks for the First Route header
 * @param msg - the sip message
 * @param hr - the header to be filled with the result
 * @returns the event value
 */
str sip2ims_get_first_route(struct sip_msg *msg,struct hdr_field **hr)
{
	struct hdr_field *h;
	rr_t *r;
	str route={0,0};
	if (hr) *hr = 0;	
	if (!msg) return route;
	if (parse_headers(msg, HDR_ROUTE_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_first_route: error parsing headers\n");
		return route;
	}
	h = msg->route;
	if (!h){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_first_route: Header Route not found\n");
		return route;
	}
	if (hr) *hr = h;
	if (parse_rr(h)<0){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_first_route: Error parsing as Route header\n");
		return route;
	}
	r = (rr_t*)h->parsed;
	route = r->nameaddr.uri;
	
	return route;
}


static str route_hdr_s={"Route: ",7};
static str route_hdr_e={"\r\n",2};
/**
 * Looks if the first entry in the first Route points is equal to the given value and
 * deletes the old header. If there are more entries inserts a new header at top with those values.
 * @param msg - the SIP message
 * @param value - the value to look for
 * @returns 1 if removed, else 0
 */
int sip2ims_remove_first_route(struct sip_msg *msg,str value)
{
	struct hdr_field *h;
	str route={0,0},x;
	int i;
		
	route = sip2ims_get_first_route(msg,&h);
	if (!h||!route.len) return 0;
	
	if ((route.len == value.len || (route.len>value.len && route.s[value.len]==';')) &&
		strncasecmp(route.s,value.s,value.len)==0)
	{
		sip2ims_delete_header(msg,h);
		route = h->body;
		i=0;
		while(i<route.len && route.s[i]!=',')
			i++;
		i++;
		if (i<route.len){
			route.s+=i;
			route.len-=i;
			x.s = pkg_malloc(route_hdr_s.len + route.len +route_hdr_e.len);
			if (!x.s){
				LOG(L_ERR, "ERR"M_NAME":sip2ims_remove_first_route: Error allocating %d bytes\n",
					route.len);
				x.len=0;
			}else{
				x.len = 0;
				STR_APPEND(x,route_hdr_s);
				STR_APPEND(x,route);
				STR_APPEND(x,route_hdr_e);				
				if (!sip2ims_add_header_first(msg,&x))
					pkg_free(x.s);
			}
		}
	}
	
	return 1;
}

/**
 * Returns if the uri is of myself
 */
inline int sip2ims_is_myself(str uri)
{
	int ret;
	struct sip_uri puri;
	
	if (parse_uri(uri.s,uri.len,&puri)<0){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_is_myself: error parsing uri <%.*s>\n",
			uri.len,uri.s);
		return 0;			
	}
	
	ret = check_self(&(puri.host), puri.port_no ? puri.port_no : SIP_PORT, 0);
	if (ret < 0) return 0;
	
	return ret;
}
/**
 * Looks if the first entry in the first Route points to myself and if positive, it removes it
 * deletes the old header. If there are more entries inserts a new header at top with those values.
 * @param msg - the SIP message
 * @param h - the header to be filled with the result
 * @returns 1 if removed, else 0
 */
int sip2ims_remove_own_route(struct sip_msg *msg,struct hdr_field **h)
{
	str route={0,0},x;
	int i;
		
	route = sip2ims_get_first_route(msg,h);
	if (!h||!route.len) return 0;
	
	LOG(L_INFO,"DBG:"M_NAME":sip2ims_remove_own_route: <%.*s>\n",
		route.len,route.s);
	if (sip2ims_is_myself(route))
	{
		sip2ims_delete_header(msg,*h);
		route = (*h)->body;
		i=0;
		while(i<route.len && route.s[i]!=',')
			i++;
		i++;
		if (i<route.len){
			route.s+=i;
			route.len-=i;
			x.s = pkg_malloc(route_hdr_s.len + route.len +route_hdr_e.len);
			if (!x.s){
				LOG(L_ERR, "ERR"M_NAME":sip2ims_remove_own_route: Error allocating %d bytes\n",
					route.len);
				x.len=0;
			}else{
				x.len = 0;
				STR_APPEND(x,route_hdr_s);
				STR_APPEND(x,route);
				STR_APPEND(x,route_hdr_e);				
				if (!sip2ims_add_header_first(msg,&x))
					pkg_free(x.s);
			}
		}
	}
	
	return 1;
}

/**
 * Looks for the Content-Type header and extracts its content
 * @param msg - the sip message
 * @returns the content-type string, or {0,0} if not found
 */
str sip2ims_get_content_type(struct sip_msg *msg)
{
	str ct={0,0};
	if (!msg) return ct;
	if (parse_headers(msg, HDR_CONTENTTYPE_F, 0) != -1 && msg->content_type)
		ct = msg->content_type->body;		
	return ct;
}

/**
 * Looks for the Content-length header and extracts its content
 * @param msg - the sip message
 * @returns the content-type string, or {0,0} if not found
 */
int sip2ims_get_content_len(struct sip_msg *msg)
{
	int cl=0;
	if (!msg) return 0;
	if (parse_headers(msg, HDR_CONTENTLENGTH_F, 0) != -1 && msg->content_length &&
			msg->content_length->parsed)
		cl = get_content_length(msg);		
	return cl;
}


/**
 * Looks for the Subscription-State header and extracts its expires if active
 * @param msg - the sip message
 * @returns the expires value if active, 0 if terminated, -1 if not found
 */
int sip2ims_get_subscription_state(struct sip_msg *msg)
{
	int exp=-1;
	int i=0;
	struct hdr_field *h;

	if (!msg) return exp;
	if (parse_headers(msg, HDR_EOH_F, 0)<0) {
		return exp;
	}
	h = msg->headers;
	while(h)
	{
		if (h->name.len == 18  &&
			strncasecmp(h->name.s,"Subscription-State",18)==0)
		{
			while(i<h->body.len && h->body.s[i]!='=')
				i++;
			if (i<h->body.len){
				exp = atoi(h->body.s+i+1);				
			} else 
				exp =0;
			return exp;
		}
		h = h->next;
	}
	return exp;
}

/**
 * Returns the content of the Service-Route header.
 * data vector is pkg_alloced and should be later freed
 * inside values are not duplicated
 * @param msg - the SIP message
 * @param size - size of the returned vector, updated
 * @returns - the str vector of uris
 */
str* sip2ims_get_service_route(struct sip_msg *msg,int *size)
{
	struct hdr_field *h;
	rr_t *r,*r2;
	str *x = 0;
	int k;
	if (!size) return 0;
	
	*size=0;
		
	if (!msg) return 0;
	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_service_route: error parsing headers\n");
		return 0;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==13 &&
			strncasecmp(h->name.s,"Service-Route",13)==0)
		{
			if (parse_rr(h)<0){
				LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_service_route: Error parsing as Route header\n");
				continue;
			}
			r = (rr_t*)h->parsed;
			h->type = HDR_ROUTE_T;
			r2 = r; k=0;
			while(r2){
				k++;
				r2 = r2->next;
			}
			if (!k){
				LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_service_route: No items in this Service-Route\n");
				continue;
			}
			x = pkg_realloc(x,(*size+k)*sizeof(str));
			if (!x){
				LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_service_route: Error reallocating to %d bytes\n",
					(*size+k)*sizeof(str));
				return 0;
			}
			r2 = r; 
			while(r2){
				x[*size] = r2->nameaddr.uri;
				(*size) = (*size)+1;
				r2 = r2->next;
			}			
		}
		h = h->next;
	}
	
	return x;
}



/**
 * Looks for the uri parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @returns the nonce
 */
str sip2ims_get_digest_uri(struct sip_msg *msg, str realm)
{
	struct hdr_field* h=0;
	int ret;
	str uri={0,0};

	if (parse_headers(msg,HDR_AUTHORIZATION_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_digest_uri: Error parsing until header Authorization: \n");
		return uri;
	}

	if (!msg->authorization){
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_digest_uri: Message does not contain Authorization header.\n");
		return uri;
	}

	ret = find_credentials(msg, &realm, HDR_AUTHORIZATION_F, &h);
	if (ret < 0) {
		LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_digest_uri: Error while looking for credentials.\n");
		return uri;
	} else 
		if (ret > 0) {
			LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_digest_uri: No credentials for this realm found.\n");
			return uri;
		}
	
	if (h&&h->parsed) {
		uri = ((auth_body_t*)h->parsed)->digest.uri;
	}
	return uri;	
}


static str algorithm_st={"algorithm=",10};
/**
 * Looks for the algorithm parameter in the Authorization header and returns its value.
 * @param msg - the SIP message
 * @param realm - realm to match the right Authorization header
 * @returns the nonce
 */
str sip2ims_get_algorithm(struct sip_msg *msg, str realm)
{
	struct hdr_field* h=0;
	int i;
	str alg={0,0};

	if (parse_headers(msg,HDR_EOH_F,0)!=0) {
		LOG(L_ERR,"ERR:"M_NAME":sip2ims_get_algorithm: Error parsing until header WWW-Authenticate: \n");
		return alg;
	}
	h = msg->headers;
	while(h){
		if (h->name.len==www_authenticate.len &&
			strncasecmp(h->name.s,www_authenticate.s,www_authenticate.len)==0){
			for(i=0;i<h->body.len-algorithm_st.len;i++)
			if (strncasecmp(h->body.s+i,algorithm_st.s,algorithm_st.len)==0){
				alg.s = h->body.s+i+algorithm_st.len;
				if (alg.s[0]=='\"'){alg.s++;i++;};
				while(i+alg.len<h->body.len && 
					alg.s[alg.len]!=','&& 
					alg.s[alg.len]!=' '&& 
					alg.s[alg.len]!='\t'&& 
					alg.s[alg.len]!='\"'&&
					alg.s[alg.len]!='\n'&&
					alg.s[alg.len]!='\r')
					alg.len++;
				return alg;	
			}		
		}

		h = h->next;
	}
	LOG(L_ERR, "ERR:"M_NAME":sip2ims_get_algorithm: Message does not contain WWW-Authenticate header.\n");
	return alg;
}
