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
  
#ifndef SIP2IMS_SIP_H
#define SIP2IMS_SIP_H

#include "../../sr_module.h"
#include "../../parser/contact/parse_contact.h"

int sip2ims_add_header_first(struct sip_msg *msg, str *hdr);
int sip2ims_add_header(struct sip_msg *msg, str *hdr,int type);
int sip2ims_add_header_rpl(struct sip_msg *msg, str *hdr);
int sip2ims_add_contact(struct sip_msg *msg,str uri,int expires);
int sip2ims_delete_header(struct sip_msg *msg, struct hdr_field *hdr);

str sip2ims_get_private_identity(struct sip_msg *msg, str realm);
str sip2ims_get_public_identity(struct sip_msg *msg);
int sip2ims_get_expires_hdr(struct sip_msg *msg);
int sip2ims_get_expires(struct sip_msg *msg);
str sip2ims_get_public_identity_from_requri(struct sip_msg *msg);

struct sip_msg *sip2ims_get_request(unsigned int hash,unsigned int label);

int sip2ims_get_integrity_protected(struct sip_msg *msg,str realm);
int sip2ims_get_transaction(struct sip_msg *msg, unsigned  int *hash, unsigned int *label);
str sip2ims_get_nonce(struct sip_msg *msg, str realm);
int sip2ims_get_nonce_response(struct sip_msg *msg, str realm,str *nonce,str *response);
str sip2ims_get_user_agent(struct sip_msg *msg);
contact_body_t *sip2ims_parse_contacts(struct sip_msg *msg);
str sip2ims_get_path(struct sip_msg *msg);
str sip2ims_get_event(struct sip_msg *msg);
str sip2ims_get_asserted_identity(struct sip_msg *msg);

str sip2ims_get_visited_network_id(struct sip_msg *msg, struct hdr_field **h);

str sip2ims_get_authorization(struct sip_msg *msg, struct hdr_field **h);
str sip2ims_get_authenticate(struct sip_msg *msg,struct hdr_field **h);

int sip2ims_del_header(struct sip_msg *msg,struct hdr_field *h);

str sip2ims_get_first_via(struct sip_msg *msg, struct hdr_field **h);

str sip2ims_get_realm(struct sip_msg *msg);
str sip2ims_get_opaque(struct sip_msg *msg);

int sip2ims_get_p_associated_uri(struct sip_msg *msg,str **public_id,int *public_id_cnt);

str sip2ims_get_preferred_identity(struct sip_msg *msg,struct hdr_field **h);

str sip2ims_get_first_route(struct sip_msg *msg,struct hdr_field **hr);

int sip2ims_remove_first_route(struct sip_msg *msg,str value);

int sip2ims_is_myself(str uri);
int sip2ims_remove_own_route(struct sip_msg *msg,struct hdr_field **h);

str sip2ims_get_content_type(struct sip_msg *msg);

int sip2ims_get_content_len(struct sip_msg *msg);

int sip2ims_get_subscription_state(struct sip_msg *msg);

str* sip2ims_get_service_route(struct sip_msg *msg,int *size);

str sip2ims_get_digest_uri(struct sip_msg *msg, str realm);
str sip2ims_get_algorithm(struct sip_msg *msg, str realm);

#endif /* SIP2IMS_SIP_H */
