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
 * SIP-to-IMS Gateway - Gm interface
 * 
 * For now, only authentication translation
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include <time.h>
 
#include "gm.h"

#include "../../data_lump.h"
#include "../../mem/mem.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../../parser/msg_parser.h"
#include "../../parser/hf.h"
#include "mod.h"
#include "sip.h"
#include "registration.h"


/**
 * Convert the response in the message from MD5 to an AKA response.
 * @param msg - SIP message to apply to
 * @param str1 - whether to do a AKA authorized response or not (1/0)
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not
 */
int	Gw_MD5_to_AKA(struct sip_msg *msg,char *str1,char *str2)
{
	str auth={0,0},new_auth={0,0};;
	struct hdr_field *hdr;
	int is_authorized=0;
	
	/* check if we received what we should */
	if (msg->first_line.type!=SIP_REQUEST) {
		LOG(L_ERR,"ERR:"M_NAME":Gw_MD5_to_AKA: The message is not a request\n");
		goto error;
	}
	if (msg->first_line.u.request.method.len!=8||
		memcmp(msg->first_line.u.request.method.s,"REGISTER",8)!=0)
	{
		LOG(L_ERR,"ERR:"M_NAME":Gw_MD5_to_AKA: The method is not a REGISTER\n");
		goto error;		
	}

	/* check if it is an authorized REGISTER */
	if (str1&&strlen(str1)>=1&&str1[0]=='1')
		is_authorized=1;
	/* translate Authorization header */
	auth = sip2ims_get_authorization(msg,&hdr);
	new_auth = translate_authorization(msg,auth,is_authorized);
	
	if (sip2ims_add_header(msg,&new_auth,HDR_AUTHORIZATION_T)){
		if (!sip2ims_del_header(msg,hdr)){
			LOG(L_INFO,"INF:"M_NAME":Gw_MD5_to_AKA: Error dropping old authorization header.\n");		
			goto error;
		}
	} else goto error;

	
	LOG(L_INFO, "INF:"M_NAME":Gw_MD5_to_AKA: \n===\n%.*s\n---\n%.*s===\n",
			auth.len,auth.s,new_auth.len,new_auth.s);
				
	return CSCF_RETURN_TRUE;		
error:
	if (new_auth.s) pkg_free(new_auth.s);
	return CSCF_RETURN_FALSE;	
} 


/**
 * Convert the challenge from AKA to MD5
 * @param msg - SIP message to apply to
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not
 */
int Gw_AKA_to_MD5(struct sip_msg *msg,char *str1,char *str2)
{		
	str auth={0,0};
	str new_auth={0,0};
	struct hdr_field *hdr;
	int code = msg->first_line.u.reply.statuscode;

	if (!msg) return CSCF_RETURN_FALSE;
	
	LOG(L_INFO,"DBG:"M_NAME":Gw_AKA_to_MD5: %d\n",code);
		
	switch(code){
		case 401:
			/* translate Authorization header */
			auth = sip2ims_get_authenticate(msg,&hdr);
			new_auth = translate_challenge(msg,auth);
			
			if (sip2ims_add_header(msg,&new_auth,HDR_AUTHORIZATION_T)){
				if (!sip2ims_del_header(msg,hdr)){
					LOG(L_INFO,"INF:"M_NAME":Gw_AKA_to_MD5: Error dropping old authorization header.\n");		
				}
				LOG(L_INFO, "INF:"M_NAME":Gw_AKA_to_MD5: \n+++\n%.*s\n---\n%.*s+++\n",
					auth.len,auth.s,new_auth.len,new_auth.s);				
			}else{
				LOG(L_ERR, "ERR:"M_NAME":Gw_AKA_to_MD5: Error adding new header\n");				
				if (new_auth.s) pkg_free(new_auth.s);
			}
			break;
		default:
			return CSCF_RETURN_FALSE;
	}	
	return CSCF_RETURN_TRUE;
}
