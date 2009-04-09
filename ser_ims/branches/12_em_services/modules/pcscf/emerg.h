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
 
#ifndef P_CSCF_EMERG_H
#define P_CSCF_EMERG_H 

#include <libxml/parser.h>
#include "../../sr_module.h"

#define ANONYMOUS_DOMAIN_STR		"anonymous.invalid"
#define ANONYMOUS_DOMAIN_STR_LEN	(sizeof(ANONYMOUS_DOMAIN_STR)-1)



//emergency URNs from the RFC 5031 "A Uniform Resource Name (URN) for Emergency and Other Well-Known Services"
#define SOS_URN   		"urn:service:sos"
#define SOS_URN_LEN		(sizeof(SOS_URN)-1)

#define SOS_URN_AMB		"ambulance"
#define SOS_URN_AMB_LEN		(sizeof(SOS_URN_AMB)-1)
#define SOS_URN_AN_CTR		"animal-control"
#define SOS_URN_AN_CTR_LEN	(sizeof(SOS_URN_AN_CTR)-1)

#define SOS_URN_FIRE		"fire"
#define SOS_URN_FIRE_LEN	(sizeof(SOS_URN_FIRE)-1)

#define SOS_URN_GAS		"gas"
#define SOS_URN_GAS_LEN		(sizeof(SOS_URN_GAS)-1)

#define SOS_URN_MAR		"marine"
#define SOS_URN_MAR_LEN		(sizeof(SOS_URN_MAR)-1)
#define SOS_URN_MOUNT		"mountain"
#define SOS_URN_MOUNT_LEN	(sizeof(SOS_URN_MOUNT)-1)

#define SOS_URN_POL		"police"
#define SOS_URN_POL_LEN		(sizeof(SOS_URN_POL)-1)
#define SOS_URN_PHYS		"physician"
#define SOS_URN_PHYS_LEN	(sizeof(SOS_URN_PHYS)-1)
#define SOS_URN_POIS		"poison"
#define SOS_URN_POIS_LEN	(sizeof(SOS_URN_POIS)-1)

//name of the nodes from the XML body of an 380 Alternative Service reply, TS 24.229
#define IMS_3GPP_XML_NODE	"ims-3gpp"
#define IMS_3GPP_XML_VS		"1.0"
#define IMS_3GPP_XML_ENC	"UTF-8"
#define ALTERN_SERV_XML_NODE	"alternative-service"
#define TYPE_XML_NODE		"type"
#define ALT_SERV_TYPE_VAL	"emergency"
#define ACTION_XML_NODE		"action"
#define ALT_SERV_ACTION_VAL	"emergency-registration"
#define REASON_XML_NODE		"reason"

int init_emergency_cntxt();

int P_is_anonymous_identity(struct sip_msg *msg,char *str1,char *str2);

int P_emergency_flag(struct sip_msg *msg,char *str1,char *str2);

int P_emergency_ruri(struct sip_msg *msg, char* str1, char* str2);

int P_enforce_sos_routes(struct sip_msg *msg, char* str1, char* str2);

int P_emergency_serv_enabled(struct sip_msg *msg, char* str1, char* str2);

int P_380_em_alternative_serv(struct sip_msg *msg, char* str1, char* str2);

int fixup_380_alt_serv(void** param, int param_no);

#endif
