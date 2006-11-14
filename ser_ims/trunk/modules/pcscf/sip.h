  
#ifndef PIS_CSCF_SIP_H
#define PIS_CSCF_SIP_H

#include "../../sr_module.h"
#include "../../parser/contact/parse_contact.h"
#include "../../parser/parse_rr.h"

int cscf_add_header_first(struct sip_msg *msg, str *hdr);
int cscf_add_header(struct sip_msg *msg, str *hdr,int type);
int cscf_add_header_rpl(struct sip_msg *msg, str *hdr);
int cscf_add_contact(struct sip_msg *msg,str uri,int expires);
int cscf_delete_header(struct sip_msg *msg, struct hdr_field *hdr);

str cscf_get_private_identity(struct sip_msg *msg, str realm);
str cscf_get_public_identity(struct sip_msg *msg);
int cscf_get_expires_hdr(struct sip_msg *msg);
int cscf_get_expires(struct sip_msg *msg);
str cscf_get_public_identity_from_requri(struct sip_msg *msg);

struct sip_msg *cscf_get_request(unsigned int hash,unsigned int label);

int cscf_get_integrity_protected(struct sip_msg *msg,str realm);
int cscf_get_transaction(struct sip_msg *msg, unsigned  int *hash, unsigned int *label);
int cscf_reply_transactional(struct sip_msg *msg, int code, char *text);
str cscf_get_auts(struct sip_msg *msg, str realm);
str cscf_get_nonce(struct sip_msg *msg, str realm);
str cscf_get_algorithm(struct sip_msg *msg, str realm);
str cscf_get_digest_uri(struct sip_msg *msg, str realm);
int cscf_get_nonce_response(struct sip_msg *msg, str realm,str *nonce,str *response);
str cscf_get_user_agent(struct sip_msg *msg);
contact_body_t *cscf_parse_contacts(struct sip_msg *msg);
str cscf_get_path(struct sip_msg *msg);
str cscf_get_event(struct sip_msg *msg);
str cscf_get_asserted_identity(struct sip_msg *msg);
str cscf_get_contact(struct sip_msg *msg);

str cscf_get_first_route(struct sip_msg *msg,struct hdr_field **hr);
int cscf_remove_first_route(struct sip_msg *msg,str value);

int cscf_remove_own_route(struct sip_msg *msg,struct hdr_field **h);

str cscf_get_record_routes(struct sip_msg *msg);

struct hdr_field* cscf_get_next_record_route(struct sip_msg *msg,struct hdr_field *start);

str cscf_get_realm_from_ruri(struct sip_msg *msg);

str cscf_get_identity_from_ruri(struct sip_msg *msg);

str cscf_get_call_id(struct sip_msg *msg,struct hdr_field **hr);
int cscf_get_cseq(struct sip_msg *msg,struct hdr_field **hr);

struct sip_msg* cscf_get_request_from_reply(struct sip_msg *reply);

str cscf_get_called_party_id(struct sip_msg *msg,struct hdr_field **hr);

int cscf_get_subscription_state(struct sip_msg *msg);

// from icscf
int cscf_replace_string(struct sip_msg *msg, str orig,str repl);

struct hdr_field* cscf_get_header(struct sip_msg * msg , str header_name);
struct hdr_field* cscf_get_next_header(struct sip_msg * msg ,
						 str header_name,struct hdr_field* last_header);
struct hdr_field* cscf_get_next_header_type(struct sip_msg * msg ,
						 hdr_types_t type, struct hdr_field* last_header);						 

str cscf_get_headers_content(struct sip_msg * msg , str header_name);


// from pcscf

str cscf_get_visited_network_id(struct sip_msg *msg, struct hdr_field **h);

str cscf_get_authorization(struct sip_msg *msg, struct hdr_field **h);
str cscf_get_authenticate(struct sip_msg *msg,struct hdr_field **h);
str cscf_get_security_client(struct sip_msg *msg,struct hdr_field **h);
str cscf_get_security_verify(struct sip_msg *msg,struct hdr_field **h);


int cscf_del_header(struct sip_msg *msg,struct hdr_field *h);

struct via_body* cscf_get_first_via(struct sip_msg *msg, struct hdr_field **h);
struct via_body* cscf_get_last_via(struct sip_msg *msg);
struct via_body* cscf_get_ue_via(struct sip_msg *msg,str pcscf_sip2ims_via_host,int pcscf_sip2ims_via_port);

str cscf_get_realm(struct sip_msg *msg);

int cscf_get_p_associated_uri(struct sip_msg *msg,str **public_id,int *public_id_cnt);
int cscf_get_first_p_associated_uri(struct sip_msg *msg,str *public_id);

str cscf_get_preferred_identity(struct sip_msg *msg,struct hdr_field **h);
str cscf_get_called_party_id(struct sip_msg *msg,struct hdr_field **hr);

struct hdr_field* cscf_get_next_route(struct sip_msg *msg,struct hdr_field *start);

int cscf_is_myself(str uri);

str cscf_get_content_type(struct sip_msg *msg);

int cscf_get_content_len(struct sip_msg *msg);

str* cscf_get_service_route(struct sip_msg *msg,int *size);

int cscf_get_originating_contact(struct sip_msg *msg,str *host,int *port,int *transport,str pcscf_sip2ims_via_host,int pcscf_sip2ims_via_port);
int cscf_get_terminating_contact(struct sip_msg *msg,str *host,int *port,int *transport);


#endif /* PIS_CSCF_NDS_H */
