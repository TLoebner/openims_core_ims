/* route.c deals with routing of the E-CSCF module
 *
 */

#include "../../parser/msg_parser.h"
#include "../../dset.h"
#include "dlg_state.h"
#include "sip.h"

static str route_s={"Route: <",8};
static str route_e={">\r\n",3};
extern str ecscf_record_route_mo;

static str esqk_hdr_s = {"Esqk: ",6};
static str esqk_hdr_e = {"\r\n",2};

/* Deletes all the Route headers, Adds a Route towards the PSAP URI and sets the RURI to the PSAP URI
 * TODO:add headers like ESQK
 * @param d  - the dialog that the INVITE request created
 */
int E_fwd_to_psap(struct sip_msg * msg, str psap_uri, str esqk){
	
	str x = {0,0}, y = {0,0};

	if(!psap_uri.len || !psap_uri.s){
		LOG(L_ERR, "ERR:"M_NAME":E_fwd_to_psap: invalid psap_uri parameter\n");
		goto ret_false;
	}

	if(!esqk.len || !esqk.s){
		LOG(L_ERR, "ERR:"M_NAME":E_fwd_to_psap: invalid esqk parameter\n");
		goto ret_false;
	}

	if(cscf_del_all_headers(msg, HDR_ROUTE_T)==0){
		LOG(L_ERR, "ERR:"M_NAME":E_fwd_to_psap:could not delete all the existing route headers\n");
		goto ret_false;
	}
		
	x.s = pkg_malloc(route_s.len + psap_uri.len +route_e.len);
	if (!x.s){
		LOG(L_ERR, "ERR"M_NAME":E_fwd_to_psap: Error allocating %d bytes\n",
			psap_uri.len);
		x.len=0;
		goto ret_false;

	}

	y.s = pkg_malloc(esqk_hdr_s.len + esqk.len +esqk_hdr_e.len);
	if (!y.s){
		LOG(L_ERR, "ERR"M_NAME":E_fwd_to_psap: Error allocating %d bytes\n",
			esqk.len);
		y.len=0;
		goto ret_false;
	}

	LOG(L_DBG, "DBG:"M_NAME":E_fwd_to_psap:psap uri is %.*s\n",
			psap_uri.len, psap_uri.s);

	if(rewrite_uri(msg, &psap_uri) < 0) {
		LOG(L_ERR,"ERR:"M_NAME":E_fwd_to_psap: Error rewritting uri with <%.*s>\n",
			psap_uri.len, psap_uri.s);
		goto ret_false;	
	} 

	if(set_dst_uri(msg, &psap_uri)){
	
		LOG(L_ERR, "ERR:"M_NAME":E_fwd_to_psap: Could not set the destination uri %.*s\n",
				psap_uri.len, psap_uri.s);
		goto ret_false;
	}

	x.len = 0;
	STR_APPEND(x,route_s);
	STR_APPEND(x, psap_uri);
	STR_APPEND(x,route_e);	

	if (!cscf_add_header_first(msg,&x,HDR_ROUTE_T)){
		goto ret_false;
	}

	y.len = 0;
	STR_APPEND(y, esqk_hdr_s);
	STR_APPEND(y, esqk);
	STR_APPEND(y, esqk_hdr_e);	

	if (!cscf_add_header_first(msg,&y,HDR_ESQK_T)){
		goto ret_false;
	}

	return CSCF_RETURN_TRUE;

ret_false:
	LOG(L_ERR, "ERR:"M_NAME":E_fwd_to_psap: error while preparing to forward to the PSAP\n");
	if(x.s){
		pkg_free(x.s);
		x.s = NULL;
	}
	if(y.s){
		pkg_free(y.s);
		y.s = NULL;
	}

	return CSCF_RETURN_FALSE;

}

/* Adds a Record-Route header with the URI of the mobile orig of the ECSCF, 
 * for example sip:mo@ecscf.open-ims.test
 * @param msg  - the INVITE request to be added to
 * @param str1 - not used
 * @param str2 - not used
 */
int E_add_record_route(struct sip_msg* msg, char* str1, char* str2){

	str rr={0,0};
	
	STR_PKG_DUP(rr,ecscf_record_route_mo,"pkg");
	
	if (cscf_add_header_first(msg,&rr,HDR_RECORDROUTE_T)) 
		return CSCF_RETURN_TRUE;
	else{
		if (rr.s) {pkg_free(rr.s);
			rr.s = NULL;
		}
		return CSCF_RETURN_BREAK;
	}

	return CSCF_RETURN_TRUE;

out_of_memory:
	LOG(L_ERR, "ERR:"M_NAME":E_add_record_route: out of memory\n");
	return CSCF_RETURN_FALSE;
}

