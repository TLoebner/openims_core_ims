/**
 * $Id$
 *   
 * Copyright (C) 2009-2010 FhG Fokus
 *
 * This file is part of Open EPC - an implementation of EPS core components
 * 
 * The Open EPC project  is a prototype implementation of the 3GPP 
 * Release 8 and later Evolved Packet Core (EPC) that will allow academic and 
 * industrial researchers and engineers around the world to obtain a 
 * practical look and feel of the capabilities of the Evolved Packet Core.
 * For more information on the Open EPC project, please visit www.openepc.net
 * or please contact Fraunhofer FOKUS by e-mail at the following addresses:
 *     info@openepc.net
 *
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
 * CDiameter AVP Operations modules _ EPC APP 
 * 
 * 
 *  \author Dragos Vingarzan dragos dot vingarzan _at_ fokus dot fraunhofer dot de
 * 
 */ 

/*
 * TS 29.061  (partly)
 * http://www.3gpp.org/ftp/Specs/html-info/29061.htm

 * TS 29.212  Gx/Gxx
 * http://www.3gpp.org/ftp/Specs/html-info/29212.htm
 * 
 * TS 29.214  Rx
 * http://www.3gpp.org/ftp/Specs/html-info/29214.htm
 * 
 * TS 29.272  
 * http://www.3gpp.org/ftp/Specs/html-info/29272.htm
 *
 */

#include "macros.h"

#undef CDP_AVP_MODULE
#define CDP_AVP_MODULE epcapp

#if !defined(CDP_AVP_DECLARATION) && !defined(CDP_AVP_EXPORT) && !defined(CDP_AVP_INIT) && !defined(CDP_AVP_REFERENCE)
	#ifndef _CDP_AVP_EPCAPP_H_1
	#define _CDP_AVP_EPCAPP_H_1

		#include "../cdp/cdp_load.h"

	#else

		/* undo the macros definition if this was re_included */
		#define CDP_AVP_EMPTY_MACROS
			#include "macros.h"
		#undef CDP_AVP_EMPTY_MACROS

	#endif
#endif //_CDP_AVP_NASAPP_H_1	

/*
 * The list of AVPs must be declared in the following format:
 * 
 * 		cdp_avp_add(<avp_name>.<vendor_id>,<flags>,<avp_type>,<data_type>)
 * 		or
 * 		cdp_avp_add_ptr(<avp_name>.<vendor_id>,<flags>,<avp_type>,<data_type>)
 * 
 * 		cdp_avp_get(<avp_name>.<vendor_id>,<avp_type>,<data_type>)
 * 
 * or, to add both add and get at once:
 * 
 * 		cdp_avp(<avp_name>.<vendor_id>,<flags>,<avp_type>,<data_type>)
 * 		or 
 * 		cdp_avp_ptr(<avp_name>.<vendor_id>,<flags>,<avp_type>,<data_type>)
 * 
 * The add macros ending in _ptr will generate function with the extra AVPDataStatus data_do parameter
 * 
 * Parameters:
 *  - avp_name - a value of AVP_<avp_name> must resolve to the AVP code
 *  - vendor_id - an int value
 *  - flags	- AVP Flags to add to the AVP
 *  - avp_type - an avp type for which a function was defined a
 * 				int cdp_avp_get_<avp_type>(AAA_AVP *avp,<data_type> *data)
 * 		Some valid suggestions (and the data_type):		
 *  
 *  			OctetString 	_ str
 *  			Integer32		_ int32_t
 *  			Integer64 		_ int64_t
 *  			Unsigned32 		_ uint32_t
 *  			Unsigned64 		_ uint64_t
 *  			Float32 		_ float
 *  			Float64 		_ double
 *  			Grouped 		_ AAA_AVP_LIST
 *  
 *  			Address 		_ ip_address
 *  			Time 			_ time_t
 *  			UTF8String 		_ str
 *  			DiameterIdentity_ str
 *  			DiameterURI		_ str
 *  			Enumerated		_ int32_t
 *  			IPFilterRule	_ str
 *  			QoSFilterRule	_ str
 *  - data_type - the respective data type for the avp_type defined above
 *  
 *  The functions generated will return 1 on success or 0 on error or not found
 *  The prototype of the function will be:
 *  
 *  	int cdp_avp_get_<avp_name_group>(AAA_AVP_LIST list,<data_type> *data,AAA_AVP **avp_ptr)
 * 
 * 
 *  
 *  For Grouped AVPs with 2 or 3 known inside AVPs, you can define a shortcut function which will find the group and
 *  also extract the 2 or 3 AVPs. 
 *  Do not define both 2 and 3 for the same type!
 * 
 * 
 *		cdp_avp_add2(<avp_name_group>.<vendor_id_group>,<flags_group>,<avp_name_1>,<data_type_1>,<avp_name_2>,<data_type_2>)
 * 		cdp_avp_get2(<avp_name_group>.<vendor_id_group>,<avp_name_1>,<data_type_1>,<avp_name_2>,<data_type_2>)
 *  	
 *		cdp_avp_get3(<avp_name_group>.<vendor_id_group>,<flags_group>,<avp_name_1>,<data_type_1>,<avp_name_2>,<data_type_2>,<avp_name_3>,<data_type_3>)
 *  	cdp_avp_get3(<avp_name_group>.<vendor_id_group>,<avp_name_1>,<data_type_1>,<avp_name_2>,<data_type_2>,<avp_name_3>,<data_type_3>)
 * 
 * 	 or, to add both add and get at once:
 * 
 *		cdp_avp2(<avp_name_group>.<vendor_id_group>,<flags_group>,<avp_name_1>,<data_type_1>,<avp_name_2>,<data_type_2>)
 * 		cdp_avp3(<avp_name_group>.<vendor_id_group>,<flags_group>,<avp_name_1>,<data_type_1>,<avp_name_2>,<data_type_2>)
 *  
 *  - avp_name_group - a value of AVP_<avp_name_group> must resolve to the AVP code of the group
 *  
 *  - vendor_id_group - an int value
 *  
 *  - avp_name_N	- the name of the Nth parameter. 
 *  	Previously, a cdp_avp_get(<avp_name_N>,<vendor_id_N>,<avp_type_N>,<data_type_N>) must be defined!
 *  
 *  - data_type_N	- the respective data type for avp_type_N (same as <data_type_N) 
 *  
 *  The functions generated will return the number of found AVPs inside on success or 0 on error or not found
 *  The prototype of the function will be:
 *  
 *  	int cdp_avp_get_<avp_name_group>_Group(AAA_AVP_LIST list,<data_type_1> *avp_name_1,<data_type_2> *avp_name_2[,<data_type_3> *avp_name_3],AAA_AVP **avp_ptr)
 *  
 *  Note - generally, all data of type str will need to be defined with ..._ptr
 *  Note - Groups must be defined with:
 *  	 cdp_avp_add_ptr(...) and data_type AAA_AVP_LIST*
 *  	 cdp_avp_get(...) and data_type AAA_AVP_LIST 	
 */
#undef CDP_AVP_NAME
#define CDP_AVP_NAME(avp_name) AVP_EPC_##avp_name

/*
 * TS 29.212
 */

cdp_avp_add_ptr	(Access_Network_Charging_Identifier_Gx,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Access_Network_Charging_Identifier_Gx,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp_add_ptr	(Allocation_Retention_Priority,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Allocation_Retention_Priority,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp			(AN_GW_Address,							EPC_vendor_id_3GPP,	0,							Address,		ip_address)	

cdp_avp			(APN_Aggregate_Max_Bitrate_DL,			EPC_vendor_id_3GPP,	0,							Unsigned32,		uint32_t)

cdp_avp			(APN_Aggregate_Max_Bitrate_UL,			EPC_vendor_id_3GPP,	0,							Unsigned32,		uint32_t)

cdp_avp			(Bearer_Control_Mode,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_ptr		(Bearer_Identifier,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(Bearer_Operation,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Bearer_Usage,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_add_ptr	(Charging_Rule_Install,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Charging_Rule_Install,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp_add_ptr	(Charging_Rule_Remove,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Charging_Rule_Remove,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp_add_ptr	(Charging_Rule_Definition,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Charging_Rule_Definition,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp_ptr		(Charging_Rule_Base_Name,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		UTF8String,		str)	

cdp_avp_ptr		(Charging_Rule_Name,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_add_ptr	(Charging_Rule_Report,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Charging_Rule_Report,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp			(CoA_IP_Address,						EPC_vendor_id_3GPP,	0,							Address,		ip_address)	

cdp_avp_add_ptr	(CoA_Information,						EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(CoA_Information,						EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST)	

cdp_avp_add_ptr	(Default_EPS_Bearer_QoS,				EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Default_EPS_Bearer_QoS,				EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST)	

cdp_avp_add_ptr	(Event_Report_Indication,				EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Event_Report_Indication,				EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST)	

cdp_avp			(Event_Trigger,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_add_ptr	(Flow_Information,						EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Flow_Information,						EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST)	

cdp_avp_ptr		(Flow_Label,							EPC_vendor_id_3GPP,	0,							OctetString,	str)	

cdp_avp			(IP_CAN_Type,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Guaranteed_Bitrate_DL,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Guaranteed_Bitrate_UL,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Metering_Method,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Network_Request_Support,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Offline,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Online,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_ptr		(Packet_Filter_Content,					EPC_vendor_id_3GPP,	0,							IPFilterRule,	str)	

cdp_avp_ptr		(Packet_Filter_Identifier,				EPC_vendor_id_3GPP,	0,							OctetString,	str)	

cdp_avp_add_ptr	(Packet_Filter_Information,				EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Packet_Filter_Information,				EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST)	

cdp_avp			(Packet_Filter_Operation,				EPC_vendor_id_3GPP,	0,							Enumerated,		int32_t)	

cdp_avp_ptr		(PDN_Connection_ID,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(Precedence,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Pre_emption_Capability,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Pre_emption_Vulnerability,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Priority_Level,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Reporting_Level,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(PCC_Rule_Status,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Session_Release_Cause,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(QoS_Class_Identifier,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_add_ptr	(QoS_Information,						EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(QoS_Information,						EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST)	

cdp_avp			(QoS_Negotiation,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_add_ptr	(QoS_Rule_Install,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(QoS_Rule_Install,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp_add_ptr	(QoS_Rule_Remove,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(QoS_Rule_Remove,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp_add_ptr	(QoS_Rule_Definition,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(QoS_Rule_Definition,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp_ptr		(QoS_Rule_Name,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_add_ptr	(QoS_Rule_Report,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(QoS_Rule_Report,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	


cdp_avp			(QoS_Upgrade,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Resource_Allocation_Notification,		EPC_vendor_id_3GPP,	0,							Enumerated,		int32_t)	

cdp_avp			(Rule_Failure_Code,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_ptr		(Security_Parameter_Index,				EPC_vendor_id_3GPP,	0,							OctetString,	str)	

cdp_avp_ptr		(TFT_Filter,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		IPFilterRule,	str)	

cdp_avp_add_ptr	(TFT_Packet_Filter_Information,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(TFT_Packet_Filter_Information,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)	

cdp_avp_ptr		(ToS_Traffic_Class,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(Tunnel_Header_Filter,					EPC_vendor_id_3GPP,	0,							IPFilterRule,	str)	

cdp_avp			(Tunnel_Header_Length,					EPC_vendor_id_3GPP,	0,							Unsigned32,		uint32_t)

cdp_avp_add_ptr	(Tunnel_Information,					EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Tunnel_Information,					EPC_vendor_id_3GPP,	0,							Grouped,		AAA_AVP_LIST)	

cdp_avp			(RAT_Type,								EPC_vendor_id_3GPP,	0,							Enumerated,		int32_t)	

cdp_avp			(Revalidation_Time,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Time,			time_t)	

cdp_avp			(Rule_Activation_Time,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Time,			time_t)	

cdp_avp			(Rule_DeActivation_Time,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Time,			time_t)	

cdp_avp			(Session_Linking_Indicator,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

/*
 * TS 29.061 Gi/SGi
 */

cdp_avp_ptr		(3GPP_SGSN_Address,						EPC_vendor_id_3GPP,	0,							OctetString,	str)	

cdp_avp_ptr		(3GPP_SGSN_IPv6_Address,				EPC_vendor_id_3GPP,	0,							OctetString,	str)	

cdp_avp_ptr		(3GPP_SGSN_MCC_MNC,						EPC_vendor_id_3GPP,	0,							UTF8String,		str)	

cdp_avp_ptr		(3GPP_User_Location_Info,				EPC_vendor_id_3GPP,	0,							OctetString,	str)	

cdp_avp_ptr		(RAI,									EPC_vendor_id_3GPP,	0,							UTF8String,		str)	

cdp_avp_ptr		(3GPP_MS_TimeZone,						EPC_vendor_id_3GPP,	0,							OctetString,	str)	

/*
 * TS 29.214 Rx
 */

cdp_avp			(Abort_Cause,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Access_Network_Charging_Address,		EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Address,		ip_address)	

cdp_avp_add_ptr	(Access_Network_Charging_Identifier,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Access_Network_Charging_Identifier,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(Access_Network_Charging_Identifier_Value,EPC_vendor_id_3GPP,AAA_AVP_FLAG_MANDATORY,	OctetString,	str)	

cdp_avp_add_ptr	(Acceptable_Service_Info,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Acceptable_Service_Info,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(AF_Application_Identifier,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(AF_Charging_Identifier,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(Codec_Data,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(Flow_Description,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		IPFilterRule,	str)	

cdp_avp			(Flow_Number,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp_add_ptr	(Flows,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Flows,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp			(Flow_Status,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Flow_Usage,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_ptr		(Service_URN,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(Specific_Action,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Max_Requested_Bandwidth_DL,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Max_Requested_Bandwidth_UL,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp_add_ptr	(Media_Component_Description,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Media_Component_Description,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp			(Media_Component_Number,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp_add_ptr	(Media_Sub_Component,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Media_Sub_Component,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp			(Media_Type,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(RR_Bandwidth,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(RS_Bandwidth,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Service_Info_Status,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(SIP_Forking_Indication,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	


/*
 * TS 29.272  
 * http://www.3gpp.org/ftp/Specs/html-info/29272.htm
 */

cdp_avp_add_ptr	(Subscription_Data,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Subscription_Data,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_add_ptr	(Terminal_Information,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Terminal_Information,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(IMEI,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		UTF8String,		str)	

cdp_avp_ptr		(Software_Version,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		UTF8String,		str)	

cdp_avp_ptr		(QoS_Subscribed,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(ULR_Flags,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(ULA_Flags,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp_ptr		(Visited_PLMN_Id,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_add_ptr	(Requested_EUTRAN_Authentication_Info,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Requested_EUTRAN_Authentication_Info,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_add_ptr	(Requested_UTRAN_GERAN_Authentication_Info,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,	Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Requested_UTRAN_GERAN_Authentication_Info,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,	Grouped,		AAA_AVP_LIST)

cdp_avp			(Number_Of_Requested_Vectors,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp_ptr		(Re_Synchronization_Info,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(Immediate_Response_Preferred,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp_add_ptr	(Authentication_Info,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Authentication_Info,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_add_ptr	(E_UTRAN_Vector,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(E_UTRAN_Vector,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_add_ptr	(UTRAN_Vector,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(UTRAN_Vector,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_add_ptr	(GERAN_Vector,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(GERAN_Vector,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp			(Network_Access_Mode,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(HPLMN_ODB,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Item_Number,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Cancellation_Type,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(DSR_Flags,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(DSA_Flags,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Context_Identifier,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Subscriber_Status,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Operator_Determined_Barring,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(Access_Restriction_Data,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp_ptr		(APN_OI_Replacement,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		UTF8String,		str)	

cdp_avp			(All_APN_Configurations_Included_Indicator,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,	Enumerated,		int32_t)	

cdp_avp_add_ptr	(APN_Configuration_Profile,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(APN_Configuration_Profile,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_add_ptr	(APN_Configuration,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(APN_Configuration,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_add_ptr	(EPS_Subscribed_QoS_Profile,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(EPS_Subscribed_QoS_Profile,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp			(VPLMN_Dynamic_Address_Allowed,			EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_ptr		(STN_SR,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(Alert_Reason,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_add_ptr	(AMBR,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_add2	(AMBR,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Max_Requested_Bandwidth_UL,	uint32_t,	Max_Requested_Bandwidth_DL,	uint32_t)	
cdp_avp_get		(AMBR,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)
cdp_avp_get2	(AMBR,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Max_Requested_Bandwidth_UL,	uint32_t,	Max_Requested_Bandwidth_DL,	uint32_t)	

cdp_avp_add_ptr	(CSG_Subscription_Data,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(CSG_Subscription_Data,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp			(CSG_Id,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(PDN_Gw_Allocation_Type,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Expiration_Date,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Time,			time_t)

cdp_avp			(RAT_Frequency_Selection_Priority_ID,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(IDA_Flags,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(PUA_Flags,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(NOR_Flags,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp_ptr		(User_Id,								EPC_vendor_id_3GPP,	0,							UTF8String,		str)	

cdp_avp			(Equipment_Status,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_ptr		(Regional_Subscription_Zone_Code,		EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(RAND,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(XRES,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(AUTN,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(KASME,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(Trace_Collection_Entity,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Address,		ip_address)

cdp_avp_ptr		(Kc,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(SRES,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(PDN_Type,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(Roaming_Restricted_Due_To_Unsupported_Feature,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,	Enumerated,	int32_t)	

cdp_avp_add_ptr	(Trace_Data,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Trace_Data,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(Trace_Reference,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(Trace_Depth,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_ptr		(Trace_NE_Type_List,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(Trace_Interface_List,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(Trace_Event_List,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(OMC_Id,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_add_ptr	(GPRS_Subscription_Data,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(GPRS_Subscription_Data,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp			(Complete_Data_List_Included_Indicator,	EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_add_ptr	(PDP_Context,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(PDP_Context,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(PDP_Type,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_ptr		(3GPP2_MEID,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_add_ptr	(Specific_APN_Info,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Specific_APN_Info,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_add_ptr	(LCS_Info,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(LCS_Info,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(GMLC_Number,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_add_ptr	(LCS_Privacy_Exception,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(LCS_Privacy_Exception,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(SS_Code,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_add_ptr	(SS_Status,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(SS_Status,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp			(Notification_To_UE_User,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_add_ptr	(External_Client,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(External_Client,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(Client_Identity,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(GMLC_Restriction,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp			(PLMN_Client,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Enumerated,		int32_t)	

cdp_avp_add_ptr	(Service_Type,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Service_Type,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp			(Sevice_Type_Identity,					EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp_add_ptr	(MO_LR,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(MO_LR,									EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_add_ptr	(Teleservice_List,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Teleservice_List,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(TS_Code,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_add_ptr	(Call_Barring_Infor_List,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(Call_Barring_Infor_List,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp_ptr		(SGSN_Number,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp			(IDR_Flags,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Unsigned32,		uint32_t)

cdp_avp			(ICS_Indicator,							EPC_vendor_id_3GPP,	0,							Enumerated,		int32_t)	

cdp_avp			(IMS_Voice_Over_PS_Sessions_Supported,	EPC_vendor_id_3GPP,	0,							Enumerated,		int32_t)	

cdp_avp			(Homogenous_Support_of_IMS_Over_PS_Sessions, EPC_vendor_id_3GPP, 0, 					Enumerated,		int32_t)	

cdp_avp			(Last_UE_Activity_Time,					EPC_vendor_id_3GPP,	0,							Time,			time_t)



cdp_avp			(PDN_Gw_Address,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Address,		ip_address)	


cdp_avp_ptr		(PDN_Gw_Name,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		OctetString,	str)	

cdp_avp_add_ptr	(PDN_Gw_Identity,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	
cdp_avp_get		(PDN_Gw_Identity,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)


/*
 * TS 29.173  
 * http://www.3gpp.org/ftp/Specs/html-info/29173.htm
 */
cdp_avp			(GMLC_Address,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Address,		ip_address)	



/*
 * TS 32.299  
 * http://www.3gpp.org/ftp/Specs/html-info/32299.htm
 */

cdp_avp			(Served_Party_IP_Address,				EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Address,		ip_address)	


/*
 *	http://tools.ietf.org/html/draft-ietf-dime-mip6-split-17
 */

cdp_avp_ptr		(Service_Selection,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		UTF8String,		str)	

cdp_avp_ptr		(QoS_Profile_Name,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		UTF8String,		str)	

/*
 * Generic Gateway related AVPs
 */

cdp_avp_add_ptr	(GG_Enforce,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST*)	

cdp_avp_get (GG_Enforce,						EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		Grouped,		AAA_AVP_LIST)

cdp_avp (GG_IP,								EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		
		Address,	ip_address)	

cdp_avp (UE_Locator,							EPC_vendor_id_3GPP,	AAA_AVP_FLAG_MANDATORY,		
		Address,	ip_address)	



/*
 * ETSI TS 183 017?
 * 
 */
#undef CDP_AVP_NAME
#define CDP_AVP_NAME(avp_name) AVP_ETSI_##avp_name


cdp_avp			(Reservation_Priority,						IMS_vendor_id_ETSI,	0,							Unsigned32,		uint32_t)


#undef CDP_AVP_NAME
#define CDP_AVP_NAME(avp_name) AVP_##avp_name


/*
 * From here-on you can define/export/init/declare functions which can not be generate with the macros
 */

#if defined(CDP_AVP_DEFINITION)

	/*
	 * Put here your supplimentary definitions. Typically:
	 * 
	 * int <function1>(param1)
	 * {
	 *   code1
	 * }
	 * 
	 * 
	 */


#elif defined(CDP_AVP_EXPORT)

	/*
	 * Put here your supplementary exports in the format: 
	 * 	<function_type1> <nice_function_name1>; 
	 *  <function_type2> <nice_function_name1>;
	 *  ...
	 *  
	 */


#elif defined(CDP_AVP_INIT)

	/*
	 * Put here your supplementary inits in the format: 
	 * 	<function1>,
	 *  <function2>,
	 *  ...
	 * 
	 * Make sure you keep the same order as in export!
	 * 
	 */


#elif defined(CDP_AVP_REFERENCE)
	/*
	 * Put here what you want to get in the reference. Typically:
	 * <function1>
	 * <function2>
	 * ... 
	 * 
	 */

	
#elif defined(CDP_AVP_EMPTY_MACROS)
	
	/* this should be left blank */
	
#else

	/*
	 * Put here your definitions according to the declarations, exports, init, etc above. Typically:
	 * 
	 * int <function1(params1);>
	 * typedef int <*function_type1>(params1);
	 * 
	 * int <function2(param2);>
	 * typedef int <*function_type2>(params2);
	 * 
	 * ...
	 *  
	 */

	
	#ifndef _CDP_AVP_EPCAPP_H_2
	#define _CDP_AVP_EPCAPP_H_2




	#endif //_CDP_AVP_EPCAPP_H_2
	
#endif



#define CDP_AVP_UNDEF_MACROS
	#include "macros.h"
#undef CDP_AVP_UNDEF_MACROS
	



