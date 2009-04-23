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
 * Proxy-CSCF -Emergency Related Operations
 * 
 * 
 *  \author Ancuta Onofrei	andreea dot ancuta dot onofrei -at- fokus dot fraunhofer dot de
 */
 
#include <time.h>
 
#include "../../data_lump.h"
#include "../../data_lump_rpl.h"
#include "../../mem/mem.h"
#include "../../locking.h"
#include "../tm/tm_load.h"

#include "mod.h"
#include "sip.h"
#include "emerg.h"
#include "registrar.h"
#include "registrar_subscribe.h"
#include "ims_pm_pcscf.h"
#include "e2.h"
#include "e2_avp.h"
#include "dlg_state.h"

extern struct tm_binds tmb;            				/**< Structure with pointers to tm funcs 			*/
extern int emerg_support;
extern str ecscf_uri_str;
extern int anonym_em_call_support;

/*global variables*/
str ecscf_uri_str;
xmlDocPtr reply_380_doc= NULL;
xmlNode * alt_serv_node = NULL;
xmlNode * reason_alt_serv_node = NULL;
xmlNode * action_alt_serv_node = NULL;

int init_em_alt_serv_body();

int init_emergency_cntxt(){
	
	/*init the XML library*/
	xmlInitParser();
	
	/*init the xml doc*/
	return init_em_alt_serv_body();
}

/* Contructing the rough XML body for the 380 Alternative Service reply
 * when used the value of the node "reason" can be set, using xmlNodeSetContent 
 * and the "action_alt_serv" node can be temporarily removed if the request did not contain an emergency service URN,
 * using xmlUnlinkNode and in the end: AddChild	
 */
int init_em_alt_serv_body(){

   	xmlNodePtr em_alt_serv_node, type_node;

	/* creating the xml doc*/
   	reply_380_doc= xmlNewDoc(BAD_CAST "1.0");
   	if(reply_380_doc== NULL){

   		LOG(L_ERR, "ERR:"M_NAME":init_em_alt_serv_body: when creating new xml doc\n");
   		goto error;
   	}
   	
	em_alt_serv_node = xmlNewNode(NULL, BAD_CAST IMS_3GPP_XML_NODE);
   	if(em_alt_serv_node==0){

		LOG(L_ERR, "ERR:"M_NAME":init_em_alt_serv_body: when adding new node %s\n", IMS_3GPP_XML_NODE);
   		goto error;
   	}
   	xmlDocSetRootElement(reply_380_doc, em_alt_serv_node);

	/*if(!xmlNewNs(root_node, BAD_CAST LOST_NS_HREF, NULL)){
		ERROR_LOG("could not add the namespace %s to the root node\n",
				LOST_NS_HREF);
		goto error;
	}*/

	alt_serv_node = xmlNewChild(em_alt_serv_node, NULL, BAD_CAST ALTERN_SERV_XML_NODE, NULL);
	if(!alt_serv_node){
		LOG(L_ERR, "ERR:"M_NAME":init_em_alt_serv_body: when adding new node %s\n", ALTERN_SERV_XML_NODE);
		goto error;
	}

	type_node = xmlNewChild(alt_serv_node, NULL, BAD_CAST TYPE_XML_NODE, BAD_CAST ALT_SERV_TYPE_VAL);
	if(!type_node){
		LOG(L_ERR, "ERR:"M_NAME":init_em_alt_serv_body: when adding new node %s\n", TYPE_XML_NODE);
		goto error;
	}

	reason_alt_serv_node = xmlNewChild(alt_serv_node, NULL, BAD_CAST REASON_XML_NODE, BAD_CAST "");
	if(!type_node){
		LOG(L_ERR, "ERR:"M_NAME":init_em_alt_serv_body: when adding new node %s\n", REASON_XML_NODE);
		goto error;
	}

	action_alt_serv_node = xmlNewChild(alt_serv_node, NULL, BAD_CAST ACTION_XML_NODE, BAD_CAST ALT_SERV_ACTION_VAL);
	if(!type_node){
		LOG(L_ERR, "ERR:"M_NAME":init_em_alt_serv_body: when adding new node %s\n", ACTION_XML_NODE);
		goto error;
	}

	return 0;
error:
	return -1;
}

#define check_sos_URN(_uri, _type, _len)\
	do{\
		if((_uri[SOS_URN_LEN+_len+1] == '\0') && \
				(strncmp(_uri+SOS_URN_LEN+1, _type, _len) ==0)){\
				LOG(L_DBG, "DBG:"M_NAME":emergency_URN: call to %s\n",_type);\
				goto is_emerg;\
		}\
	}while(0);


int emergency_urn(char* uri,  int len){

	if((len < SOS_URN_LEN ) || (strncmp(uri,SOS_URN, SOS_URN_LEN)!=0))
		goto not_emerg;

	if(len == SOS_URN_LEN)
		goto is_emerg;

	if(uri[SOS_URN_LEN] !='.'){	
		goto error;
	}

	switch(uri[SOS_URN_LEN+1]){
		case 'a':
			check_sos_URN(uri, SOS_URN_AMB, SOS_URN_AMB_LEN);
			check_sos_URN(uri, SOS_URN_AN_CTR, SOS_URN_AN_CTR_LEN);
			break;			
		case 'f':
			check_sos_URN(uri, SOS_URN_FIRE, SOS_URN_FIRE_LEN);
			break;
		case 'g':
			check_sos_URN(uri, SOS_URN_GAS, SOS_URN_GAS_LEN);
			break;
		case 'm':
			check_sos_URN(uri, SOS_URN_MAR, SOS_URN_MAR_LEN);
			check_sos_URN(uri, SOS_URN_MOUNT, SOS_URN_MOUNT_LEN);
			break;
		case 'p':
			check_sos_URN(uri, SOS_URN_POL, SOS_URN_POL_LEN);
			check_sos_URN(uri, SOS_URN_POIS, SOS_URN_POIS_LEN);
			check_sos_URN(uri, SOS_URN_PHYS, SOS_URN_PHYS_LEN);
		default:
			break;
	}

error:
	LOG(L_DBG, "DBG:"M_NAME":emergency_urn: invalid emergency URI %.*s\n",
			len, uri);
	return CSCF_RETURN_ERROR;

not_emerg:
	LOG(L_DBG, "DBG:"M_NAME":emergency_urn: no emergency URI %.*s\n",
			len, uri);

	return CSCF_RETURN_FALSE;

is_emerg:
	LOG(L_DBG, "DBG:"M_NAME":emergency_urn: we have an emergency call for the URI %.*s\n",
			len, uri);
	return CSCF_RETURN_TRUE;
}

/* Checks if the Request Uri is used for an Emergency Service
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if the case, #CSCF_RETURN_FALSE if not
 */
int P_emergency_ruri(struct sip_msg *msg, char* str1, char* str2){

	return emergency_urn(msg->first_line.u.request.uri.s,
				msg->first_line.u.request.uri.len);
}

/**
 * check if the P-CSCF should accept anonymous Emergency calls
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if anonymous user, #CSCF_RETURN_FALSE if not 
 */
int P_accept_anonym_em_call(struct sip_msg *msg,char *str1,char *str2)
{
	LOG(L_INFO,"DBG:"M_NAME":P_accept_anonym_em_call: Check if the P-CSCF is configured to accept an anonymous emergency call or not\n");

	if(anonym_em_call_support)	
		return CSCF_RETURN_TRUE;
	else	
		return CSCF_RETURN_FALSE;
}

/**
 * Finds if the message comes from a user that made an Emergency Registration
 * @param msg - the SIP message
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if sos uri parameter in Contact header, #CSCF_RETURN_FALSE if not 
 */
int P_emergency_flag(struct sip_msg *msg,char *str1,char *str2)
{
	contact_t *c;
	int sos_reg;
	contact_body_t * contact_bd=NULL;

	sos_reg = 0;

	LOG(L_INFO,"DBG:"M_NAME":P_emergency_flag: Check if the user made an Emergency Registration\n");

	//contact parsed
	if(!(contact_bd = cscf_parse_contacts(msg)) ){
		LOG(L_ERR, "ERR:"M_NAME":P_emergency_flag: parsing Contact header failed\n");
		return CSCF_RETURN_ERROR;
	}
	
	for(c=contact_bd->contacts;c;c=c->next){
		LOG(L_DBG,"DBG:"M_NAME":P_emergency_flag: contact <%.*s>\n",c->uri.len,c->uri.s);
			
		sos_reg += cscf_get_sos_uri_param(c->uri);
		if(sos_reg < 0)
			return CSCF_RETURN_FALSE;
	}
	
	if(sos_reg)
		return CSCF_RETURN_TRUE;

	return CSCF_RETURN_FALSE;
}

/**
 * Finds if the message comes from an emergency registered UE at this P-CSCF
 * @param msg - the SIP message
 * @param str1 - the realm to look into
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if registered, #CSCF_RETURN_FALSE if not 
 */
int P_is_em_registered(struct sip_msg *msg,char *str1,char *str2)
{
	int ret=CSCF_RETURN_FALSE;
	struct via_body *vb;

	LOG(L_INFO,"DBG:"M_NAME":P_is_em_registered: Looking if it has emergency registered\n");
//	print_r(L_INFO);

	vb = cscf_get_ue_via(msg);

	
	if (vb->port==0) vb->port=5060;
	LOG(L_INFO,"DBG:"M_NAME":P_is_em_registered: Looking for <%d://%.*s:%d>\n",
		vb->proto,vb->host.len,vb->host.s,vb->port);
	
	if (r_is_registered(vb->host,vb->port,vb->proto, EMERG_REG)) 
		ret = CSCF_RETURN_TRUE;
	else 
		ret = CSCF_RETURN_FALSE;	
	
	return ret;
}


int select_ECSCF(str * ecscf_used){

	ecscf_used->s = ecscf_uri_str.s;
	ecscf_used->len = ecscf_uri_str.len;

	return 0;
}

static str route_s={"Route: <",8};
static str route_e={">\r\n",3};

/**
 * Inserts the Route header containing the ecscf selected to be enforced
 * @param msg - the SIP message to add to
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok or #CSCF_RETURN_ERROR on error
 */
int P_enforce_sos_routes(struct sip_msg *msg,char *str1,char*str2)
{
	str newuri={0,0};
	str x = {0,0};
	p_dialog *d = NULL;
	str sel_ecscf_uri, call_id, host;
	int port,transport;
	enum p_dialog_direction dir;
	
	dir = DLG_MOBILE_ORIGINATING;
	
	if (!find_dialog_contact(msg,dir,&host,&port,&transport)){
		LOG(L_ERR,"ERR:"M_NAME":P_enforce_sos_routes(): Error retrieving orig contact\n");
		return CSCF_RETURN_BREAK;
	}		
		
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;

	LOG(L_DBG,"DBG:"M_NAME":P_enforce_sos_routes(): Call-ID <%.*s>\n",call_id.len,call_id.s);

	d = get_p_dialog(call_id,host,port,transport,&dir);
	if(!d){
		LOG(L_ERR, "ERR:"M_NAME":P_enforce_sos_routes: could not find the emergency dialog\n");
		return CSCF_RETURN_BREAK;
	}

	if(!d->em_info.em_dialog){
		LOG(L_ERR, "ERR:"M_NAME":P_enforce_sos_routes: script error: trying to use Emergency Services to route a non-emergency call\n");
		goto error;
	}

	if(select_ECSCF(&sel_ecscf_uri))
		goto error;

	x.len = route_s.len + route_e.len + sel_ecscf_uri.len;
			
	x.s = pkg_malloc(x.len);
	if (!x.s){
		LOG(L_ERR, "ERR:"M_NAME":P_enforce_sos_routes: Error allocating %d bytes\n",
			x.len);
		x.len=0;
		goto error;
	}
	x.len=0;
	STR_APPEND(x,route_s);
	STR_APPEND(x,sel_ecscf_uri);
	STR_APPEND(x,route_e);
	
	if(set_dst_uri(msg, &sel_ecscf_uri)){
	
		LOG(L_ERR, "ERR:"M_NAME":P_enforce_sos_routes: Could not set the destination uri %.*s\n",
				sel_ecscf_uri.len, sel_ecscf_uri.s);
		goto error;
	}

	if (cscf_add_header_first(msg,&x,HDR_ROUTE_T)) {
		if (cscf_del_all_headers(msg,HDR_ROUTE_T))
			goto end;
		else {
			LOG(L_ERR,"ERR:"M_NAME":P_enforce_sos_routes: new Route header added, but failed to drop old ones.\n");
		}
	}

error:
out_of_memory:
	if(d) d_unlock(d->hash);
	if (x.s) pkg_free(x.s);
	if(newuri.s) pkg_free(newuri.s);
	LOG(L_ERR, "ERR:"M_NAME":P_enforce_sos_routes: could not select an ECSCF\n");
	return CSCF_RETURN_ERROR;
end:
	STR_SHM_DUP(d->em_info.ecscf_uri, sel_ecscf_uri, "P_enforce_sos_routes");
	d_unlock(d->hash);
	return CSCF_RETURN_TRUE;
	
}

/* Check if the module has Emergency Services enabled
 * @param msg - not used
 * @param str1 - not used
 * @param str2 - not used
 */
int P_emergency_serv_enabled(struct sip_msg *msg,char *str1,char*str2){

	return (emerg_support>0)?CSCF_RETURN_TRUE:CSCF_RETURN_FALSE;
}

int fixup_380_alt_serv(void** param, int param_no){

	char* str1;

	if(param_no!=1){
		LOG(L_ERR, "ERR:"M_NAME":fixup_380_alt_serv: invalid param number");
		return -1;
	}

	str1 = (char*) *param;
	if(!str1 || str1[0] == '\0'){
	
		LOG(L_ERR, "ERR:"M_NAME":fixup_380_alt_serv: NULL reason");
		return -1;
	}

	return 0;
}


//Content-type as specified in TS 24.229, subsection 7.6
#define IMS_3GPP_XML_CNTENT_TYPE "Content-type: application/3gpp-ims+xml;schemaversion=\"1\"\n"
str Cont_type_3gpp_app_xml= {IMS_3GPP_XML_CNTENT_TYPE, (sizeof(IMS_3GPP_XML_CNTENT_TYPE)-1)};
static str invite_method={"INVITE",6}; 
/* Create the body of a 380 Alternative Service reply for Emergency reasons
 * @param msg - the SIP Request
 * @param str1 - the reason of the 380 reply
 * @param str2 - not used
 */
int P_380_em_alternative_serv(struct sip_msg * msg, char* str1, char* str2){

	str body_str = {0, 0};
	xmlChar * body = NULL;
       	const xmlChar *reason;
	int len = 0, ret;

	ret = CSCF_RETURN_FALSE;

	if(!reply_380_doc){
		LOG(L_ERR, "ERR:"M_NAME":P_380_em_alternative_service: the xml body of the reply was not intialized\n");
		return CSCF_RETURN_FALSE;
	}
	
	if(msg->first_line.u.request.method.len == invite_method.len &&
			strncmp(msg->first_line.u.request.method.s, invite_method.s, invite_method.len) == 0){
		
		
		ret = emergency_urn(msg->first_line.u.request.uri.s, 
				msg->first_line.u.request.uri.len);
		if(ret == CSCF_RETURN_ERROR)
			return CSCF_RETURN_ERROR;
	}

	reason = (xmlChar*) str1;
	xmlNodeSetContent(reason_alt_serv_node, BAD_CAST reason);

	if(ret == CSCF_RETURN_FALSE){
		xmlUnlinkNode(action_alt_serv_node);	
	}

	xmlDocDumpFormatMemoryEnc(reply_380_doc, &body, &len, IMS_3GPP_XML_ENC, 1);
	xmlAddChild(alt_serv_node, action_alt_serv_node);

	body_str.s = (char*) body;
	body_str.len = len;
	if(!body_str.s || !body_str.len){
		LOG(L_ERR, "ERR:"M_NAME":P_380_em_alternative_service: could not output the xml document\n");
		return CSCF_RETURN_FALSE;
	}

	LOG(L_DBG, "DBG:"M_NAME":P_380_em_alternative_service: the body for the 380 reply is:\n %.*s\n",
			body_str.len, body_str.s);

	cscf_add_header_rpl(msg, &Cont_type_3gpp_app_xml);
	
	if (add_lump_rpl( msg, body_str.s, body_str.len, LUMP_RPL_BODY)==0) {
		LOG(L_ERR, "ERR:"M_NAME":cscf_add_header_rpl: Can't add header <%.*s>\n",
			body_str.len,body_str.s);
 		return 0;
 	}

	return CSCF_RETURN_TRUE;
}
