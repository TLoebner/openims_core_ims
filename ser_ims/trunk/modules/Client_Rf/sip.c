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
 * Main SIP Operations 
 * 
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * \author Alberto Diez - get_from_tag,get_to_tag,get_from_uri added
 * \author Ancuta Onofrei	andreea dot ancuta dot onofrei -at- fokus dot fraunhofer dot de
 * 
 * Copyright (C) 2005 FhG Fokus
 * 		
 */

/*
 * Please be careful about parsing the content of headers from shared-memory
 * all headers are parsed and stored in tm shared-memory
 * but only via/from/to/cseq/auth contents are parsed and stored to the shm
 * so if other headers' contents need to be parsed, be sure to free the memory  
 * see: 
 * http://lists.sip-router.org/pipermail/sr-dev/2009-December/005314.html
 * http://lists.berlios.de/pipermail/openimscore-cscf/2009-December/002349.html
 * Feb, 1, 2010,  Min Wang ( wang@basis-audionet.com )
 */

#ifndef WHARF

#include "sip.h"

#include "../../mem/mem.h"
#include "../../data_lump.h"
#include "../../data_lump_rpl.h"
#include "../../parser/parse_to.h"
#include "../../parser/parse_from.h"
#include "../../parser/parse_expires.h"
#include "../../parser/parse_via.h"
#include "../../parser/parse_content.h"
#include "../../parser/parse_nameaddr.h"
#include "../../parser/parse_param.h"
#include "../../parser/contact/contact.h"
#include "../../parser/contact/parse_contact.h"

#include "../../modules/tm/tm_load.h"

#include "mod.h"

/**
 * Looks for the Call-ID header
 * @param msg - the sip message
 * @param hr - ptr to return the found hdr_field 
 * @returns the callid value
 */
str cscf_get_call_id(struct sip_msg *msg,struct hdr_field **hr)
{
	struct hdr_field *h;
	str call_id={0,0};
	if (hr) *hr = 0;	
	if (!msg) return call_id;
	if (parse_headers(msg, HDR_CALLID_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_call_id: error parsing headers\n");
		return call_id;
	}
	h = msg->callid;
	if (!h){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_call_id: Header Call-ID not found\n");
		return call_id;
	}
	if (hr) *hr = h;
	call_id = h->body;	
	return call_id;
}

/**
 * Get the local uri from the From header.
 * @param msg - the message to look into
 * @param local_uri - ptr to fill with the value
 * @returns 1 on success or 0 on error
 */  
int cscf_get_from_uri(struct sip_msg* msg,str *local_uri)
{	
	struct to_body* from;

	if (!msg || parse_from_header(msg)<0 || !msg->from || !msg->from->parsed){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_from_uri: error parsing From header\n");
		if (local_uri) {local_uri->s = 0;local_uri->len = 0;}
		return 0;
	}
	from = msg->from->parsed;		
	if (local_uri) *local_uri = from->uri;
	return 1;
	
}

/**
 * Get the local uri from the To header.
 * @param msg - the message to look into
 * @param local_uri - ptr to fill with the value
 * @returns 1 on success or 0 on error
 */  
int cscf_get_to_uri(struct sip_msg* msg,str *local_uri)
{	
	struct to_body* to=	NULL;

	if (!msg || !msg->to || !msg->to->parsed || parse_headers(msg,HDR_TO_F,0)==-1 ){
		LOG(L_ERR,"ERR:"M_NAME":cscf_get_to_uri: error parsing TO header\n");
		if (local_uri) {local_uri->s = 0;local_uri->len = 0;}
		return 0;
	}
	to = msg->to->parsed;		
	if (local_uri) *local_uri = to->uri;
	return 1;
	
}

#endif /* WHARF*/
