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
 * Proxy-CSCF - Registrar Refreshment Through SUBSCRIBE to reg event at the S-CSCF
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "registrar_subscribe.h"

#include <libxml/xmlschemas.h>
#include <libxml/parser.h>
 

#include "registrar_storage.h"
#include "mod.h" 
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../parser/parse_uri.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "sip.h"

extern struct tm_binds tmb;   		/**< Structure with pointers to tm funcs 		*/

extern str pcscf_name_str;			/**< fixed SIP URI of this P-CSCF 				*/
extern str pcscf_path_str;			/**< fixed Path URI  							*/

extern time_t time_now;				/**< current time 								*/

extern r_hash_slot *registrar;		/**< the actual registrar						*/
extern int r_hash_size;				/**< number of hash slots in the registrar		*/

extern int pcscf_subscribe_retries;	/**< times to retry subscribe to reg on failure */

r_subscription_list *subscription_list=0;/**< list of subscriptions					*/

/**
 * Initialize the subscription list.
 * @returns 1 if ok, 0 on error
 */
int r_subscription_init()
{
	subscription_list = shm_malloc(sizeof(r_subscription_list));
	if (!subscription_list) return 0;
	memset(subscription_list,0,sizeof(r_subscription_list));
	subscription_list->lock = lock_alloc();
	if (!subscription_list->lock) return 0;
	subscription_list->lock = lock_init(subscription_list->lock);
	return 1;
}

/**
 * Destroys the subscription list
 */
void r_subscription_destroy()
{
	r_subscription *s,*ns;
	lock_get(subscription_list->lock);
	s = subscription_list->head;
	while(s){
		ns = s->next;
		//TODO send out unSUBSCRIBE
		free_r_subscription(s);
		s = ns;
	}
	lock_destroy(subscription_list->lock);
	lock_dealloc(subscription_list->lock);	
	shm_free(subscription_list);
}


/**
 * Subscribe to the reg event to the S-CSCF.
 * @param rpl - 200 OK response to REGISTER containing contacts and Service-Route header
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if subscribed, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_ERROR on error
 */
int P_subscribe(struct sip_msg *rpl, char* str1, char* str2)
{
	int expires_hdr=0,r,max_expires;
	str public_id={0,0};
	contact_t* c=0;
	contact_body_t* b=0;	
	
	cscf_get_first_p_associated_uri(rpl,&public_id);
	
	expires_hdr = cscf_get_expires_hdr(rpl);
	
	if (parse_headers(rpl, HDR_EOH_F, 0) <0) {
		LOG(L_ERR,"ERR:"M_NAME":P_subscribe: error parsing headers\n");
		return CSCF_RETURN_ERROR;
	}	
	
	b = cscf_parse_contacts(rpl);
	
	if (!b||!b->contacts) {
		LOG(L_DBG,"DBG:"M_NAME":P_subscribe: No contacts found\n");
		return CSCF_RETURN_FALSE;
	}
	
	if (b) c = b->contacts;
	
	max_expires = expires_hdr;
	while(c){
		r = expires_hdr;
		if (!c || str2int(&(c->expires->body), (unsigned int*)&r) < 0) {
			r = 0;
		}
		if (r>max_expires) max_expires = r;
		c = c->next;
	}
	if (max_expires<=0){
		LOG(L_INFO,"DBG:"M_NAME":P_subscribe: skipped because de-register\n");		
		return CSCF_RETURN_FALSE;		
	}
	
	if (public_id.s){
//		if (max_expires==0)
//			r_subscribe(public_id,0);
//		else
		r_subscribe(public_id,max_expires+30);
		return CSCF_RETURN_TRUE;
	}else{
		return CSCF_RETURN_FALSE;
	}
}

/**
 * Creates a subcription and starts the timer resubscription for the given contact.
 * @param uri - the contact to subscribe to (actually to its default public id)
 * @param duration - SUBCRIBE expires
 * @returns 1 on success, 0 on failure
 */
int r_subscribe(str uri,int duration)
{
	r_subscription *s;
	str asserted_id={0,0};
	asserted_id.s = pcscf_path_str.s;
	asserted_id.len = pcscf_path_str.len;
	/* first we try to update. if not found, add it */
	if (!is_r_subscription(uri)){			
		s = new_r_subscription(uri,pcscf_name_str,duration,asserted_id);
		if (!s){
			LOG(L_ERR,"ERR:"M_NAME":r_subscribe: Error creating new subscription\n");
			return 0;
		}
		add_r_subscription(s);
	}
	return 1;
}


static str method={"SUBSCRIBE",9};
static str event_hdr={"Event: reg\r\n",12};
static str accept_hdr={"Accept: application/reginfo+xml\r\n",33};
static str content_len_hdr={"Content-Length: 0\r\n",19};
static str max_fwds_hdr={"Max-Forwards: 10\r\n",18};
static str expires_s={"Expires: ",9};
static str expires_e={"\r\n",2};
static str contact_s={"Contact: <",10};
static str contact_e={">\r\n",3};
static str p_asserted_identity_s={"P-Asserted-Identity: <",22};
static str p_asserted_identity_e={">\r\n",3};
/**
 * Send a subscription
 * @param s - the subsription to send for
 * @param duration - expires time
 * @returns true if OK, false if not, error on failure
 * \todo store the dialog and reSubscribe on the same dialog
 */
int r_send_subscribe(r_subscription *s,int duration)
{
	str h={0,0};

	LOG(L_DBG,"DBG:"M_NAME":r_send_subscribe: SUBSCRIBE to <%.*s>\n",
		s->req_uri.len,s->req_uri.s);
	
	h.len = event_hdr.len+accept_hdr.len+content_len_hdr.len+max_fwds_hdr.len;
	h.len += expires_s.len + 12 + expires_e.len;

	h.len += contact_s.len + pcscf_name_str.len + contact_e.len;
	if (s->asserted_identity.len) h.len += p_asserted_identity_s.len + 
		p_asserted_identity_e.len + s->asserted_identity.len;

	h.s = pkg_malloc(h.len);
	if (!h.s){
		LOG(L_ERR,"ERR:"M_NAME":r_send_subscribe: Error allocating %d bytes\n",h.len);
		h.len = 0;
		return 0;
	}

	h.len = 0;
	STR_APPEND(h,event_hdr);
	STR_APPEND(h,accept_hdr);
	STR_APPEND(h,content_len_hdr);
	STR_APPEND(h,max_fwds_hdr);

	STR_APPEND(h,expires_s);
	sprintf(h.s+h.len,"%d",duration);
	h.len += strlen(h.s+h.len);
	STR_APPEND(h,expires_e);

	STR_APPEND(h,contact_s);
	STR_APPEND(h,pcscf_name_str);
	STR_APPEND(h,contact_e);
	
	if (s->asserted_identity.len) {
		STR_APPEND(h,p_asserted_identity_s);
		STR_APPEND(h,s->asserted_identity);
		STR_APPEND(h,p_asserted_identity_e);
	}
	
	if (tmb.t_request(&method, &(s->req_uri), &(s->req_uri), &(s->from), &h, 0, 0,
		 r_subscribe_response, &(s->req_uri))<0)
	{
		LOG(L_ERR,"ERR:"M_NAME":r_send_subscribe: Error sending in transaction\n");
		goto error;
	}


	if (h.s) pkg_free(h.s);
	return 1;

error:
	if (h.s) pkg_free(h.s);
	return 0;
}

/**
 * Response callback for subscribe
 */
void r_subscribe_response(struct cell *t,int type,struct tmcb_params *ps)
{
	str req_uri;
	int expires;
	LOG(L_DBG,"DBG:"M_NAME":r_subscribe_response: code %d\n",ps->code);
	if (!ps->rpl) {
		LOG(L_ERR,"INF:"M_NAME":r_subscribe_response: No reply\n");
		return;	
	}
	if (ps->code>=200 && ps->code<300){
		if (ps->rpl)
			expires = cscf_get_expires_hdr(ps->rpl);
		else 
			return;	
		req_uri = *((str*) *(ps->param));
		update_r_subscription(req_uri,expires);
	}else
	if (ps->code==404){
		update_r_subscription(req_uri,-1);			
	}else{
		LOG(L_INFO,"INF:"M_NAME":r_subscribe_response: code %d\n",ps->code);				
	}	
}

/**
 * The Subscription timer looks for almost expired subscriptions and subscribes again.
 * @param ticks - the current time
 * @param param - the generic parameter
 */
void subscription_timer(unsigned int ticks, void* param)
{
	r_subscription *s,*ns;
	lock_get(subscription_list->lock);
	s = subscription_list->head;
	r_act_time();
	while(s){
		ns = s->next;
		/* send initial subscribe */
		if (s->expires == 0){
			if (s->attempts>=pcscf_subscribe_retries){
				LOG(L_ERR,"ERR:"M_NAME":subscription_timer: Error on SUBSCRIBE for %d times... aborting\n",pcscf_subscribe_retries);
				del_r_subscription_nolock(s);			
			}else
			if (!r_send_subscribe(s,s->duration)){
				LOG(L_ERR,"ERR:"M_NAME":subscription_timer: Error on initial SUBSCRIBE... droping\n");
				del_r_subscription_nolock(s);
			}else{
				s->attempts ++;
			}
		}else
		if ((s->duration<1200 && s->expires-time_now<s->duration/2)||
			(s->duration>=1200 && s->expires-time_now<600))
		{
			if (s->attempts>=3){
				LOG(L_ERR,"ERR:"M_NAME":subscription_timer: Error on SUBSCRIBE for 3 times... aborting\n");
				del_r_subscription_nolock(s);			
			}else
			if (!r_send_subscribe(s,s->duration)){
				LOG(L_ERR,"ERR:"M_NAME":subscription_timer: Error on initial SUBSCRIBE... droping\n");
				del_r_subscription_nolock(s);
			}else{
				del_r_subscription_nolock(s);			
				s->attempts++;
			}
		}

		s = ns;
	}	
	lock_release(subscription_list->lock);
}



/**
 * Creates a subscription based on the given parameters.
 * @param req_uri - the AOR of the user to subcribe to
 * @param from - the From header
 * @param duration - expires time in seconds
 * @param asserted_identity - P-Asserted-Identity-Header to use
 * @returns the r_notification or NULL on error
 */
r_subscription* new_r_subscription(str req_uri,str from,int duration,str asserted_identity)
{
	r_subscription *s=0;
	
	s = shm_malloc(sizeof(r_subscription));
	if (!s){
		LOG(L_ERR,"ERR:"M_NAME":new_r_subscription: Error allocating %d bytes\n",
			sizeof(r_subscription));
		goto error;
	}
	memset(s,0,sizeof(r_subscription));
	
	STR_SHM_DUP(s->req_uri,req_uri,"new_r_subscription");
	if (!s->req_uri.s) goto error;
	
	STR_SHM_DUP(s->from,from,"new_r_subscription");
	if (!s->from.s) goto error;
	
	s->duration = duration;
	s->expires = 0;
	
	STR_SHM_DUP(s->asserted_identity,asserted_identity,"new_r_subscription");
	if (!s->asserted_identity.s) goto error;
	
	
	return s;
error:
	if (s->req_uri.s) shm_free(s->req_uri.s);
	if (s->from.s) shm_free(s->from.s);
	if (s->asserted_identity.s) shm_free(s->asserted_identity.s);
	if (s) shm_free(s);	
	return 0;
}

/**
 * Adds a subscription to the list of subscriptions at the end (FIFO).
 * @param s - the subscription to be added
 */
void add_r_subscription(r_subscription *s)
{
	if (!s) return;
	lock_get(subscription_list->lock);
	s->next = 0;
	s->prev = subscription_list->tail;
	if (subscription_list->tail) {
		subscription_list->tail->next = s;
		subscription_list->tail = s;
	}
	else subscription_list->tail = s;
	if (!subscription_list->head) subscription_list->head = s;		
	lock_release(subscription_list->lock);
}

/**
 * Updates the expiration time of a subscription.
 * \todo Maybe we should use a hash here to index it as this is called for every notification
 * @param aor - aor to look for
 * @param expires - new expiration time
 * @returns 1 if found, 0 if not
 */
int update_r_subscription(str aor,int expires)
{
	r_subscription *s;
	lock_get(subscription_list->lock);
	s = subscription_list->head;
	r_act_time();
	while(s){
		if (s->req_uri.len == aor.len &&
			strncasecmp(s->req_uri.s,aor.s,aor.len)==0)
		{
			LOG(L_DBG,"DBG:"M_NAME":update_r_subscription: refreshing subscription for <%.*s> [%d]\n",
				aor.len,aor.s,expires);
			s->attempts = 0;
			if (expires == 0){
				del_r_subscription_nolock(s);
			}else{
				s->expires = expires+time_now;;
			}
			lock_release(subscription_list->lock);	
			return 1;
		}
		s = s->next;
	}
	lock_release(subscription_list->lock);	
	return 0;
}

/**
 * Finds out if a subscription exists
 * \todo Maybe we should use a hash here to index it as this is called for every notification
 * @param aor - AOR to look for
 * @returns 1 if found, 0 if not
 */
int is_r_subscription(str aor)
{
	r_subscription *s;
	lock_get(subscription_list->lock);
	s = subscription_list->head;
	while(s){
		if (s->req_uri.len == aor.len &&
			strncasecmp(s->req_uri.s,aor.s,aor.len)==0)
		{
			lock_release(subscription_list->lock);	
			return 1;
		}
		s = s->next;
	}
	lock_release(subscription_list->lock);	
	return 0;
}

/**
 * Deletes a subscription from the list of subscriptions 
 * @param s - the subscription to be deleted
 */
void del_r_subscription(r_subscription *s)
{
	if (!s) return;
	lock_get(subscription_list->lock);
	if (subscription_list->head == s) subscription_list->head = s->next;
	else s->prev->next = s->next;
	if (subscription_list->tail == s) subscription_list->tail = s->prev;
	else s->next->prev = s->prev;
	lock_release(subscription_list->lock);
	free_r_subscription(s);
}

/**
 * Deletes a subscription from the list of subscriptions.
 * \note Must have the lock to do this
 * @param s - the subscription to be deleted
 */
void del_r_subscription_nolock(r_subscription *s)
{
	if (!s) return;
	if (subscription_list->head == s) subscription_list->head = s->next;
	else s->prev->next = s->next;
	if (subscription_list->tail == s) subscription_list->tail = s->prev;
	else s->next->prev = s->prev;
	free_r_subscription(s);
}

/**
 * Frees up space taken by a subscription
 * @param s - the subscription to free
 */
void free_r_subscription(r_subscription *s)
{
	if (s){
		if (s->req_uri.s) shm_free(s->req_uri.s);
		if (s->from.s) shm_free(s->from.s);
		if (s->asserted_identity.s) shm_free(s->asserted_identity.s);
		shm_free(s);
	}
}










char *pcscf_reginfo_dtd;/**< DTD to check the reginfo/xml in the NOTIFY to reg */

static xmlDtdPtr	dtd=0;	/**< DTD file */
static xmlValidCtxt	cvp;	/**< XML Validating context */

/**
 * Initializes the libxml parser.
 * @param dtd_filename - path to the DTD file
 * @returns 1 if OK, 0 on error
 */
int parser_init(char *dtd_filename)
{
	dtd = xmlParseDTD(NULL,(unsigned char*)dtd_filename);
	if (!dtd){
		LOG(L_ERR,"ERR:"M_NAME":parser_init: unsuccesful DTD parsing from file <%s>\n",
			dtd_filename);
		return 0;
	}
	cvp.userData = (void*)stderr;
	cvp.error = (xmlValidityErrorFunc) fprintf;
	cvp.warning = (xmlValidityWarningFunc) fprintf;
	return 1;
}

/**
 * Destroys the parser. 
 */
void parser_destroy()
{
	xmlCleanupParser();
}



/**
 * Trims spaces and duplicate content into pkg.
 * @param dest - destination
 * @param src - source
 */
static inline void space_trim_dup(str *dest, char *src)
{
	int i;
	dest->s=0;
	dest->len=0;
	if (!src) return;
	dest->len = strlen(src);
	i = dest->len-1;
	while((src[i]==' '||src[i]=='\t') && i>0) 
		i--;
	i=0;
	while((src[i]==' '||src[i]=='\t') && i<dest->len)
		i++;
	dest->len -= i;
	dest->s = pkg_malloc(dest->len);
	if (!dest->s) {
		LOG(L_ERR,"ERR:"M_NAME":space_trim_dup: Out of memory allocating %d bytes\n",dest->len);
		dest->len=0;
		return;
	}
	memcpy(dest->s,src+i,dest->len);
}

/**
 * Parses a notification and creates the r_notification object
 * @param xml - the XML data
 * @returns the new r_notification* or NULL on error
 */
r_notification* r_notification_parse(str xml)
{
	r_notification *n;
	r_registration *r;
	r_regcontact *rc;
	xmlDocPtr doc=0;
	xmlNodePtr root=0,child=0,nephew=0,node=0;
	xmlChar *reginfo_state=0,*x;
	char c=0;
	
	n = pkg_malloc(sizeof(r_notification));
	if (!n) {
		LOG(L_ERR,"ERR:"M_NAME":r_notification_parse: Error allocating %d bytes\n",
			sizeof(r_notification));
		goto error;
	}
	memset(n,0,sizeof(r_notification));

	if (!dtd) parser_init(pcscf_reginfo_dtd);
	doc=0;
	c = xml.s[xml.len];
	xml.s[xml.len]=0;
	doc = xmlParseDoc((unsigned char *)xml.s);
	if (!doc){
		LOG(L_ERR,"ERR:"M_NAME":r_notification_parse:  This is not a valid XML <%.*s>\n",
			xml.len,xml.s);
		goto error;
	}
	if (xmlValidateDtd(&cvp,doc,dtd)!=1){
		LOG(L_ERR,"ERR:"M_NAME":r_notification_parse:  Verification of XML against DTD failed <%.*s>\n",
			xml.len,xml.s);
		goto error;
	}
	root = xmlDocGetRootElement(doc);
	if (!root){
		LOG(L_ERR,"ERR:"M_NAME":r_notification_parse:  Empty XML <%.*s>\n",
			xml.len,xml.s);
		goto error;
	}

	reginfo_state = xmlGetProp(root,"state");
	LOG(L_DBG,"DBG:"M_NAME":r_notification_parse: reginfo_state <%s>\n",
			reginfo_state);
	if (reginfo_state[0]=='f'||reginfo_state[0]=='F')
		n->state = IMS_REGINFO_FULL;
	else 
		n->state = IMS_REGINFO_PARTIAL;
	
	for(child = root->children; child; child = child->next)
		if (child->type == XML_ELEMENT_NODE)
	{
		r = pkg_malloc(sizeof(r_registration));
		if (!r){
			LOG(L_ERR,"ERR:"M_NAME":r_notification_parse: Error allocating %d bytes\n",
				sizeof(r_registration));
			goto error;
		}
		memset(r,0,sizeof(r_registration));
		
		x = xmlGetProp(child,"id");
		space_trim_dup(&(r->id),x);
		xmlFree(x);

		x = xmlGetProp(child,"aor");
		space_trim_dup(&(r->aor),x);
		xmlFree(x);
		
		x = xmlGetProp(child,"state");
		
		if (x[0]=='a'||x[0]=='A') 
			r->state = IMS_REGINFO_ACTIVE;
		else 
			r->state = IMS_REGINFO_TERMINATED;
		xmlFree(x);

		for(nephew = child->children; nephew; nephew = nephew->next)
				if (nephew->type == XML_ELEMENT_NODE)
		{
			rc = pkg_malloc(sizeof(r_regcontact));
			if (!rc){
				LOG(L_ERR,"ERR:"M_NAME":r_notification_parse: Error allocating %d bytes\n",
					sizeof(r_regcontact));
				goto error;
			}
			memset(rc,0,sizeof(r_regcontact));
			
			x = xmlGetProp(nephew,"id");
			space_trim_dup(&(rc->id),x);
			xmlFree(x);
				
			x = xmlGetProp(nephew,"state");
			if (x[0]=='a'||x[0]=='A') 
				rc->state = IMS_REGINFO_ACTIVE;
			else 
				rc->state = IMS_REGINFO_TERMINATED;
			xmlFree(x);
			
			x = xmlGetProp(nephew,"event");
			switch(x[0]){
				case 'r':case 'R':
					switch (x[2]){
						case 'g': case 'G':
							rc->event = IMS_REGINFO_CONTACT_REGISTERED;
							break;
						case 'f': case 'F':
							rc->event = IMS_REGINFO_CONTACT_REFRESHED;
							break;						
						case 'j': case 'J':
							rc->event = IMS_REGINFO_CONTACT_REJECTED;
							break;						
						default:
							rc->event = IMS_REGINFO_NONE;
					}
					break;
				case 'c':case 'C':
					rc->event = IMS_REGINFO_CONTACT_CREATED;	
					break;
				case 's':case 'S':
					rc->event = IMS_REGINFO_CONTACT_SHORTENED;	
					break;
				case 'e':case 'E':
					rc->event = IMS_REGINFO_CONTACT_EXPIRED;	
					break;
				case 'd':case 'D':
					rc->event = IMS_REGINFO_CONTACT_DEACTIVATED;	
					break;
				case 'p':case 'P':
					rc->event = IMS_REGINFO_CONTACT_PROBATION;	
					break;
				case 'u':case 'U':
					rc->event = IMS_REGINFO_CONTACT_UNREGISTERED;	
					break;
				default:
					rc->event = IMS_REGINFO_NONE;	
			}
			xmlFree(x);

			x = xmlGetProp(nephew,"expires");			
			if (x) {
				rc->expires = atoi(x);
				xmlFree(x);
			}
			
			node = nephew->children;
			while(node && node->type!=XML_ELEMENT_NODE)
				node =node->next;
			if (node) {
				x = xmlNodeGetContent(node);
				space_trim_dup(&(rc->uri),x);
				xmlFree(x);
			}
			
			rc->next = r->contact;
			r->contact = rc;
		}
		
		r->next = n->registration;
		n->registration = r;
					
	}
			
	if (reginfo_state) xmlFree(reginfo_state);		
	
	if (doc) xmlFreeDoc(doc);
	xml.s[xml.len]=c;
	return n;
error:	
	if (reginfo_state) xmlFree(reginfo_state);		

	if (doc) xmlFreeDoc(doc);
	xml.s[xml.len]=c;
	if (n) r_notification_free(n);
	return 0;
}



/**
 * Processes a notification and updates the registrar info.
 * @param n - the notification
 * @param expires - the Subscription-Status expires parameter
 * @returns 1 on success, 0 on error
 */
int r_notification_process(r_notification *n,int expires)
{
	r_registration *r;
	r_regcontact *rc;	
	r_contact *c;
	struct sip_uri puri;
	enum Reg_States reg_state;
	int expires2;
	
	r_notification_print(n);	
	if (!n) return 0;
	
	r_act_time();
	r = n->registration;
	while(r){
		
		rc = r->contact;
		while(rc){
			if (parse_uri(rc->uri.s,rc->uri.len,&puri)<0){
				LOG(L_ERR,"ERR:"M_NAME":r_notification_process: Error parsing Contact URI <%.*s>\n",
					rc->uri.len,rc->uri.s);
				goto next;
			}
//			LOG(L_CRIT,"DBG:"M_NAME":r_notification_process: refreshing contacts <%.*s> [%d]\n",rc->uri.len,rc->uri.s,rc->expires);
			if (rc->state==IMS_REGINFO_TERMINATED){
				reg_state = DEREGISTERED;
				expires2 = time_now+30;
				c = update_r_contact(puri.host,puri.port_no,puri.proto,
					0,&reg_state,&expires2,0,0,0);
				if (c) {
					LOG(L_DBG,"DBG:"M_NAME":r_notification_process: expired contact <%.*s>\n",
						c->uri.len,c->uri.s);
					r_unlock(c->hash);					
				}
			}else{
				reg_state = REGISTERED;
				expires2 = rc->expires+time_now;
				c = update_r_contact(puri.host,puri.port_no,puri.proto,
					0,&reg_state,&expires2,0,0,0);
				if (c) {
					LOG(L_DBG,"DBG:"M_NAME":r_notification_process: refreshing contact <%.*s> [%d]\n",
						c->uri.len,c->uri.s,rc->expires);
					r_unlock(c->hash);					
				}

			}
next:				
			rc = rc->next;	
		}
		
		update_r_subscription(r->aor,expires);
		r = r->next;
	}

	return 1;
}

/** 
 * Prints the content of a notification
 * @param n - the notification to print
 */
void r_notification_print(r_notification *n)
{
	r_registration *r;
	r_regcontact *c;
	
	if (!n) return;
	LOG(L_DBG,"DBG:"M_NAME":r_notification_process: State %d\n",n->state);
	r = n->registration;
	while(r){
		LOG(L_DBG,"DBG:"M_NAME":r_notification_process: \tR [%d] ID<%.*s> AOR<%.*s>\n",r->state,
			r->id.len,r->id.s,r->aor.len,r->aor.s);
		c = r->contact;
		while(c){
			LOG(L_DBG,"DBG:"M_NAME":r_notification_process: \t\tC [%d]>[%d] ID<%.*s> URI<%.*s>\n",c->state,
				c->event,c->id.len,c->id.s,c->uri.len,c->uri.s);
			c = c->next;
		}
		r = r->next;
	}	
}

/**
 * Frees up the space taken by a notification
 * @param n - the notification to free
 */
void r_notification_free(r_notification *n)
{
	r_registration *r,*nr;
	r_regcontact *c,*nc;
	r = n->registration;
	while(r){
		nr = r->next;
		if (r->id.s) pkg_free(r->id.s);
		if (r->aor.s) pkg_free(r->aor.s);
		c = r->contact;
		while(c){
			nc = c->next;
			if (c->id.s) pkg_free(c->id.s);			
			if (c->uri.s) pkg_free(c->uri.s);
			pkg_free(c);
			c = c->next;
		}
		pkg_free(r);
		r = nr;
	}
	if (n) pkg_free(n);
}






