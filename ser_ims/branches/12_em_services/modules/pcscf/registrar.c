/*
 * $Id$
 *  
 * Copyright (C) 2004-2009 FhG Fokus
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
 * Proxy-CSCF -Registrar Related Operations
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *  \author Ancuta Onofrei	andreea dot ancuta dot onofrei -at- fokus dot fraunhofer dot de : changes for E-CSCF support
 * 
 */

#include <time.h>
#include <string.h>

#include "../../parser/contact/contact.h"
#include "../../parser/contact/parse_contact.h"
#include "../../ut.h"
#include "../tm/tm_load.h"
#include "../../dset.h"

#include "registrar.h"
#include "registrar_subscribe.h"
#include "mod.h"
#include "sip.h"
#include "nat_helper.h"
#include "security.h"
#include "ims_pm.h"


extern struct tm_binds tmb;				/**< Structure with pointers to tm funcs 	*/
extern time_t time_now;					/**< current time 							*/
extern r_hash_slot *registrar;			/**< the actual registrar					*/
extern int 	   r_hash_size;				/**< number of hash slots in the registrar	*/
extern int pcscf_nat_enable; 			/**< whether to enable NAT					*/
extern int pcscf_nat_ping; 				/**< whether to ping anything 				*/
extern int pcscf_nat_pingall; 			/**< whether to ping also the UA that don't look like being behind a NAT */
extern int pcscf_nat_detection_type; 	/**< the NAT detection tests 				*/

#ifdef WITH_IMS_PM
	static str zero={0,0};
#endif
/**
 * The Registrar timer looks for expires contacts and removes them.
 * For the non-deleted contacts a ping is sent if the UA is behind a NAT.
 * @param ticks - the current time
 * @param param - pointer to the domain_list
 */
void registrar_timer(unsigned int ticks, void* param)
{
	r_contact *c,*cn;
	int i;
	#ifdef WITH_IMS_PM
		int impu_cnt=0,contact_cnt=0,ipsec_cnt=0,tls_cnt=0,nat_cnt=0;
		r_public *rp;
	#endif
	
	LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Called at %d\n",ticks);
	if (!registrar) registrar = (r_hash_slot*)param;

	r_act_time();
	
	for(i=0;i<r_hash_size;i++){
		r_lock(i);
			c = registrar[i].head;
			while(c){
				cn = c->next;
				switch (c->reg_state){
					case NOT_REGISTERED:
						LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> Not Registered and removed.\n",
								c->uri.len,c->uri.s);
						del_r_contact(c);
						break;
					case REGISTERED:
						if (c->expires<=time_now) {
							LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> expired and Deregistered.\n",
								c->uri.len,c->uri.s);		
							if (c->security){
								/* If we have IPSec SAs, we keep them 60 seconds more to relay further messages */
								c->reg_state = DEREGISTERED;
								c->expires = time_now + 60;
							}else{
								LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> expired and removed.\n",
									c->uri.len,c->uri.s);						
								del_r_contact(c);
							}
						}
						#ifdef WITH_IMS_PM
							else {
									contact_cnt++;
									for(rp=c->head;rp;rp=rp->next)
										impu_cnt++;
									if (c->security && c->security->type==SEC_IPSEC) ipsec_cnt++;
									if (c->security && c->security->type==SEC_TLS) tls_cnt++;				
									if (c->pinhole) nat_cnt++;
							}			
						#endif
						break;
					case DEREGISTERED:
						if (c->expires<=time_now) {
							LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> expired and removed.\n",
								c->uri.len,c->uri.s);		
							P_security_drop(c,c->security);
							P_security_drop(c,c->security_temp);
							del_r_contact(c);
						}
						break;
					case REG_PENDING:
						if (c->expires<=time_now) {
							LOG(L_DBG,"DBG:"M_NAME":registrar_timer: Contact <%.*s> Registration pending expired and removed.\n",
								c->uri.len,c->uri.s);		
							P_security_drop(c,c->security);
							P_security_drop(c,c->security_temp);
							del_r_contact(c);
						}
						break;
				}				
				if (pcscf_nat_enable && pcscf_nat_ping) nat_send_ping(c);
				c = cn;
			}
		r_unlock(i);
	}
	print_r(L_INFO);
	#ifdef WITH_IMS_PM
		IMS_PM_LOG01(RD_NbrContact,contact_cnt);
		IMS_PM_LOG01(RD_NbrIMPU,impu_cnt);
		IMS_PM_LOG01(RD_NbrIPSecSA,ipsec_cnt);
		IMS_PM_LOG01(RD_NbrTLSSA,tls_cnt);
		IMS_PM_LOG01(RD_NbrNATPinHoles,nat_cnt);
	#endif
}


/**
 * Calculates the expiration time for one contact.
 * Tries to use the Expiration header, if not present then use the 
 * expires parameter of the contact, if param not present it defaults
 * to the default value.
 * Also checks 
 * @param c - the contact to calculate for
 * @param expires_hdr - value of expires hdr if present, if not -1
 * @param local_time_now - the local time
 * @returns the time of expiration
 */
static inline int r_calc_expires(contact_t *c,int expires_hdr, int local_time_now)
{
	unsigned int r;
	if (expires_hdr>=0) r = expires_hdr;
	if (c) 
		str2int(&(c->expires->body), (unsigned int*)&r);
		
	return local_time_now+r;
}

static inline int contact_was_in_req(contact_t *c,struct sip_msg *req)
{
	struct hdr_field* h;
	contact_t *x;
	for(h=cscf_get_next_header_type(req,HDR_CONTACT_T,0);h;h=cscf_get_next_header_type(req,HDR_CONTACT_T,h)){
		if (parse_contact(h)<0||h->parsed==0) continue;
		for(x=((contact_body_t*)h->parsed)->contacts;x;x=x->next){
			if (c->uri.len == x->uri.len &&
				strncasecmp(c->uri.s,x->uri.s,x->uri.len)==0){
				LOG(L_DBG,"DBG:contact_was_in_req(): Found the contact in the request: %.*s\n",x->uri.len,x->uri.s);
				free_contact((contact_body_t**)&(h->parsed));
				return 1;
			}
		}
		free_contact((contact_body_t**)&(h->parsed));
	}
	return 0;
}

/**
 * Updates the registrar with the new values
 * @param req - the REGISTER request - to extract NAT info 
 * @param rpl - the REGISTER reply - to extract contact info 
 * @param is_star - whether this was a STAR contact header
 * @param expires_hdr - value of the Expires header
 * @param public_id - array of public identities attached to this contact
 * @param public_id_cnt - size of the public_id array
 * @param service_route - array of Service-Routes
 * @param service_route_cnt - size of the service_route array
 * @param requires_nat - if to create pinholes 
 * @returns the maximum expiration time, -1 on error
 */
static inline int update_contacts(struct sip_msg *req,struct sip_msg *rpl,unsigned char is_star,int expires_hdr,
	str *public_id,int public_id_cnt,str *service_route,int service_route_cnt, int requires_nat)
{
	r_contact *rc;
	enum Reg_States reg_state=REGISTERED;
	int expires=0;
	int is_default=0,i;
	struct sip_uri puri;
	int max_expires=-1;
	int local_time_now;
	struct hdr_field *h;
	contact_t *c;
	r_nat_dest *pinhole;
	int sos_reg;
	
	
	r_act_time();
	local_time_now = time_now;
	if (is_star){
		/* first of all, we shouldn't get here...
		 * then, we will update on NOTIFY */
		return 0;
	}	
	for(h=rpl->contact;h;h=h->next)
		if (h->type==HDR_CONTACT_T && h->parsed)
		 for(c=((contact_body_t*)h->parsed)->contacts;c;c=c->next){
			LOG(L_DBG,"DBG:"M_NAME":update_contact: <%.*s>\n",c->uri.len,c->uri.s);
			
			expires = r_calc_expires(c,expires_hdr,local_time_now);
			
			if (parse_uri(c->uri.s,c->uri.len,&puri)<0){
				LOG(L_DBG,"DBG:"M_NAME":update_contact: Error parsing Contact URI <%.*s>\n",c->uri.len,c->uri.s);
				continue;			
			}
			if (puri.port_no==0) puri.port_no=5060;
			LOG(L_DBG,"DBG:"M_NAME":update_contact: %d %.*s : %d\n",
				puri.proto, puri.host.len,puri.host.s,puri.port_no);
			
			sos_reg = cscf_get_sos_uri_param(c->uri);
			if(sos_reg < 0)
				return 0;

			if(sos_reg>0)
				LOG(L_DBG,"DBG:"M_NAME":update_contact: with sos uri param\n");
			
			if (expires>local_time_now) {
				if (requires_nat &&				/* only if NAT was enabled */ 
					contact_was_in_req(c,req)	/* and the contact was refreshed, not just sent from the S-CSCF */
					) {							
					pinhole = nat_msg_origin(req);
					rc = update_r_contact(puri.host,puri.port_no,puri.proto,
								&(c->uri),&reg_state,&expires,&service_route,
								&service_route_cnt, &pinhole, &sos_reg);
				}else{
					rc = update_r_contact(puri.host,puri.port_no,puri.proto,
								&(c->uri),&reg_state,&expires,&service_route,
								&service_route_cnt, 0, &sos_reg);
				}
				if (expires-time_now>max_expires) max_expires=expires-time_now;
			}
			else {
				reg_state = DEREGISTERED;
				expires = local_time_now+30;
				rc = update_r_contact(puri.host,puri.port_no,puri.proto,
						0,&reg_state,&expires,0,0,0, &sos_reg);
				if (rc) r_unlock(rc->hash);
				if (0>max_expires) max_expires = 0;
				rc = 0;
			}
			
			/** Add the public identities */
			if (rc){
				is_default=1;
				if (public_id_cnt){
					update_r_public(rc,public_id[0],&is_default);
					is_default=0;
					for(i=1;i<public_id_cnt;i++)
						update_r_public(rc,public_id[i],&is_default);
				}
				r_unlock(rc->hash);
			}
	}
	return max_expires;
}




/**
 * Save the contacts and their associated public ids.
 * @param rpl - the SIP Register 200 OK response that contains the Expire and Contact headers
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if OK, #CSCF_RETURN_ERROR on error
 */
int P_save_location(struct sip_msg *rpl,char *str1, char *str2)
{
	struct sip_msg *req;
	contact_body_t* b=0;	
	str realm;
	int expires_hdr=0;
	str *public_id=0;
	int public_id_cnt=0;
	int expires;
	str *service_route=0;
	int service_route_cnt;
		
	req = cscf_get_request_from_reply(rpl);
	if (!req){
		LOG(L_ERR,"ERR:"M_NAME":P_save_location: No transactional request found.\n");
		goto error;
	}
	
	expires_hdr = cscf_get_expires_hdr(rpl);
	/** Removed because this would parse the hdr, but then it will fail to free the hdr->parsed */
//	if (expires_hdr<0) 
//		expires_hdr = cscf_get_expires_hdr(req);
	
	if (parse_headers(rpl, HDR_EOH_F, 0) <0) {
		LOG(L_ERR,"ERR:"M_NAME":P_save_location: error parsing headers\n");
		return CSCF_RETURN_ERROR;
	}	
	
	b = cscf_parse_contacts(rpl);
	
	if (!b||(!b->contacts && !b->star)) {
		LOG(L_DBG,"DBG:"M_NAME":P_save_location: No contacts found\n");
		return 0;
	}
	
	realm = cscf_get_realm(req);
	
	cscf_get_p_associated_uri(rpl,&public_id,&public_id_cnt);
	
	service_route = cscf_get_service_route(rpl,&service_route_cnt);
			
	if ((expires=update_contacts(req,rpl,b->star,expires_hdr,
			public_id,public_id_cnt,service_route,service_route_cnt,
			requires_nat(req)))<0) 
		goto error;

	//print_r(L_ERR);
	
	
	if (service_route)	pkg_free(service_route);
	if (public_id) pkg_free(public_id);
	return CSCF_RETURN_TRUE;
error:
	if (service_route)	pkg_free(service_route);
	if (public_id) pkg_free(public_id);
	return CSCF_RETURN_ERROR;
}


/**
 * Finds if the message is integrity protected
 * @param host - host of the UE
 * @param port - port of the UE
 * @param r_port - received port (added by PCSCF) 
 * @param transport - transport of the UE
 * @param session_hash - TLS session hash
 * @returns 1 if registered, 0 if not or error
 */
int r_is_integrity_protected(str host,int port,int r_port,int transport, unsigned long session_hash)
{
	int ret=0;
	r_contact *c;

	if (port==0) port=5060;
//	LOG(L_ERR,"DBG:"M_NAME":r_is_registered: Looking if registered <%d://%.*s:%d>\n",
//		transport,host.len,host.s,port);

//	print_r(L_INFO);
	c = get_r_contact(host,port,transport, ANY_REG);

	if (!c) return 0;
	
	if (c->security){
		switch (c->security->type){
			case SEC_NONE:
				break;
			case SEC_TLS:
				if (c->security->data.tls&& 
					(c->security->data.tls->port_tls==r_port) &&
					(session_hash!= 0 && c->security->data.tls->session_hash == session_hash) // test same session
					){
					ret = 1;
				}
				break;
			case SEC_IPSEC:
				if (c->security->data.ipsec&&
					(c->security->data.ipsec->port_uc==port || c->security->data.ipsec->port_us==port)){
					ret = 1;
				}
				break;
		}			
	}
	if (!ret && c->security_temp){
		switch (c->security_temp->type){
			case SEC_NONE:
				break;
			case SEC_TLS:
				//nothing to do here becaues TLS security in temp does not mean that the user knows the password!
				break;
			case SEC_IPSEC:
				if (c->security_temp->data.ipsec&&
					(c->security_temp->data.ipsec->port_uc==port || c->security_temp->data.ipsec->port_us==port)){
					ret = 1;
				}
				break;
		}			
	}
	r_unlock(c->hash);
	return ret;
}

/**
 * Finds if the user is registered.
 * @param host - host of the UE
 * @param port - port of the UE
 * @param transport - transport of the UE
 * @param sos_mask - type of registration
 * @returns 1 if registered, 0 if not or error
 */
int r_is_registered(str host,int port,int transport, r_reg_type sos_mask)
{
	int ret=0;
	r_contact *c;

	if (port==0) port=5060;
//	LOG(L_ERR,"DBG:"M_NAME":r_is_registered: Looking if registered <%d://%.*s:%d>\n",
//		transport,host.len,host.s,port);

//	print_r(L_INFO);
	c = get_r_contact(host,port,transport, sos_mask);

	if (!c){		
		return 0;
	}	
	r_act_time();
	if (r_reg_contact(c)){
		ret = 1;
	}
	r_unlock(c->hash);
	
	return ret;
}


/**
 * Asserts the identity of the user and returns the value
 * @param host - host of the UE
 * @param port - port of the UE
 * @param transport - transport of the UE
 * @param preferred - the P-Preferred-Identity header value
 * @returns 1 if registered, {0,0} if not or error
 */
name_addr_t r_assert_identity(str host,int port,int transport,name_addr_t preferred, r_reg_type reg_type)
{
	r_contact *c;
	r_public *p;
	name_addr_t id;
	if (port==0) port=5060;

	memset(&id,0,sizeof(name_addr_t));
	
	LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: Asserting preferred id <%.*s>\n",
		preferred.uri.len,preferred.uri.s);
//	print_r(L_INFO);
	c = get_r_contact(host,port,transport, reg_type);

	if (!c){
		LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: Contact not found\n");		
		return id;
	}
	r_act_time();
	if (!r_reg_contact(c)){
		LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: Contact expired\n");
		r_unlock(c->hash);	
		return id;
	}
	
	if (!c->head){
		LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: No public ids for this user\n");
		r_unlock(c->hash);
		return id;	
	}
	id.name = preferred.name;	
	if (!preferred.uri.len){
		p = c->head;
		while(p&&!p->is_default)
			p = p->next;
		if (p) {
			LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: to <%.*s>\n",p->aor.len,p->aor.s);
			id.uri=p->aor;
			r_unlock(c->hash);
			return id;
		} else {
			LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: to <%.*s>\n",c->head->aor.len,c->head->aor.s);
			id.uri=c->head->aor;
			r_unlock(c->hash);
			return id;	
		}
	}else{
		p = c->head;
		while(p){
			if (p->aor.len==preferred.uri.len &&
				strncasecmp(p->aor.s,preferred.uri.s,preferred.uri.len)==0)
			{
				LOG(L_DBG,"DBG:"M_NAME":r_assert_identity: to <%.*s>\n",p->aor.len,p->aor.s);
				id.uri = preferred.uri;
				r_unlock(c->hash);
				return id;					
			}
			p = p->next;
		}
	}
	r_unlock(c->hash);
	return id;
}





