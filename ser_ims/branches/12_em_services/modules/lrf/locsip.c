/*
 *  $Id: locsip.c 655 2009-08-7 15:31:52Z aon $
 *
 * Copyright (C) 2009 FhG Fokus
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
 * author Ancuta Onofrei, 
 * 	email andreea dot ancuta dot onofrei -at- fokus dot fraunhofer dot de
 */
/**
 * Location Retrieving Function module
 * 
 * Scope:
 * - LOCSIP interface, see http://member.openmobilealliance.org/ftp/Public_documents/LOC/Permanent_documents/
 * 
 */


#include "mod.h"
#include "sip.h"
#include "locsip.h"
#include "locsip_subscribe.h"

extern int locsip_srv_port;
extern str locsip_srv_ip_s;
extern int use_locsip;
extern str locsip_server_route;

/*
 * check if the LRF was configured to use the LOCSIP interface
 */
int LRF_uses_LOCSIP(struct sip_msg* msg, char * str1, char* str2){

	if(use_locsip == 0){

		return CSCF_RETURN_FALSE;
	}
	return CSCF_RETURN_TRUE;
}

/**
 * Subscribe to the location event to the LOCSIP server
 * @param req - OPTIONS req from the E-CSCF
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if subscribed, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_ERROR on error
 */
int LRF_subscribe_LOCSIP(struct sip_msg *req, char* str1, char* str2)
{
	int max_expires;
	str public_id={0,0};
	loc_subscription * subscr;
	

	public_id.s = req->first_line.u.request.uri.s;
	public_id.len = req->first_line.u.request.uri.len;
	
	LOG(L_DBG, "DBG:"M_NAME":LRF_subscribe_LOCSIP: trying to send a subscribe for public id %.*s\n",
			public_id.len, public_id.s);

	max_expires = 0;
	
	if (public_id.s){
		subscr = loc_subscribe(public_id,max_expires);
		if(!subscr)
			return CSCF_RETURN_FALSE;
		LOG(L_DBG, "DBG:"M_NAME":LRF_subscribe_LOCSIP: created a subscription for %.*s\n",
				subscr->req_uri.len, subscr->req_uri.s);
		if(!loc_send_subscribe(subscr, locsip_server_route, max_expires)){
			LOG(L_ERR, "ERR:"M_NAME":LRF_subscribe_LOCSIP: could not send the subscription\n");
			return CSCF_RETURN_FALSE;
		}
		return CSCF_RETURN_TRUE;
	}else{
		return CSCF_RETURN_FALSE;
	}
}


int LRF_return_default_PSAP(struct sip_msg* msg, char * str1, char* str2){

	return CSCF_RETURN_FALSE;
}



