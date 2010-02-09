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
 
/**
 * \file
 *
 * P-CSCF Policy and Charging Control interfaces AVPs
 * 
 * \author Alberto Diez Albaladejo -at- fokus dot fraunhofer dot de
 */ 
 
#ifndef __PCC_AVP_H
#define __PCC_AVP_H

#include "../../sr_module.h"
#include "mod.h"
#include "../cdp/cdp_load.h"
#include "sdp_util.h"



#define PCC_MAX_Char 64
#define PCC_MAX_Char4 256
/* Maximum Number of characters to represent some AVP datas*/
/*and ipv6 addresses in character*/
#define PCC_Media_Sub_Components 10

#include <string.h>
#include <stdio.h>


/** NO DATA WILL BE DUPLICATED OR FREED - DO THAT AFTER SENDING THE MESSAGE!!! */

typedef struct _bandwidth {
		int bAS;
		int bRS;
		int bRR;		
} bandwidth;

enum ip_type {
	ip_type_v4 	= 1,
	ip_type_v6	= 2
};

/*helper*/
int pcc_get_ip_port(struct sip_msg *r, struct sip_uri * parsed_aor, str *ip, unsigned short * port);
AAA_AVP* pcc_create_framed_ip_address(str ip);
/*just headers*/

int PCC_add_destination_realm(AAAMessage *msg, str data);
int PCC_add_autheapplication_id(AAAMessage *msg, unsigned int data);
inline int PCC_add_subscription_ID(AAAMessage *msg,struct sip_msg *r,int tag);
AAA_AVP *PCC_create_media_subcomponent(int number, char *proto, 
					str ipA, unsigned int intportA, 
					str ipB, unsigned int intportB ,
					char *options,int atributes);
inline int PCC_create_add_media_subcomp_dialog(AAA_AVP_LIST *list,str sdpA,
											str sdpB,int number,AAA_AVP **media_sub_component,int tag);
int PCC_add_media_component_description_for_register(AAAMessage *msg, struct sip_uri * parsed_aor);
inline int PCC_add_media_component_description(AAAMessage *msg,str sdpinvite,str sdp200,char *mline,int number,int tag);
AAA_AVP* PCC_create_codec_data(str sdp,int number,int direction);

int extract_mclines(str sdpA,str sdpB,char **mlineA,char **clineA,char **mlineB,char **clineB,int number);
int extract_token(char *line, str *token,int max,int number);
int extract_bandwidth(bandwidth *bw,str sdp,char *start);
int extract_id(struct sip_msg *r,int tag,str *identification);
int check_atributes(str sdpbody,char *mline);
int is_a_port(str port);
/*int is_an_address(char *ad);*/
inline int PCC_add_auth_application_id(AAAMessage *msg, unsigned int data);
inline int PCC_get_result_code(AAAMessage *msg, unsigned int *data);
#endif /*__PCC_AVP_H*/
