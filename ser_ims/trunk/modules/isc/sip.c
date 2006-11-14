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
 * IMS Service Control - SIP Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../../str.h"
#include "../../parser/parse_expires.h"
#include "../../dprint.h"
#include "../../mem/mem.h"
#include "../scscf/scscf_load.h"
#include "../tm/tm_load.h"


#include "sip.h"
#include "mod.h"

extern struct tm_binds isc_tmb;            /**< Structure with pointers to tm funcs 		*/
extern struct scscf_binds isc_scscfb;            /**< Structure with pointers to S-CSCF funcs 		*/

/**
 *	Compare 2 strings
 *  @param x - first string
 *  @param y - second string
 *	@returns 0 if equal, -1 or 1 else
 */
int str2cmp(str x, str y) {
	if (x.len < y.len) return -1;
	if (x.len > y.len) return +1;
	return strncmp(x.s, y.s, x.len);
}

/**
 *	Compare 2 strings case insensitive
 *  @param x - first string
 *  @param y - second string
 *	@returns 0 if equal, -1 or 1 else
 */
int str2icmp(str x, str y) {
	if (x.len < y.len) return -1;
	if (x.len > y.len) return +1;
	return strncasecmp(x.s, y.s, x.len);
}


/**
 * Check if the message is an initial request for a dialog. 
 *		- BYE, PRACK, UPDATE, NOTIFY belong to an already existing dialog
 * @param msg - the message to check
 * @returns 1 if initial, 0 if not
 */
int isc_is_initial_request(struct sip_msg *msg)
{
	if (msg->first_line.type != SIP_REQUEST ) return 0;
	if (strncasecmp(msg->first_line.u.request.method.s,"BYE",3)==0) return 0;
	if (strncasecmp(msg->first_line.u.request.method.s,"ACK",3)==0) return 0;
	if (strncasecmp(msg->first_line.u.request.method.s,"PRACK",5)==0) return 0;		
	if (strncasecmp(msg->first_line.u.request.method.s,"UPDATE",6)==0) return 0;		
	if (strncasecmp(msg->first_line.u.request.method.s,"NOTIFY",6)==0) return 0;					
	return 1;
}


/**
 *	Delete parameters and stuff from uri.
 * @param uri - the string to operate on 
 */
static inline void isc_strip_uri(str *uri)
{
	int i;
	/* Strip the ending */
	i=0;
	while(i<uri->len&&uri->s[i]!='@')
		i++;
	while(i<uri->len&&
			uri->s[i]!=':'&&
			uri->s[i]!='/'&&
			uri->s[i]!='&')
		i++;
	uri->len=i;
}


/**
 * Looks for the P-Asserted-Identity header and extracts its content.
 * @param msg - the SIP message
 * @returns the event value
 */
str cscf_get_asserted_identity(struct sip_msg *msg)
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
		if (h->name.len == 19  &&
			strncasecmp(h->name.s,"P-Asserted-Identity",19)==0)
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
 *	Get the public identity from P-Asserted-Identity, or From if asserted not found.
 * @param msg - the SIP message
 * @param uri - uri to fill into
 * @returns 1 if found, 0 if not
 */
int isc_get_originating_user( struct sip_msg * msg, str *uri )
{
	struct to_body * from;
	*uri = cscf_get_asserted_identity(msg);
	if (!uri->len) {		
		/* Fallback to From header */
		if ( parse_from_header( msg ) == -1 ) {
			LOG(L_ERR,"ERROR:"M_NAME":isc_get_originating_user: unable to extract URI from FROM header\n" );
			return 0;
		}
		if (!msg->from) return 0;
		from = (struct to_body*) msg->from->parsed;
		*uri = from->uri;
		isc_strip_uri(uri);
	}
	DBG("DEBUG:"M_NAME":isc_get_originating_user: From %.*s\n", uri->len,uri->s );
	return 1;
}

/**
 *	Find if user is registered or not => TRUE/FALSE.
 * This uses the S-CSCF registrar to get the state.
 * @param uri - uri of the user to check
 * @returns the reg_state
 */
int isc_is_registered(str *uri)
{
	int result = 0;
	r_public *p;
	
	p = isc_scscfb.get_r_public(*uri);
	if (p) {
		result = p->reg_state;
		isc_scscfb.r_unlock(p->hash);
	}	
	return result;
}

/**
 *	Get terminating type for a user.
 * This uses the S-CSCF registrar to get the state.
 * @param uri - uri of the user to check
 * @returns #IFC_TERMINATING_SESSION if the user is registered or else #IFC_TERMINATING_UNREGISTERED
 */
inline int isc_get_terminating_type(str *uri)
{
	if (isc_is_registered(uri)) return IFC_TERMINATING_SESSION;
		else return IFC_TERMINATING_UNREGISTERED;
}

/**
 * Get the Public Identity from the Request URI of the message.
 * 	returns the result in duplicated pkg
 * @param msg - the SIP message
 * @returns the public identity
 */
str cscf_get_public_identity_from_requri(struct sip_msg *msg)
{
	str pu={0,0};
	
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_INFO,"ERR:"M_NAME":cscf_get_public_identity_from_requri: This ain't a request \n");	
		return pu;
	}
	if (parse_sip_msg_uri(msg)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_public_identity_from_requri: Error parsing requesturi \n");	
		return pu;
	}
	if (msg->parsed_uri.user.len) {
		pu.len = 4 + msg->parsed_uri.user.len + 1 + msg->parsed_uri.host.len;
		pu.s = pkg_malloc(pu.len+1);
		sprintf(pu.s,"sip:%.*s@%.*s",
			msg->parsed_uri.user.len,	
			msg->parsed_uri.user.s,	
			msg->parsed_uri.host.len,	
			msg->parsed_uri.host.s);	
	}else{
		pu.len = 4 + msg->parsed_uri.host.len;
		pu.s = pkg_malloc(pu.len+1);
		sprintf(pu.s,"sip:%.*s",
			msg->parsed_uri.host.len,	
			msg->parsed_uri.host.s);	
	}
	
	LOG(L_DBG,"DBG:"M_NAME":cscf_get_public_identity_from_requri: <%.*s> \n",
		pu.len,pu.s);	
	return pu;
}

/**
 *	Get public identity from Request-URI for terminating.
 * returns in uri the freshly pkg allocated uri - don't forget to free
 * @param msg - the SIP message
 * @param uri - uri to fill into
 * @returns #IMS_USER_REGISTERED if found, else #IMS_USER_NOT_REGISTERED 
 */
int isc_get_terminating_user( struct sip_msg * msg, str *uri )
{
	*uri = cscf_get_public_identity_from_requri(msg);
	if (!uri->len) return IMS_USER_NOT_REGISTERED;
	/*if ( isc_get_terminating_type( uri ) == IFC_TERMINATING_UNREGISTERED )
		DBG("DBG:"M_NAME":isc_get_terminating_type: To UNREGISTERED %.*s\n", uri->len,uri->s);
	else
		DBG("DBG:"M_NAME":isc_get_terminating_type: To REGISTERED %.*s\n", uri->len,uri->s);	*/
	return IMS_USER_REGISTERED;
}


/**
 *	Get the expires header value from a message. 
 * @param msg - the SIP message
 * @returns the expires value or -1 if not found
 */
int isc_get_expires(struct sip_msg *msg)
{	
	if (msg->expires) {
		if (parse_expires(msg->expires) < 0) {
 			LOG(L_ERR, "INFO:ifc:ifc_get_expires:Error while parsing Expires header\n");
		    return -1;
 		}
		 return ((exp_body_t*) msg->expires->parsed)->val;
 	} else {
		return -1;
	}
}


/**
 * Returns the tm transaction identifiers.
 * If no transaction, then creates one
 * @param msg - the SIP message
 * @param hash - where to write the hash
 * @param label - where to write the label
 * @returns 1 on success and creation of a new transaction, 0 if transaction existed,
 * -1 if failure
 */
int cscf_get_transaction(struct sip_msg *msg, unsigned int *hash,unsigned int *label)
{
	
	if (isc_tmb.t_get_trans_ident(msg,hash,label)<0){	
		LOG(L_DBG,"DBG:"M_NAME":cscf_get_transaction: SIP message without transaction. OK - first request\n");
		if (isc_tmb.t_newtran(msg)<0) 
			LOG(L_INFO,"INF:"M_NAME":cscf_get_transaction: Failed creating SIP transaction\n");
		if (isc_tmb.t_get_trans_ident(msg,hash,label)<0){	
			LOG(L_INFO,"INF:"M_NAME":cscf_get_transaction: SIP message still without transaction\n");
			return -1;
		}else {
			LOG(L_DBG,"DBG:"M_NAME":cscf_get_transaction: New SIP message transaction %u %u\n",
				*hash,*label);
			return 1;
		}						
	}else {
		LOG(L_INFO,"INF:"M_NAME":cscf_get_transaction: Transaction %u %u exists."
		"Retransmission?\n",*hash,*label);
		return 0;
	}
}


/** 
 * Returns the corresponding request for a reply, using tm transactions.
 * @param reply - the SIP Reply message
 * @returns the corresponding SIP Request or NULL if not found
 */
struct sip_msg* cscf_get_request_from_reply(struct sip_msg *reply)
{
	struct cell *t;
	t = isc_tmb.t_gett();
	if (!t){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_request_from_reply: Reply without transaction\n");
		return 0;
	}
	return t->uas.request;
}
