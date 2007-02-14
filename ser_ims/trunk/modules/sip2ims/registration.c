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
 * SIP-to-IMS Gateway - Authorization translation functionality
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
 
#include <time.h>
 
#include "registration.h"

#include "../../data_lump.h"
#include "../../mem/mem.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../../parser/msg_parser.h"
#include "../../parser/hf.h"
#include "mod.h"
#include "sip.h"
#include "auth_api.h"
#include "DigestAKAv1MD5.h"
#include "db.h"


str authorization_s={"Authorization: ",15};
str authorization_e={"\r\n",2};
str authenticate_s={"WWW-Authenticate: ",18};
str authenticate_e={"\r\n",2};
str digest={"Digest",6};
str username_s={" username=\"",11};
str username_e={"\",",2};
str realm_s={" realm=\"",8};
str realm_e={"\",",2};
str uri_s={" uri=\"",6};
str uri_e={"\",",2};
str opaque_s={" opaque=\"",9};
str opaque_e={"\",",2};
str nonce_s={" nonce=\"",8};
str nonce_e={"\",",2};
str response_s={" response=\"",11};
str response_e={"\",",2};

str algorithm_s={" algorithm=\"",12};
str algorithm_e={"\",",2};
str akav1={"AKAv1-MD5",9};
str akav2={"AKAv2-MD5",9};

/**
 * Translates the Authorization header.
 * 
 * @param msg - the SIP message to which it belongs
 * @param old_auth - old Authorization header contents
 * @param is_authorized - whether to produce a correct authorization or not
 * @returns the new authorization header
 */
str translate_authorization(struct sip_msg *msg,str old_auth,int is_authorized)
{
	int i;
	str x={0,0};
	str username={0,0},userpart={0,0};
	str realm={0,0};
	str opaque={0,0};
	str nonce={0,0};
	str uri={0,0};
	str algorithm={0,0};int version=0;
	str response={0,0};
	str responseMD5={0,0};
	str k={0,0};

	x.len = authorization_s.len + authorization_e.len;
	if (!old_auth.len){
		/* No authorization header present */
		x.len+=digest.len;
		/*username*/
		username = sip2ims_get_public_identity(msg);
		if (username.len>4&&strncasecmp(username.s,"sip:",4)==0){
			username.s+=4;
			username.len-=4;
		}
		x.len+=username_s.len+username.len+username_e.len;
		/*realm*/
		if (username.s)
		for(i=0;i<username.len;i++)
			if (username.s[i]=='@'){
				if (i>=username.len-1) break;
				i++;
				realm.s = username.s+i;
				realm.len=0;
				while(i+realm.len<username.len && 
					realm.s[realm.len]!=':' &&
					realm.s[realm.len]!=';')
						realm.len++;
			}
		x.len+=realm_s.len+realm.len+realm_e.len;
	}else{
		/* Authorization header is present */
		/* Compute the new response */
		
		x.len+=digest.len;
		/*username*/
		username = sip2ims_get_public_identity(msg);
		if (username.len>4&&strncasecmp(username.s,"sip:",4)==0){
			username.s+=4;
			username.len-=4;
		}
		x.len+=username_s.len+username.len+username_e.len;
		/*realm*/
		realm = sip2ims_get_realm(msg);
		x.len+=realm_s.len+realm.len+realm_e.len;
		/*nonce*/
		opaque = sip2ims_get_opaque(msg);
		nonce = opaque;
		algorithm = akav1;
		for(i=0;i<opaque.len;i++)
			if (opaque.s[i]=='|'){
				algorithm.s = opaque.s;
				algorithm.len = i;
				nonce.s = opaque.s+i+1;
				nonce.len = opaque.len-i-1;
			}
		x.len+=nonce_s.len+nonce.len+nonce_e.len;
		/*uri*/
		uri = sip2ims_get_digest_uri(msg,realm);
		x.len+=uri_s.len+uri.len+uri_e.len;
		/* algorithm */			
		x.len+=algorithm_s.len+algorithm.len+algorithm_e.len;
		if (algorithm.len==akav1.len && strncasecmp(algorithm.s,akav1.s,akav1.len)==0){
			version = 1;
		}else if (algorithm.len==akav2.len && strncasecmp(algorithm.s,akav2.s,akav2.len)==0){
			version = 2;
		}else
			version = 0;			
		/* response AKA */
		if (is_authorized){
			userpart=username;
			userpart.len=0;
			while(userpart.len<username.len && userpart.s[userpart.len]!='@')
				userpart.len++;
			sip2ims_db_get_password(userpart,realm,&k);
			response=AKA(version,nonce,k);
			pkg_free(k.s);
			k.s=0;k.len=0;
			if (response.s){
				/* response AKA MD5 */
				responseMD5 = MD5(msg,response,username,realm,nonce,uri);
				pkg_free(response.s);				
			}		
		}else{
			responseMD5.s=0;responseMD5.len=0;
		}
		x.len+=response_s.len+responseMD5.len+response_e.len;
	}
	
	x.s = pkg_malloc(x.len);
	if (!x.s){
		LOG(L_ERR,"ERR:"M_NAME":translate_authorization: Error allocating %d bytes\n",
			x.len);
		x.len = 0;
		return x;
	}
	x.len = 0;
	STR_APPEND(x,authorization_s);
	
	if (!old_auth.len){
		/* No authorization header present */
		STR_APPEND(x,digest);
		STR_APPEND(x,username_s);
		STR_APPEND(x,username);
		STR_APPEND(x,username_e);
		STR_APPEND(x,realm_s);
		STR_APPEND(x,realm);
		STR_APPEND(x,realm_e);
	}else{
		/* Authorization header is present */
		STR_APPEND(x,digest);
		STR_APPEND(x,username_s);
		STR_APPEND(x,username);
		STR_APPEND(x,username_e);
		STR_APPEND(x,realm_s);
		STR_APPEND(x,realm);
		STR_APPEND(x,realm_e);
		STR_APPEND(x,nonce_s);
		STR_APPEND(x,nonce);
		STR_APPEND(x,nonce_e);
		STR_APPEND(x,uri_s);
		STR_APPEND(x,uri);
		STR_APPEND(x,uri_e);
		STR_APPEND(x,response_s);
		STR_APPEND(x,responseMD5);
		STR_APPEND(x,response_e);
		STR_APPEND(x,algorithm_s);
		STR_APPEND(x,algorithm);
		STR_APPEND(x,algorithm_e);
	}

	STR_APPEND(x,authorization_e);
		
	if (responseMD5.s) pkg_free(responseMD5.s);	
	return x;
}

/**
 * Translates the WWW-Authenticate header.Containing a challenge
 * 
 * @param msg - the SIP message to which it belongs
 * @param old_auth - old Authorization header contents
 * @returns the new authorization header
 */
str translate_challenge(struct sip_msg *msg,str old_auth)
{
	int i;
	str x={0,0};
	str username={0,0};
	str realm={0,0};
	str opaque={0,0};
	str challenge={0,0};
	str algorithm={0,0};

	x.len = authenticate_s.len + authenticate_s.len;
	if (!old_auth.len){
		LOG(L_ERR,"INFO:"M_NAME":translate_challenge: Strange... there was no challenge in the 401.\n");
		return x;
	}
	x.len+=digest.len;
	/*username*/
	username = sip2ims_get_public_identity(msg);
	if (username.len>4&&strncasecmp(username.s,"sip:",4)==0){
		username.s+=4;
		username.len-=4;
	}
	x.len+=username_s.len+username.len+username_e.len;
	/*realm*/
	if (username.s)
	for(i=0;i<username.len;i++)
		if (username.s[i]=='@'){
			if (i>=username.len-1) break;
			i++;
			realm.s = username.s+i;
			realm.len=0;
			while(i+realm.len<username.len && 
				realm.s[realm.len]!=':' &&
				realm.s[realm.len]!=';')
					realm.len++;
		}
	x.len+=realm_s.len+realm.len+realm_e.len;

	/*algorithm*/
	algorithm = sip2ims_get_algorithm(msg,realm);
	/*opaque = AKAalg||AKAnonce*/
	opaque = sip2ims_get_nonce(msg,realm);
	x.len+=opaque_s.len+algorithm.len+1+opaque.len+opaque_e.len;

	/* the new MD5 challenge */	
	challenge.s = build_auth_hf(0,0,&realm,&(challenge.len),0);	
	LOG(L_DBG,"DBG:"M_NAME":translate_challenge: MD5<%.*s>\n",
		challenge.len,challenge.s);
	x.len+=challenge.len;	
		
		
	x.s = pkg_malloc(x.len);
	if (!x.s){
		LOG(L_ERR,"ERR:"M_NAME":translate_challenge: Error allocating %d bytes\n",
			x.len);
		x.len = 0;
		goto done;
	}
	x.len = 0;
	STR_APPEND(x,authenticate_s);
	
	STR_APPEND(x,digest);
	STR_APPEND(x,username_s);
	STR_APPEND(x,username);
	STR_APPEND(x,username_e);
	STR_APPEND(x,realm_s);
	STR_APPEND(x,realm);
	STR_APPEND(x,realm_e);
	STR_APPEND(x,opaque_s);
	STR_APPEND(x,algorithm);
	x.s[x.len++]='|';
	STR_APPEND(x,opaque);
	STR_APPEND(x,opaque_e);
	STR_APPEND(x,challenge);

	STR_APPEND(x,authenticate_e);
		
done:
	if (challenge.s) pkg_free(challenge.s);		
	return x;
}


