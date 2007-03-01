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
 * \dir modules/sip2ims
 *
 * This is the SIP-to-IMS Gateway module. For general documentation, look at \ref SIP2IMS
 * 
 */
 
 /**
  *  \file modules/sip2ims/Makefile 
  * SIP-to-IMS Gateway SER module Makefile
  * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
  */
 
/** 
 * \page SIP2IMS The SIP-to-IMS Gateway Module (sip2ims)
 * \b Module \b Documentation
 *
 * [\subpage sip2ims_overview]
 * [\subpage sip2ims_code]
 * [\subpage sip2ims_db]
 * [\subpage sip2ims_config]
 * [\subpage sip2ims_example]
 *
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 * \todo Save the Service-Route header and add it to all initial requests
 * 
 * \todo Provide NATHelper for SIP clients
 * 
 *
 *  
 * \section sip2ims_overview Overview
 * 
 * This module is supposed to provide the translation capabilities to enable
 * normal SIP User Endpoints to use the Open IMS Core through the Proxy-CSCF.
 * 
 * For now, it only performs Authentication translation between:
 * - Digest-MD5 <-> Digest-AKAv1-MD5 
 * - Digest-MD5 <-> Digest-AKAv2-MD5 
 * - Digest-MD5 <-> Digest-MD5 
 * - and not only. Theoretically any authentication method that can be enabled 
 * from the SER script can be translated to AKAv1, AKAv2 or MD5 as the two 
 * parts are decoupled in the script.
 * 
 * No syncrhonization of SQN is performed.
 * 
 * It has to be noted that this is merely a hack than a full solution. Because a 
 * Proxy-CSCF expects to be directly connected with an User Endpoint, there are a 
 * number of deficiencies that could only be solved by using a full B2BUA for all
 * messages. Unfortunately SER was not designed to act primarily as a UAC or B2BUA
 * and as such developing such a functionality with SER would be difficult.
 * 
 * For stateless purposes, the AKA challenge is transmitted towards the MD5 client in
 * the opaque field. When a challenge response comes back, the AKA response is 
 * computet based on this opaque field. There are several clients which do not return
 * the opaque (for example Ekiga 2.0.2 and older) and as such are unusable.  
 * 
 * Here you have a usage example where we employ the auth_db module to check the MD5
 * authorization:
 * \code
route[REGISTER]
{
    if (www_authorize("open-ims.test","credentials")) {
        if (!Gw_MD5_to_AKA("1")){
            sl_send_reply("500","Error translating MD5->AKA authorized REGISTER");
            exit;
        }
    }else{ 
        if (!Gw_MD5_to_AKA("0")){
            sl_send_reply("500","Error translating MD5->AKA unauthorized REGISTER");
            exit;
        }               
    }
    
    t_on_reply("REGISTER_reply");
    #t_on_failure("REGISTER_failure");
    route(PCSCF);       
    exit;
}

onreply_route[REGISTER_reply]
{
    if (t_check_status("401")){
        if (!Gw_AKA_to_MD5()){
            log(1,"Error translating from AKA to MD5");
            #sl_send_reply("500","Error translating AKA->MD5 challenge");
            break;
        }   
    } 
   \endcode 
 * 
 * 
 * 
 * \section sip2ims_code Code Structure
 *
 * The AKA authentication is implemented into DigestAKAv1MD5.c with the Milenage 
 * and Rijndael functionality in milenage.c and rijndael.c. auth_api.c and rfc2617.c 
 * are taken from SER modules auth and auth_db and they enable the MD5 in AKA.
 * The registration.c translates between authentication and gm.c packages this into
 * headers. Also included are base64.c translations and sip.c for SIP operations. The
 * module interfaces are in mod.c. 
 * 
 * \section sip2ims_db Database Prerequisites
 * 
 * There are several tables that need to be provisioned in a database, in order for the
 * SIP2IMS gateway to function properly. In a nutshell, we're using the SER credentials
 * table. You can create it with the scripts provided with SER, or here you have a MySQL
 * dump as example:
 * \include sip2ims.sql
 * 
 * 
 * \section sip2ims_config Configuration and Usage
 * 
 * For exported functions look at #sip2ims_cmds.\n
 * For configuration parameters look at #sip2ims_params.\n 
 * 
 *  
 * \section sip2ims_example Example
 * And here is a real usage example:
 * - SIP-to-IMS Gateway configuration file \include sip2ims.cfg
 * 
 * 
 */
 
/**
 * \file
 * 
 * SIP-to-IMS Gateway - SER module interface
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#ifndef SIP2IMS_MOD_H
#define SIP2IMS_MOD_H

#define M_NAME "SIP2IMS"

/** Return and break the execution of routng script */
#define CSCF_RETURN_BREAK	0 
/** Return true in the routing script */
#define CSCF_RETURN_TRUE	1
/** Return false in the routing script */
#define CSCF_RETURN_FALSE -1
/** Return error in the routing script */
#define CSCF_RETURN_ERROR -2



#define STR_SHM_DUP(dest,src,txt)\
{\
	(dest).s = shm_malloc((src).len);\
	if (!(dest).s){\
		LOG(L_ERR,"ERR:"M_NAME":"txt": Error allocating %d bytes\n",(src).len);\
	}else{\
		(dest).len = (src).len;\
		memcpy((dest).s,(src).s,(src).len);\
	}\
}


#define STR_APPEND(dst,src)\
	{memcpy((dst).s+(dst).len,(src).s,(src).len);\
	(dst).len = (dst).len + (src).len;}



#endif /* SIP2IMS_MOD_H */
