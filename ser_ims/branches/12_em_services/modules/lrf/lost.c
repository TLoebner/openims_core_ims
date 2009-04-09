/**
 * \file
 * 
 * Location Routing Function - SER module interface
 * 
 * Scope:
 *	interface with the lost_client library 
 *  \author Ancuta Onofrei ancuta_onofrei -at- yahoo dot com
 * 
 */
 

#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mod.h"

#include "../../db/db.h"
#include "../../sr_module.h"
#include "../../socket_info.h"
#include "../../timer.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"
#include "../../parser/parser_f.h"
#include "../../parser/contact/parse_contact.h"
#include <lost/client.h>
#include <lost/parsing.h>
#include <lost/pidf_loc.h>

#include "sdp_helpr_funcs.h"
#include "lost.h"
#include "parse_content.h"
#include "sip.h"
#include "user_data.h"

extern lost_server_info lost_server;	

static str service_hdr_name = {"Service",7};

xmlNode* get_location(struct sip_msg* msg, loc_fmt * crt_loc_fmt);
int get_pidf_lo_body(struct sip_msg* _m, str * pidf_body);
int get_mixed_body_content(str* mixed_body, str delimiter, unsigned int type, unsigned int subtype, 
	str * body_content);
int get_body_content(struct sip_msg * msg, str * body_content, unsigned int type, unsigned int subtype);

int snd_rcv_LoST(str lost_req, str * result){

	CURL* conn;

	//initiate the CURL handle for the HTTP connenction
	//"http://lost.open-ims.test/lost/LoSTServlet";
	LOG(L_DBG, "DBG:"M_NAME":snd_rcv_LoST: lost server: %.*s port %i\n",
			lost_server.host.len, lost_server.host.s,
			lost_server.port);
	conn = lost_http_conn(lost_server.host.s, lost_server.port, result);
	if(!conn){
	
		LOG(L_ERR, "ERR:"M_NAME": snd_rcv_LoST: could not connect to the LoST server [%s]:%i\n",
				lost_server.host.s, lost_server.port);
		goto error;
	}
	
	//send the POST request including the xml body content for LoST
	LOG(L_DBG, "DBG:"M_NAME":snd_rcv_LoST: trying to send %.*s to the lost server\n", 
			lost_req.len, lost_req.s);

	if(send_POST_data(conn, lost_req)){
		LOG(L_ERR, "ERR:"M_NAME": snd_rcv_LoST: could not send data %.*s the LoST server [%s]:%i\n",
		   lost_req.len, lost_req.s, lost_server.host.s, lost_server.port);
		goto error;
	}

	LOG(L_DBG, "DBG:"M_NAME": snd_rcv_LoST: the response from the LoST server is %.*s\n",
			result->len, result->s);
	//disconnect the server
	lost_http_disconn(conn);

	return 0;
error:
	return 1;	
}

#define NO_LOC_FOUND		-3
#define ERR_PARSE_LOC		-4
#define	NO_SUPP_FMT_LOC		-5
#define OK_LOC			1

int LRF_get_location(struct sip_msg* msg, loc_fmt *crt_loc_fmt, xmlNode** loc);

/* Find the appropriate psap uri that the request should be forwarded to
 * @param msg - the sip request from the ECSCF node
 * @param str1 
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 */
int LRF_get_psap(struct sip_msg* msg, char* str1, char* str2){

	str reason, result = {NULL, 0}, lost_req = {NULL, 0};
	lost_resp_type resp_type;
	str psap_uri = {NULL, 0};
	xmlNode* location = NULL, *root= NULL;
	loc_fmt d_loc_fmt;
	struct sip_uri puri;
	expire_type exp_type;
       	time_t exp_timestamp;

	str user_uri;
	user_d * d=NULL;
	str service;

	service = cscf_get_headers_content(msg , service_hdr_name);
	if(!service.len || !service.s){
		LOG(L_ERR, "ERR:"M_NAME":LRF_get_psap: could not find the service header in the OPTIONS, or could not be parsed\n");
		return CSCF_RETURN_FALSE;
	}

	user_uri = msg->first_line.u.request.uri;
	d = get_user_data(user_uri, service);
	if(!d) {
		LOG(L_ERR, "ERR:"M_NAME":LRF_get_psap: could not found user data for uri %.*s and service %.*s\n",
				user_uri.len, user_uri.s, service.len, service.s);
		return CSCF_RETURN_FALSE;
	}

	location = d->loc;
	d_loc_fmt = d->l_fmt;

	if(create_lost_req(location, d_loc_fmt, &lost_req)){
	
		LOG(L_ERR, "ERR:"M_NAME":LRF_get_psap:could not create the LoST request\n");
		goto error;
	}
	
	if(snd_rcv_LoST(lost_req, &result)){
		LOG(L_ERR, "ERR:"M_NAME":LRF_get_psap:could not send the LoST request\n");
		goto error;
	}
	
	//verify what kind of message we have received
	root = get_LoST_resp_type(result, &resp_type, &reason);
	if(resp_type != LOST_OK){
		
		LOG(L_ERR, "ERR:"M_NAME":LRF_get_psap: LoST response type is not OK\n");
		if(reason.s != NULL)
			LOG(L_DBG, "DBG:"M_NAME": LRF_get_psap:reason: %s\n", reason.s);
		
		goto error;
	}

	//get the PSAP URI
	psap_uri = get_mapped_psap(root, &exp_type, &exp_timestamp, &puri);
	if(!psap_uri.s || !psap_uri.len){
		LOG(L_ERR, "ERR:"M_NAME": LRF_get_psap:LoST response had no valid SIP uri\n");
		goto error;
	}

	LOG(L_DBG, "DBG:"M_NAME":LRF_get_psap:psap uri is %.*s\n", psap_uri.len, psap_uri.s);

	//!!!setting a local SIP URI...for testing purposes
	psap_uri.s = "sip:bob@open-ims.test";
	psap_uri.len = strlen(psap_uri.s);

	STR_SHM_DUP(d->psap_uri, psap_uri, "LRF_get_psap");
	lrf_unlock(d->hash);

	pkg_free(result.s);
	if(lost_req.s)
		pkg_free(lost_req.s);
	xmlFreeDoc(root->doc);

	return CSCF_RETURN_TRUE;

error:	
out_of_memory:
	if(d)
		lrf_unlock(d->hash);
	if(root)
		xmlFreeDoc(root->doc);
	if(result.s)
		pkg_free(result.s);
	if(lost_req.s)
		pkg_free(lost_req.s);
	return CSCF_RETURN_FALSE;
}

/* Parse the location information by value from the OPTIONS request
 * store the location information in the user data
 * @param msg - the SIP request
 * @param str1 -not used
 * @param str2 -not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 */
int LRF_has_loc(struct sip_msg* msg, char * str1, char* str2){

	str pidf_body={NULL, 0};

	str user_uri;
	user_d * d= NULL;
	str service;

	service = cscf_get_headers_content(msg , service_hdr_name);
	if(!service.len || !service.s){
		LOG(L_ERR, "ERR:"M_NAME":LRF_has_loc: could not find the service header in the OPTIONS, or could not be parsed\n");
		return CSCF_RETURN_FALSE;
	}

	user_uri = msg->first_line.u.request.uri;
	d = get_user_data(user_uri, service);
	if(!d) {
		LOG(L_ERR, "ERR:"M_NAME":LRF_has_loc: could not found user data for uri %.*s and service %.*s\n",
				user_uri.len, user_uri.s, service.len, service.s);
		return CSCF_RETURN_FALSE;
	}

	if(get_pidf_lo_body(msg, &pidf_body)<0){
		LOG(L_ERR, "ERR:"M_NAME":LRF_has_loc:could not get the pidf+xml body\n");

		lrf_unlock(d->hash);
		return NO_LOC_FOUND;
	}

	d->loc_str.s = pidf_body.s;
	d->loc_str.len = pidf_body.len;
	lrf_unlock(d->hash);
	
	return CSCF_RETURN_TRUE;
}

int LRF_save_user_loc(struct sip_msg * msg, char* str1, char* str2){

	loc_fmt crt_loc_fmt;
	xmlNode* loc;
	str pidf_body={NULL, 0};
	xmlNode *presence;
	int ret;

	str user_uri;
	user_d * d=NULL;
	str service;

	loc = NULL;


	service = cscf_get_headers_content(msg , service_hdr_name);
	if(!service.len || !service.s){
		LOG(L_ERR, "ERR:"M_NAME":LRF_save_user_loc: could not find the service header in the OPTIONS, or could not be parsed\n");
		return CSCF_RETURN_FALSE;
	}

	user_uri = msg->first_line.u.request.uri;
	d = get_user_data(user_uri, service);
	if(!d) {
		LOG(L_ERR, "ERR:"M_NAME":LRF_save_user_loc: could not found user data for uri %.*s and service %.*s\n",
				user_uri.len, user_uri.s, service.len, service.s);
		return CSCF_RETURN_FALSE;
	}

	pidf_body.s = d->loc_str.s;
	pidf_body.len = d->loc_str.len;

	if(!(presence = xml_parse_string(pidf_body))){
		LOG(L_ERR, "ERR:"M_NAME": LRF_save_user_loc:invalid xml content\n");
		lrf_unlock(d->hash);
		return ERR_PARSE_LOC;
	}

	//print_element_names(presence);

	if(!(loc = has_loc_info(&ret, presence, &crt_loc_fmt))){
	
		LOG(L_ERR, "ERR:"M_NAME":LRF_save_user_loc:could not find a valid location element\n");
		xmlFreeDoc(presence->doc);
		lrf_unlock(d->hash);
		return ERR_PARSE_LOC;
	}

	if((crt_loc_fmt == GEO_SHAPE_LOC) || (crt_loc_fmt == NEW_CIV_LOC) ||
			(crt_loc_fmt == OLD_CIV_LOC) || (crt_loc_fmt == GEO_COORD_LOC)){
	
		LOG(L_DBG, "DBG:"M_NAME":LRF_save_user_loc:LoST supported format, setting the location\n");

	}else if(crt_loc_fmt == ERR_LOC){
		LOG(L_ERR, "ERR:"M_NAME":LRF_save_user_loc:error while parsing the location information\n");
		xmlFreeDoc(presence->doc);
		lrf_unlock(d->hash);
		return ERR_PARSE_LOC;
	}else{
		LOG(L_DBG, "DBG:"M_NAME":LRF_save_user_loc:no LoST supported format\n");
		xmlFreeDoc(presence->doc);
		lrf_unlock(d->hash);
		return NO_SUPP_FMT_LOC;
	}
//	LOG(L_DBG, "DBG:"M_NAME":LRF_save_user_loc:printing the location useful tree\n");
//	print_element_names(loc);
	d->loc = loc;
	d->l_fmt = crt_loc_fmt;
	lrf_unlock(d->hash);
	return OK_LOC;
}


int get_pidf_lo_body(struct sip_msg* msg, str * pidf_body){

	unsigned int type, subtype;

	type = TYPE_APPLICATION;
	subtype = SUBTYPE_PIDFXML;

	if(get_body_content(msg, pidf_body, type, subtype)){
	
		LOG(L_ERR, "ERR:"M_NAME":get_pidf_body:an error has occured while retrieving the pidf+xml information");
		return -1;
	}
	
	LOG(L_DBG, "DBG:"M_NAME":get_pidf_body:content body for pidf+xml object is %.*s\n",
			pidf_body->len, pidf_body->s);
	return 0;
}

int get_body_content(struct sip_msg * _m, str * body_content, unsigned int type, unsigned int subtype){

	str body, mp_delimiter;
	int mime;

	body.s = get_body(_m);
	if (body.s==0) {
		LOG(L_ERR, "ERR:"M_NAME":get_body_content:failed to get the message body\n");
		return -1;
	}

	body.len = _m->len -(int)(body.s - _m->buf);
	if (body.len==0) {
		LOG(L_DBG, "DBG:"M_NAME":get_body_content:message body has length zero\n");
		return 1;
	}

	mime = ecscf_parse_content_type_hdr(_m);
	if (mime <= 0) {
		return -1;
	}

	if((((unsigned int)mime)>>16) == type){
	       if((mime&0x00ff) == subtype){
			body_content->s = body.s;
			body_content->len = body.len;
			return 0;
		}else{
			LOG(L_DBG, "DBG:"M_NAME":get_body_content:TYPE_APPLICATION: unknown %d\n",mime&0x00ff);
			return -1;
		}
	} else if ((((unsigned int)mime)>>16) == TYPE_MULTIPART){
		switch (mime&0x00ff) {
		case SUBTYPE_MIXED:
			if(get_mixed_part_delimiter(&(_m->content_type->body),&mp_delimiter) > 0) {
				LOG(L_DBG, "DBG:"M_NAME":get_body_content: mp_delimiter is %.*s\n", 
						mp_delimiter.len, mp_delimiter.s);
				return get_mixed_body_content(&body, mp_delimiter, type, subtype, body_content);
			} else {
				LOG(L_ERR, "ERR:"M_NAME":get_body_content:could not get the delimiter of the multipart content\n");
				return -1;
			}
		default:
			LOG(L_DBG, "DBG:"M_NAME":get_body_content: TYPE_MULTIPART: unknown %d\n",mime&0x00ff);
			return -1;
		}
	} 

	LOG(L_DBG, "DBG:"M_NAME":get_body_content: type of mime unknown:%d\n",((unsigned int)mime)>>16);
	return -1;
}

int get_mixed_body_content(str* mixed_body, str delimiter, unsigned int type, unsigned int subtype, 
		str * body_content){

	int no_eoh_found, found;
	char *bodylimit, *rest;
	char *d1p, *d2p;
	char *ret, *end;
	unsigned int mime;
	str cnt_disp;
	struct hdr_field hf;

	bodylimit = mixed_body->s + mixed_body->len;
	d1p = find_sdp_line_delimiter(mixed_body->s, bodylimit, delimiter);
	if (d1p == NULL) {
		LOG(L_ERR, "ERR:"M_NAME":get_mixed_body_content: empty multipart content\n");
		return -1;
	}
	found = 0;
	d2p = d1p;

	for(;!found && d1p != NULL && d1p < bodylimit; d1p = d2p) {
		
		d2p = find_next_sdp_line_delimiter(d1p, bodylimit, delimiter, bodylimit);
		/* d2p is text limit for application parsing */
		memset(&hf,0, sizeof(struct hdr_field));
		rest = eat_line(d1p + delimiter.len + 2, d2p - d1p - delimiter.len - 2);
		if ( rest > d2p ) {
			LOG(L_ERR, "ERR:"M_NAME":get_mixed_body_content:Unparsable <%.*s>\n", (int)(d2p-d1p), d1p);
			return -1;
		}
		no_eoh_found = 1;
		found = 0;
		/*LM_DBG("we need to parse this: <%.*s>\n", d2p-rest, rest); */
		while( rest<d2p && no_eoh_found ) {
			rest = get_sdp_hdr_field(rest, d2p, &hf);
			switch (hf.type){
			case HDR_EOH_T:
				no_eoh_found = 0;
				break;
			case HDR_CONTENTTYPE_T:
				end = hf.body.s + hf.body.len;
				ret = ecscf_decode_mime_type(hf.body.s, end , &mime);
				if (ret==0)
					return -1;
				if (ret!=end) {
					LOG(L_ERR, "ERR:"M_NAME":get_mixed_body_content:the header CONTENT_TYPE contains "
						"more then one mime type :-(!\n");
					return -1;
				}
				if ((mime&0x00ff)==SUBTYPE_ALL || (mime>>16)==TYPE_ALL) {
					LOG(L_ERR, "ERR:"M_NAME":get_mixed_body_content:invalid mime with wildcard '*' in Content-Type hdr!\n");
					return -1;
				}
				
				if (((((unsigned int)mime)>>16) == type) && ((mime&0x00ff) == subtype)) {
					found = 1;
				}
				break;
			case HDR_CONTENTDISPOSITION_T:
				cnt_disp.s = hf.body.s;
				cnt_disp.len = hf.body.len;
				break;
			case HDR_ERROR_T:
				return -1;
				break;
			default:
				LOG(L_DBG, "DBG:"M_NAME":get_mixed_body_content:unknown header: <%.*s:%.*s>\n",hf.name.len,hf.name.s,hf.body.len,hf.body.s);
			}
		} /* end of while */
	}
	if(!found)
		return -1;

	body_content->s = rest;
	body_content->len = d2p-rest;

	str_trim(body_content);

	return 0;
}


