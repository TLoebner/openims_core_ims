/**
 * $Id$
 *   
 * Copyright (C) 2004-2007 FhG Fokus
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
#include "rf.h"

#include "../tm/tm_load.h"
#include "../cdp/cdp_load.h"
#include "../cdp/acct.h"
#include "rf_avp.h"
#include "sip.h"

#include "offline_charging.h"

/**< Structure with pointers to tm funcs */
extern struct tm_binds tmb;

/**< Structure with pointers to cdp funcs */
extern struct cdp_binds cdpb;

/**< Offline Charging flag */
extern struct offline_charging_flag cflag;          

/**< FQDN of CDF for offline charging, defined in mod.c */
extern str cdf_peer;	

/* default configuration of pcscf, used to generate AVPs. */
str service_context_id = {"32260@3gpp.org", 14};
unsigned int role_of_node = AVP_IMS_ORIGINATING_ROLE;
unsigned int node_functionality = AVP_IMS_P_CSCF;


AAAMessage* Rf_ACR_event(struct sip_msg *msg) 
{
	AAAMessage *acr = 0;
	AAASessionId sessId = {0,0};
	AAAMessage *aca = 0;

	sessId = cdpb.AAACreateSession();
	acr = cdpb.AAACreateRequest(IMS_Rf,ACR,Flag_Proxyable,&sessId);
	
	if (!acr) goto error;
	
	/*---------- 1. Add mandatory AVPs ----------*/
	
	/* Session-Id, Origin-Host, Origin-Realm AVP are added by the stack. */
	
	/* Add Destination-Realm AVP */
	str realm = cscf_get_realm_from_ruri(msg);
	if (!Rf_add_destination_realm(acr, realm)) goto error;
	
	/* Add Accounting-Record-Type AVP */
	if (!Rf_add_accounting_record_type(acr, AAA_ACCT_EVENT)) goto error;
	
	/* Add Accounting-Record-Number AVP */
	if (!Rf_add_accounting_record_number(acr, 0)) goto error;
	
	/* Add Acct-Application-Id AVP */
	if (!Rf_add_acct_application_id(acr, IMS_Rf)) goto error;
	
	/* add Service-Context-Id, see TS32.299 V740 7.1.7 */
	if (!Rf_add_service_context_id(acr, service_context_id)) goto error; 

	
	
	/*---------- 2. Create and add Service-Information AVP ----------*/
	
	/*
	 *  See 3GPP TS32.299 V7.4.0:
	 * 
	 *  <Service-Information> = [IMS-Information]
	 * 								 [Event-Type]
	 * 								 [Role-Of-Node]
	 * 								 [Node-Functionality]
	 * 								 [User-Session-Id]
	 * 								*[Calling-Party-Address]
	 * 								 [Called-Party-Address]
	 * 								*[Associated-URI]	
	 * 								 [Time-Stamps]
	 * 								*[Application-Server-Information]	
	 * 								*[Inter-Operator-Identifier]	
	 * 								 [IMS-Charging-Identifier]	
	 * 								*[SDP-Session-Description]
	 * 								*[SDP-Media-Component]
	 * 								 [Served-Party-IP-Address]
	 * 								 [Server-Capabilities]
	 * 								 [Trunk-Group-Id]
	 * 								 [Bearer-Service]
	 * 								 [Service-Id]
	 * 								*[Service-Specific-Info]
	 * 								*[Message-Body]
	 * 								 [Cause-Code]
	 * 								 [Access-Network-Information]
	 *
	 */
	 			 	
	AAA_AVP_LIST ls_ims; /* AVP list for member AVPs in IMS-Information AVP */
	ls_ims.head=0; ls_ims.tail = 0;

	AAA_AVP_LIST ls_ser; /* AVP list for member AVPs in Service-Information AVP */
	ls_ser.head=0; ls_ser.tail = 0;
	
	
	AAA_AVP_LIST ls_tmp;
	ls_tmp.head=0; ls_tmp.tail = 0;
	
	/* Following AVPs are created by P-CSCF directly. */
	if(!Rf_add_role_of_node(&ls_ims, role_of_node)) goto error;
	if(!Rf_add_node_functionality(&ls_ims, node_functionality)) goto error;
	
	
	/* To decide which AVP should be created based on the charging flag. */
	
	/* SIP Request Method --> Event Type:
	 * 
	 * <Event-Type> = [SIP-Method]
	 *                [Event]
	 *                [Expires]
	 * 
	 */
	if ((cflag.cf_3gpp & CF_3GPP_EVENT_TYPE) &&
		(msg->first_line.type == SIP_REQUEST)) {
		
		str method = cscf_get_sip_method(msg);
		if (!Rf_add_sip_method(&ls_tmp, method)) goto error;
			
		str event = cscf_get_event(msg);
		if (event.len) 
			if (!Rf_add_event(&ls_tmp, event)) goto error;
		
		
		int expires = cscf_get_expires_hdr(msg);	
		if (expires >= 0) 
			if (!Rf_add_expires(&ls_tmp, expires)) goto error;
		
			
		if (!Rf_add_event_type(&ls_ims, &ls_tmp)) goto error;
	}
	
	/*  Call-ID --> User-Session-ID AVP */
	if (cflag.cf_3gpp & CF_3GPP_USER_SESSION_ID) {
		str user_session_id = cscf_get_call_id(msg, 0);
		if (user_session_id.len)
			if (!Rf_add_user_session_id(&ls_ims, user_session_id)) goto error;
	}
		
	/* P-Asserted-Identity --> Calling-Party-Address AVP */
	/*
	 * Obtained from P-Asserted-Identity of non-REGISTER SIP request.
	 * 
	 * TODO may appear serveral times when the P-Asserted-Identity header
	 * contains both a SIP URI and a TEL URI.
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLING_PARTY_ADDRESS) && 
		(msg->first_line.type == SIP_REQUEST) && 
		(msg->REQ_METHOD != METHOD_REGISTER)) {
		str calling_addr = cscf_get_asserted_identity(msg);
		if (calling_addr.len)
			if (!Rf_add_calling_party_address(&ls_ims, calling_addr)) goto error; 
			
	}
	
	/* Requst URI / To URI --> Called-Party-Address AVP */
	/* if REG from To URI
	 * otherwise from Request URI
	 */
	if ((cflag.cf_3gpp & CF_3GPP_CALLED_PARTY_ADDRESS) &&
		(msg->first_line.type == SIP_REQUEST)) {
		
		str called_addr;
		
		if (msg->REQ_METHOD & METHOD_REGISTER) {
			called_addr = cscf_get_public_identity(msg);
		} else {
			called_addr = cscf_get_public_identity_from_requri(msg);
		}
		
		if (called_addr.len)
			if (!Rf_add_called_party_address(&ls_ims, called_addr)) goto error;
	}
	
	/* P-Charging-Vector: icid-value -> IMS-Charging-Identifier AVP */
//	if (cflag.cf_3gpp & CF_3GPP_IMS_CHARGING_IDENTIFER) {
//		str icid = cscf_get_p_charging_vector_icid(msg);
//	}
		
	
	
	if (!Rf_add_ims_information(&ls_ser, &ls_ims)) goto error;
	if (!Rf_add_service_information(acr, &ls_ser)) goto error;
	
	/*---------- 3. Send ACR to CDF ----------*/
	aca = cdpb.AAASendRecvMessage(acr, &cdf_peer);
	
	/*---------- 4. Destroy the session ----------*/
	cdpb.AAADropSession(&sessId);
	
	return aca;
	
error:
	//free stuff
	if (sessId.s) cdpb.AAADropSession(&sessId);
	if (acr) cdpb.AAAFreeMessage(&acr);
	return 0;

}

AAAMessage* Rf_ACR_start(struct sip_msg *msg) {
	return 0;
}
 
AAAMessage* Rf_ACR_interim(struct sip_msg *msg) {
	return 0;
}

AAAMessage* Rf_ACR_stop(struct sip_msg *msg) {
	return 0;
}

