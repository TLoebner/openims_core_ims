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
 

#include "mod.h"

#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"
#include <lost/client.h>
#include <lost/parsing.h>
#include <lost/pidf_loc.h>

#include "lost.h"
#include "multipart_parse.h"
#include "sip.h"
#include "user_data.h"

extern lost_server_info lost_server;	
extern str local_psap_uri_str;
extern int using_lost_srv;

static str service_hdr_name = {"Service",7};

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

/* Get the PSAP URI by interrogating the LoST server
 * @param d - the user data for which the PSAP URI is searched
 * @returns a null string if error, otherwise the PSAP URI
 */
str get_psap_by_LoST(user_d * d){

	str reason, psap_uri = {NULL, 0}, result = {NULL, 0}, lost_req = {NULL, 0};
	lost_resp_type resp_type;
	xmlNode* location = NULL, *root= NULL;
	loc_fmt d_loc_fmt;
	struct sip_uri puri;
	expire_type exp_type;
       	time_t exp_timestamp;

	location = d->loc;
	d_loc_fmt = d->l_fmt;

	if(create_lost_req(location, d_loc_fmt, &lost_req)){
	
		LOG(L_ERR, "ERR:"M_NAME":LRF_get_psap:could not create the LoST request\n");
		goto end;
	}
	
	if(snd_rcv_LoST(lost_req, &result)){
		LOG(L_ERR, "ERR:"M_NAME":LRF_get_psap:could not send the LoST request, setting the default PSAP URI\n");
		goto end;
	}
	
	//verify what kind of message we have received
	root = get_LoST_resp_type(result, &resp_type, &reason);
	if(resp_type != LOST_OK){
		
		LOG(L_ERR, "ERR:"M_NAME":LRF_get_psap: LoST response type is not OK\n");
		if(reason.s != NULL)
			LOG(L_DBG, "DBG:"M_NAME": LRF_get_psap:reason: %s\n", reason.s);
		
		goto end;
	}

	//get the PSAP URI
	psap_uri = get_mapped_psap(root, &exp_type, &exp_timestamp, &puri);
	if(!psap_uri.s || !psap_uri.len){
		LOG(L_ERR, "ERR:"M_NAME": LRF_get_psap:LoST response had no valid SIP uri\n");
		goto end;
	}

	LOG(L_DBG, "DBG:"M_NAME":LRF_get_psap:found psap uri is %.*s\n", psap_uri.len, psap_uri.s);
	
end:
	if(result.s)
		pkg_free(result.s);
	if(lost_req.s)
		pkg_free(lost_req.s);
	if(root)
		xmlFreeDoc(root->doc);
	return psap_uri;
}

/* Find the appropriate psap uri that the request should be forwarded to
 * @param msg - the sip request from the ECSCF node
 * @param str1 
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 */
int LRF_get_psap(struct sip_msg* msg, char* str1, char* str2){

	str psap_uri = {NULL, 0};
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

	//if not using a LoST server
	if(using_lost_srv){
		psap_uri = get_psap_by_LoST(d);
		if(!psap_uri.s || !psap_uri.len)
			goto error;
	}else{
		//setting a local SIP URI
		psap_uri.s = local_psap_uri_str.s;
		psap_uri.len = local_psap_uri_str.len;
	}

	LOG(L_DBG, "DBG:"M_NAME":LRF_get_psap:psap uri is %.*s\n", psap_uri.len, psap_uri.s);

	STR_SHM_DUP(d->psap_uri, psap_uri, "LRF_get_psap");
	
	lrf_unlock(d->hash);
	return CSCF_RETURN_TRUE;

error:	
out_of_memory:
	if(d)
		lrf_unlock(d->hash);
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



