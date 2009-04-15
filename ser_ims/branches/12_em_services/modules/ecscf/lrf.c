/**
 * \file
 * 
 * emergency-CSCF - SER module interface
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
#include "../../ut.h"
#include "../../sr_module.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../tm/h_table.h"
#include "../tm/t_reply.h"
#include "../dialog/dlg_mod.h"
#include "../cdp/cdp_load.h"
#include "../../parser/parser_f.h"
#include "../../parser/parse_geoloc.h"
#include "../../parser/contact/parse_contact.h"
#include <lost/client.h>
#include <lost/parsing.h>
#include <lost/pidf_loc.h>

#include "dlg_state.h"
#include "sdp_helpr_funcs.h"
#include "lrf.h"
#include "route.h"
#include "parse_content.h"
#include "sip.h"

extern struct tm_binds tmb;   
extern str ecscf_name_str;	
extern str lrf_sip_uri_str;

static str options_method=    {"OPTIONS",7};
static str accept_hdr=        {"Accept: application/route+xml\r\n",31};
static str no_content_len_hdr={"Content-Length: 0\r\n",19};
static str content_len_hdr_s= {"Content-Length: ",16};
static str content_len_hdr_e= {"\r\n",2};
static str max_fwds_hdr=      {"Max-Forwards: 10\r\n",18};
static str contact_s=         {"Contact: <",10};
static str contact_e=	      {">\r\n",3};
static str route_s=	      {"Route: <",8};
static str route_e=           {">\r\n",3};
static str service_hdr_s=     {"Service: ", 9};
static str service_hdr_e=     {"\r\n", 2};
static str content_type_hdr=  {"Content-Type: application/pidf+xml\r\n",36};
static str esqk_hdr_name=  {"ESQK",4};

extern int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 

xmlNode* get_location(struct sip_msg* msg, loc_fmt * crt_loc_fmt);
int get_pidf_body(struct sip_msg* _m, str * pidf_body);
int get_mixed_body_content(str* mixed_body, str delimiter, unsigned int type, unsigned int subtype, 
	str * body_content);
int get_body_content(struct sip_msg * msg, str * body_content, unsigned int type, unsigned int subtype);

#define NO_PSAP 	"no available PSAP"

/* delete the header ESQK if present in the INVITE reply
 * @param inv_repl - the INVITE reply 
 * @param str1 - not used
 * @param str2 - not used
 * @returns CSCF_RETURN_TRUE if ok, otherwise CSCF_RETURN_FALSE
 */
int E_del_ESQK_info(struct sip_msg * inv_repl, char* str1, char* str2){

	struct hdr_field* esqk_header;

	if((esqk_header = cscf_get_header(inv_repl, esqk_hdr_name))){
		LOG(L_DBG, "DBG:"M_NAME":E_del_ESQK_info: removing the header ESQK with the value %.*s\n",
				esqk_header->body.len, esqk_header->body.s);
		if(cscf_del_header(inv_repl, esqk_header)==0)
			return CSCF_RETURN_FALSE;
	}

	return CSCF_RETURN_TRUE;

}
int E_set_em_info(e_dialog * d, struct sip_msg * opt_repl){

	struct hdr_field* esqk_header;
	str esqk = {NULL, 0};
	str psap_uri = {NULL, 0};
	
	if(d->anonymous)
		goto psap_set;

	if(!(esqk_header = cscf_get_header(opt_repl, esqk_hdr_name))){
		LOG(L_ERR, "ERR:"M_NAME":E_set_em_info: could not found the header %.*s in the OPTIONS reply\n", 
				esqk_hdr_name.len, esqk_hdr_name.s);
		goto error;
	}
	esqk.s = esqk_header->body.s;	
	esqk.len = esqk_header->body.len;	
	if(!esqk.s || !esqk.len){
		LOG(L_ERR, "ERR:"M_NAME":E_set_em_info: empty Esqk header\n");
		goto error;
	}
psap_set:	
	psap_uri = cscf_get_body(opt_repl);
	if(!psap_uri.s || !psap_uri.len){
		LOG(L_ERR, "ERR:"M_NAME":E_set_em_info: empty OPTIONS reply body\n");
		goto error;
	}

	LOG(L_DBG, "DBG:"M_NAME":E_set_em_info: psap_uri is %.*s and esqk value is %.*s\n",
			psap_uri.len, psap_uri.s, esqk.len, esqk.s);

	STR_SHM_DUP(d->psap_uri, psap_uri, "E_set_em_info");
	STR_SHM_DUP(d->esqk, esqk, "E_set_em_info");

	return CSCF_RETURN_TRUE;
error:
out_of_memory:
	if(d->psap_uri.s){
		shm_free(d->psap_uri.s);
		d->psap_uri.s = NULL;
	}
	return CSCF_RETURN_FALSE;
}

/* take actions according to the response code of the OPTIONS reply from the LRF
 * @param opt_repl - the OPTIONS reply
 * @param inv_trans - the INVITE transaction
 * @param code - the code of the response 
 * @returns -1 in case of error, 0 for ok
 */
int E_process_options_repl(struct sip_msg * opt_repl, struct cell * inv_trans, int code){

	int ret;
	ret = -1;
	e_dialog * d = NULL;
	str call_id;

	if(!inv_trans->uas.request){
		LOG(L_ERR, "ERR:"M_NAME":E_process_options_repl: the INVITE message is not set in the INVITE trans\n");
		goto error;
	}

	enum e_dialog_direction dir = get_dialog_direction("orig");
		
	call_id = cscf_get_call_id(inv_trans->uas.request,0);
	if (!call_id.len)
		goto error;

	LOG(L_DBG,"DBG:"M_NAME":E_process_options_repl: Call-ID <%.*s>\n",call_id.len,call_id.s);

	d = get_e_dialog_dir(call_id,dir);
	if(!d){
		LOG(L_ERR, "ERR:"M_NAME":E_process_options_repl:message did not create no dialog\n");
		goto error;
	}

	if(code >= 300){
		//send error response to INVITE and update the dialog
		LOG(L_ERR, "ERR:"M_NAME":E_process_options_repl: received an error response %i\n", code);

		if(tmb.t_reply(inv_trans->uas.request, 500, NO_PSAP)<0){
			LOG(L_ERR, "ERR:"M_NAME":E_process_options_repl:Could not reply to the INVITE request\n");
			goto error;
		}

	}else{
		if(E_set_em_info(d, opt_repl) != CSCF_RETURN_TRUE)
			goto error;	
		if(E_fwd_to_psap(inv_trans->uas.request, d->psap_uri, d->esqk) != CSCF_RETURN_TRUE)
			goto error;
		if(E_add_record_route(inv_trans->uas.request, 0, 0) != CSCF_RETURN_TRUE)
			goto error;
		if(tmb.t_relay(inv_trans->uas.request, 0, 0) != 1){
			LOG(L_ERR, "ERR:"M_NAME":E_process_options_repl:Could not relay the INVITE request\n");
			goto error;
		}
	}
	
	ret = 0;	
error:
	if(d)	d_unlock(d->hash);
	return ret;

}
#define FParam_INT(val) { \
	 .v = { \
		.i = val \
	 },\
	.type = FPARAM_INT, \
	.orig = "int_value", \
};

#define FParam_STRING(val) { \
	.v = { \
		.str = STR_STATIC_INIT(val) \
	},\
	.type = FPARAM_STR, \
	.orig = val, \
};

static fparam_t fp_310 = FParam_INT(310);
static fparam_t fp_int_err = FParam_STRING("Internal Error");

/* callback function for the OPTIONS reply
 * @param t - OPTIONS transaction
 * @param type 
 * @param ps - shm stored parameters of the callback
 */
void options_resp_cb(struct cell* t, int type, struct tmcb_params* ps){

	struct initial_tr * cb_par;
	struct cell* inv_trans, * crt_trans;
	enum route_mode crt_rmode;

	LOG(L_DBG, "DBG:"M_NAME":options_resp_cb: received an %i answer for trans %p, req %p, repl %p\n", 
			ps->code, t, ps->req, ps->rpl);

	cb_par = *((struct initial_tr **)ps->param);
	
	LOG(L_DBG, "DBG:"M_NAME":options_resp_cb: cb_par:%p index: %u label: %u callid: %.*s\n", 
			cb_par, cb_par->hash_index, cb_par->label, cb_par->callid.len, cb_par->callid.s);

	if(tmb.t_enter_ctx(cb_par->hash_index, cb_par->label,
		&crt_rmode, MODE_REQUEST, &crt_trans, &inv_trans)!=0){
		LOG(L_ERR, "ERR:"M_NAME":options_resp_cbp: could not switch to the INVITE transaction\n");
		goto error;
	}

	LOG(L_DBG, "DBG:"M_NAME":options_resp_cb: invite trans %p callid: %.*s\n", 
			inv_trans, inv_trans->callid.len, inv_trans->callid.s);

	if(E_process_options_repl(ps->rpl, inv_trans, ps->code)<0){
	
		LOG(L_ERR, "ERR:"M_NAME":options_resp_cb:Could not process the OPTIONS response\n");
		sl_reply(inv_trans->uas.request, (char*)&fp_310, (char*)&fp_int_err);
	}


	//set the OPTIONS trans as the current one
	tmb.t_exit_ctx(t, crt_rmode);
	LOG(L_DBG, "DBG:"M_NAME":options_resp_cb: ref count of the invite trans is %i\n", inv_trans->ref_count);

error:
	//clean the callback parameters 
	shm_free(cb_par);
	ps->param = NULL;
	return;
}

/**
 * Send an OPTIONS to lrf
 * @returns true if OK, false if not, error on failure
 */
int send_options_req(str req_uri, str location, str service, struct initial_tr * inv_tr){

	str h={0,0};
	struct initial_tr * cb_par;
	int content_len;
	str content_len_str;
	
	LOG(L_DBG,"DBG:"M_NAME":send_options_req: OPTIONS to <%.*s>, service %.*s\n",
		req_uri.len, req_uri.s, service.len, service.s);

	if(!inv_tr){
		LOG(L_ERR, "ERR:"M_NAME":send_options_req:invalid initial trans argument\n");
		return 0;
	}

	if(!req_uri.len || req_uri.s[0] == '\0'){
		LOG(L_ERR, "ERR:"M_NAME":send_options_req:invalid req_uri argument\n");
		return 0;
	}

	content_len = location.len;
	content_len_str.s = int2str((unsigned int) content_len, &content_len_str.len);
	if(content_len_str.len <=0 || content_len_str.len > (INT2STR_MAX_LEN)){
		LOG(L_ERR, "ERR:"M_NAME":send_options_req: could not encode the content length, %i\n", content_len);
		return 0;
	}

	h.len = accept_hdr.len+max_fwds_hdr.len;
	h.len += contact_s.len + ecscf_name_str.len + contact_e.len;

	h.len += route_s.len + lrf_sip_uri_str.len + route_e.len;
	h.len += service_hdr_s.len + service.len + service_hdr_e.len;

	if(!location.len || !location.s)
		h.len +=no_content_len_hdr.len;
	else{
		h.len +=content_len_hdr_s.len + content_len_str.len + content_len_hdr_e.len;
		h.len +=content_type_hdr.len;
	}

	h.s = pkg_malloc(h.len);
	if (!h.s){
		LOG(L_ERR,"ERR:"M_NAME":send_options_req: Error allocating %d bytes\n",h.len);
		h.len = 0;
		return 0;
	}

	char * p = (char*)shm_malloc(sizeof(struct initial_tr)+sizeof(char)*inv_tr->callid.len);
	if(!p){
		LOG(L_ERR,"ERR:"M_NAME":send_options_req: Error allocating cb_par: %d bytes\n",
				sizeof(struct initial_tr)+sizeof(char)*inv_tr->callid.len);
		goto error;
	}
	cb_par = (struct initial_tr *)p;
	cb_par->hash_index = inv_tr->hash_index;
	cb_par->label = inv_tr->label;
	cb_par->callid.len = inv_tr->callid.len;
	cb_par->callid.s = (char*)(p+sizeof(struct initial_tr));
	memcpy(cb_par->callid.s, inv_tr->callid.s, inv_tr->callid.len*sizeof(char));
	
	h.len = 0;
	STR_APPEND(h,accept_hdr);
	
	STR_APPEND(h,service_hdr_s);
	STR_APPEND(h,service);
	STR_APPEND(h,service_hdr_e);

	STR_APPEND(h,max_fwds_hdr);

	STR_APPEND(h,contact_s);
	STR_APPEND(h,ecscf_name_str);
	STR_APPEND(h,contact_e);

	STR_APPEND(h,route_s);
	STR_APPEND(h,lrf_sip_uri_str);
	STR_APPEND(h,route_e);

	if(!location.len || !location.s){
		STR_APPEND(h,no_content_len_hdr);
	}else{
		STR_APPEND(h,content_type_hdr);
		STR_APPEND(h,content_len_hdr_s);
		STR_APPEND(h,content_len_str);
		STR_APPEND(h,content_len_hdr_e);
	}

	LOG(L_DBG, "DBG:"M_NAME":send_options_req: headers are: \n%.*s, cb_par:%p index: %u label: %u\n", 
			h.len, h.s, cb_par, cb_par->hash_index, cb_par->label);
	if (tmb.t_request(&options_method, &req_uri, &lrf_sip_uri_str, &ecscf_name_str, &h, &location, &lrf_sip_uri_str,
                 options_resp_cb, (void*)cb_par)<0){
                LOG(L_ERR,"ERR:"M_NAME":send_options_req: Error sending in transaction\n");
                goto error;
        }
	
	if (h.s) pkg_free(h.s);
	return 1;

error:
	if (h.s) pkg_free(h.s);
	return 0;
}


/* Find the appropriate psap uri that the request should be forwarded to, by querying the LRF
 * @param msg - the sip request
 * @param str1 
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 * TODO: special case: anonymous user
 */
int E_query_LRF(struct sip_msg* msg, char* str1, char* str2){

	e_dialog* d;
	str call_id, from_uri;
	str location_str = {0, 0};
	str service;
	struct initial_tr inv_tr;

	enum e_dialog_direction dir = get_dialog_direction(str1);
	if(dir == DLG_MOBILE_UNKNOWN){
		LOG(L_ERR, "ERR:"M_NAME":E_query_LRF: error invalid argument str1\n");
		return CSCF_RETURN_FALSE;
	}
		
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;

	if(cscf_get_from_uri(msg, &from_uri)==0)
		return CSCF_RETURN_FALSE;
	
	LOG(L_DBG,"DBG:"M_NAME":E_query_LRF: Call-ID <%.*s>\n",call_id.len,call_id.s);

	d = get_e_dialog_dir(call_id,dir);

	if(!d){
		LOG(L_ERR, "ERR:"M_NAME":E_query_LRF:message did not create no dialog\n");
		return CSCF_RETURN_ERROR;
	}

	location_str.s = d->location_str.s;
	location_str.len = d->location_str.len;
	if(!location_str.len || !location_str.s){
		LOG(L_ERR, "ERR:"M_NAME":E_query_LRF:the message contained no supported location information, not handled for the moment\n");
		goto ret_false;
	}


	if(tmb.t_get_trans_ident(msg, &inv_tr.hash_index, &inv_tr.label) != 1){
		LOG(L_ERR, "ERR:"M_NAME":E_query_LRF:could not retrive hash_index and label of the current message's transaction\n");
		goto ret_false;
	}
	inv_tr.callid.len = msg->callid->len; 
	inv_tr.callid.s = msg->callid->name.s; 
	service = msg->first_line.u.request.uri;

	if(!send_options_req(from_uri, location_str, service, &inv_tr)){
		goto ret_false;
	}
	
	d_unlock(d->hash);
	return CSCF_RETURN_TRUE;

ret_false:
	d_unlock(d->hash);
	return CSCF_RETURN_FALSE;
}

/* Parse the location information by value from the INVITE request
 * TODO: if the location information is by reference, or there is no location information
 * store the location information in the dialog
 * @param msg - the SIP request
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if no location-conveyance info is found or #CSCF_RETURN_ERROR on error 
 */
int E_get_location(struct sip_msg* msg, char* str1, char * str2){
	
	e_dialog * d;
	loc_fmt crt_loc_fmt;
	str pidf_body={NULL, 0};
	xmlNode * presence, * loc;
	struct loc_value *value;
	int ret;
	str call_id;

	loc = NULL;

	enum e_dialog_direction dir = get_dialog_direction(str1);
	if(dir == DLG_MOBILE_UNKNOWN){
		LOG(L_ERR, "ERR:"M_NAME":E_get_location: error invalid argument str1\n");
		return CSCF_RETURN_ERROR;
	}
		
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len)
		return CSCF_RETURN_ERROR;

	LOG(L_DBG,"DBG:"M_NAME":E_get_location: Call-ID <%.*s>\n",call_id.len,call_id.s);

	d = get_e_dialog_dir(call_id,dir);

	if(!d){
		LOG(L_ERR, "ERR:"M_NAME":E_get_location:message did not create no dialog\n");
		return CSCF_RETURN_ERROR;
	}

	if(parse_geoloc(msg)){
		
		LOG(L_ERR, "ERR:"M_NAME":E_get_location: error while parsing the Geolocation header\n");
		goto error_loc;
	}	

	print_geoloc((struct geoloc_body*)msg->geolocation->parsed);
	if(!((struct geoloc_body*)msg->geolocation)->retrans_par){
		LOG(L_ERR, "ERR:"M_NAME":E_get_location:the location does not support routing based on location\n");
		goto error_loc;
	}

	for(value = ((struct geoloc_body*)msg->geolocation->parsed)->loc_list;value!=NULL; value=value->next){
	
		if(value->locURI.type != CID_T){
			LOG(L_ERR, "ERR:"M_NAME":E_get_location: geolocation uri type unsupported\n");
			goto error_loc;
		}
	}

	
	if(get_pidf_body(msg, &pidf_body)){
		LOG(L_ERR, "ERR:"M_NAME":E_get_location:could not get the pidf+xml body, but with the Geolocation header set\\n");
		goto error_loc;
	}
	
	//LOG(L_DBG, "DBG:"M_NAME":the pidf body to be parsed is %.*s\n",
	//		pidf_body.len, pidf_body.s);

	if(!(presence = xml_parse_string(pidf_body))){
		LOG(L_ERR, "ERR:"M_NAME": E_get_location:invalid xml content\n");
		goto error_loc;
	}

	//print_element_names(presence);

	if(!(loc = has_loc_info(&ret, presence, &crt_loc_fmt))){
	
		LOG(L_ERR, "ERR:"M_NAME":E_get_location:could not find a valid location element, but with the Geolocation header set\n");
		xmlFreeDoc(presence->doc);
		goto error_loc;
	}

	if((crt_loc_fmt == GEO_SHAPE_LOC) || (crt_loc_fmt == NEW_CIV_LOC) ||
			(crt_loc_fmt == OLD_CIV_LOC) || (crt_loc_fmt == GEO_COORD_LOC)){
	
		LOG(L_DBG, "DBG:"M_NAME":E_get_location:LoST supported format, setting the location\n");

	}else if(crt_loc_fmt == ERR_LOC){
		LOG(L_ERR, "ERR:"M_NAME":E_get_location:error while parsing the location information\n");
		xmlFreeDoc(presence->doc);
		goto error_loc;
	}else{
		LOG(L_DBG, "DBG:"M_NAME":E_get_location:no LoST supported format\n");
		xmlFreeDoc(presence->doc);
		goto error_loc;
	}
//	LOG(L_DBG, "DBG:"M_NAME":get_location:printing the location useful tree\n");
//	print_element_names(loc);

	d->location = loc;
	d->location_str.s = pidf_body.s;
	d->location_str.len = pidf_body.len;
	d->d_loc_fmt = crt_loc_fmt;

	d_unlock(d->hash);
	return CSCF_RETURN_TRUE;
error_loc:
	d_unlock(d->hash);
	return CSCF_RETURN_FALSE;
}

/* get the PIDF-LO body of a request
 * @param msg - the SIP request
 * @param pidf_body - the PIDF-LO body part
 * @returns -1 if not found  or error, 0 if found
 */
int get_pidf_body(struct sip_msg* msg, str * pidf_body){

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

/* get the body with a specific type and subtype from the singlepart or multipart body of a request
 * @param _m - the SIP request
 * @param type and subtype - e.g. application and pdf+xml for a PIDF-LO body (application/pdf+xml)
 * @param body_content - the requested body part
 * @returns -1 if not found or error, 0 if found
 */
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

/* get the body with a specific type and subtype from the multipart body
 * @param mixed_body - the string of the multipart body
 * @param delimiter - the string that is delimiting the different body parts
 * @param type and subtype - e.g. application and pdf+xml for a PIDF-LO body (application/pdf+xml)
 * @param body_content - the requested body part
 * @returns -1 if not found or error, 0 if found
 */
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


