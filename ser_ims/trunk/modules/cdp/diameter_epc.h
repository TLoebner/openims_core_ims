/**
 * $Id$
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
 */
 
/**
 * \file 
 * CDiameterPeer Diameter IMS IANA defined numbers
 * 
 * This is a compilation of different 3GPP TSs for EPC:
 * 
 *  \author Alberto Diez   alberto dot diez -at- fokus dot fraunhofer dot de
 *  
 */


#ifndef DIAMETER_EPC_H_
#define DIAMETER_EPC_H_

// Diameter Application Identifier used in the EPC

//this applications are specific to the PCC
#define EPC_Rx 	16777236
#define EPC_Gx 	16777238
#define EPC_Sta 16777250
#define EPC_S6a 16777251
#define EPC_SWm 16777264
#define EPC_SWx 16777265
#define EPC_Gxx 16777266
#define EPC_S9 	16777267
#define EPC_S6b	16777272
#define EPC_Sp	16777280 //not in current standards

#define EPC_vendor_id_3GPP 		10415		/**< Vendor Id for 3GPP */
#define EPC_vendor_id_3GPP_char "10415" 	/**< char value for 3GPP's Vendor Id */
#define EPC_vendor_id_3GPP_len	5			/**< len of char value for 3GPP's Vendor Id */

/*	Command Codes used in the EPC 	*/

/*		The Rx Interface 			*/
#define Diameter_AAR		265		/**< Bearer-Authorization		Request	*/
#define Diameter_AAA		265		/**< Bearer-Authorization		Answer	*/
#define Diameter_RAR		258		/**< Re-Auth					Request */
#define Diameter_RAA		258		/**< Re-Auth					Answer	*/
#define Diameter_STR		275		/**< Session Termination 		Request */
#define Diameter_STA		275		/**< Session Termination 		Answer	*/
#define Diameter_ASR		274		/**< Abort-Session-Request		Request */
#define Diameter_ASA		274		/**< Abort-Session-Request		Answer	*/
/* The Gx and Gxx Interface */
#define Diameter_CCR		272
#define Diameter_CCA		272

/* The Sh/Sp interface */
#define Diameter_UDR 		306
#define Diameter_UDA 		306
#define Diameter_PUR		307
#define Diameter_PUA		307
#define Diameter_SNR		308
#define Diameter_SNA		308
#define Diameter_PNR		309
#define Diameter_PNA		309

/** 3GPP AVP Codes */
enum {
/**   1 to 255 reserved for backward compatibility with Radius TS29.061	*/


	AVP_EPC_User_Location_Info							= 22,
	AVP_EPC_RAI											= 909, //TS29.061
/** 256 to 299 reserved for future use										*/


/** 300 to 399 reserved for TS29.234											*/
	
/** 400 to 499 reserved for TS29.109 */
	AVP_EPC_Service_Identifier							= 439,
	AVP_EPC_Subscription_Id								= 443,
	AVP_EPC_Subscription_Id_Data						= 444,
	AVP_EPC_Subscription_Id_Type						= 450,
	AVP_EPC_User_Equipment_Info							= 458,
	AVP_EPC_User_Equipment_Info_Type					= 459,
	AVP_EPC_User_Equipment_Info_Data					= 460,
/**  or   RFC 4006 							*/

/** 500 to 599 reserved for TS29.209											*/
	AVP_EPC_Abort_Cause									= 500,
	AVP_EPC_Access_Network_Charging_Address				= 501,
	AVP_EPC_Access_Network_Charging_Identifier			= 502,
	AVP_EPC_Access_Network_Charging_Identifier_Value	= 503,
	AVP_EPC_AF_Application_Identifier					= 504,
	AVP_EPC_AF_Charging_Identifier						= 505,
	AVP_EPC_Authorization_Token							= 506,
	AVP_EPC_Flow_Description							= 507,
	AVP_EPC_Flow_Grouping								= 508,
	AVP_EPC_Flow_Number									= 509,
	AVP_EPC_Flows										= 510,
	AVP_EPC_Flow_Status									= 511,
	AVP_EPC_Flow_Usage									= 512,
	AVP_EPC_Specific_Action								= 513,
	AVP_EPC_Max_Requested_Bandwidth_DL					= 515,
	AVP_EPC_Max_Requested_Bandwidth_UL					= 516,
	AVP_EPC_Media_Component_Description					= 517,
	AVP_EPC_Media_Component_Number						= 518,
	AVP_EPC_Media_Sub_Component							= 519,
	AVP_EPC_Media_Type									= 520,
	AVP_EPC_RR_Bandwidth								= 521,
	AVP_EPC_RS_Bandwidth								= 522,
	AVP_EPC_SIP_Forking_Indication						= 523,
	AVP_EPC_Codec_Data									= 524,
/** 600 to 699 reserved for TS29.229											*/
/** 700 to 799 reserved for TS29.329											*/
	
/** 800 to 899 reserved for TS29.299											*/

/** 32.299 Partial																*/
	AVP_EPC_Served_Party_IP_Address						= 848,
	
/** 1000   from TS29.212 */
	AVP_EPC_Bearer_Usage								= 1000,
 	AVP_EPC_Charging_Rule_Install						= 1001,
 	AVP_EPC_Charging_Rule_Remove						= 1002,
 	AVP_EPC_Charging_Rule_Definition					= 1003,
 	AVP_EPC_Charging_Rule_Base_Name						= 1004,
 	AVP_EPC_Charging_Rule_Name							= 1005,
 	AVP_EPC_Event_Trigger								= 1006,
 	AVP_EPC_Metering_Method								= 1007,
 	AVP_EPC_Offline										= 1008,
 	AVP_EPC_Online										= 1009,
 	AVP_EPC_Precedence									= 1010,
 	AVP_EPC_Reporting_Level								= 1011,
 	AVP_EPC_TFT_Filter									= 1012,
 	AVP_EPC_TFT_Packet_Filter_Information				= 1013,
 	AVP_EPC_ToS_Traffic_Class							= 1014,
 	AVP_EPC_QoS_Bandwidth								= 1015,  //Not used in the EPC
 	AVP_EPC_QoS_Information								= 1016,
 	AVP_EPC_QoS_Jitter									= 1017,  //Not used in the EPC
 	AVP_EPC_Charging_Rule_Report						= 1018,
 	AVP_EPC_Pcc_Rule_Status								= 1019,
 	AVP_EPC_Bearer_Identifier							= 1020,
 	AVP_EPC_Bearer_Operation							= 1021,
 	AVP_EPC_Access_Network_Charging_Identifier_Gx		= 1022,
 	AVP_EPC_Bearer_Control_Mode							= 1023,
 	AVP_EPC_Network_Request_Support						= 1024,
 	AVP_EPC_Guaranteed_Bitrate_DL						= 1025,
 	AVP_EPC_Guaranteed_Bitrate_UL						= 1026,
 	AVP_EPC_IPCAN_Type									= 1027,
 	AVP_EPC_QoS_Class_Identifier						= 1028,
 	AVP_EPC_QoS_Negotiation								= 1029,
 	AVP_EPC_QoS_Upgrade									= 1030,
 	AVP_EPC_Rule_Failure_Code							= 1031,
 	AVP_EPC_RAT_Type									= 1032,
 	AVP_EPC_Event_Report_Indication						= 1033,
 	AVP_EPC_Allocation_Retention_Priority				= 1034,
 	AVP_EPC_CoA_IP_Address								= 1035,
 	AVP_EPC_Tunnel_Header_Filter						= 1036,
 	AVP_EPC_Tunnel_Header_Length						= 1037,
 	AVP_EPC_Tunnel_Information							= 1038,
 	AVP_EPC_CoA_Information								= 1039,
 	AVP_EPC_APN_Aggregate_Max_Bitrate_DL				= 1040,
 	AVP_EPC_APN_Aggregate_Max_Bitrate_UL				= 1041,
 	AVP_EPC_Revalidation_Time							= 1042,
 	AVP_EPC_Rule_Activation_Time						= 1043,
 	AVP_EPC_Rule_Deactivation_Time						= 1044,
 	AVP_EPC_Session_Release_Cause						= 1045,
 	AVP_EPC_ARP_Value									= 1046,
 	AVP_EPC_PreEmption_Capability						= 1047,
 	AVP_EPC_PreEmption_Vulnerability					= 1048,
 	AVP_EPC_Default_EPS_Bearer_QoS						= 1049,
 	AVP_EPC_ANGw_Address								= 1050,
 	AVP_EPC_Resource_Allocation_Notification			= 1051, //Gx
 	AVP_EPC_QoS_Rule_Install							= 1051, //Gxx
 	AVP_EPC_QoS_Rule_Remove								= 1052,
 	AVP_EPC_QoS_Rule_Definition							= 1053,
 	AVP_EPC_QoS_Rule_Name								= 1054,
 	AVP_EPC_QoS_Rule_Report								= 1055,
 	AVP_EPC_Security_Parameter_Index					= 1056,
 	AVP_EPC_Flow_Label									= 1057,
 	AVP_EPC_Flow_Information							= 1058,
	AVP_EPC_Packet_Filter_Content						= 1059,
	AVP_EPC_Packet_Filter_Identifier					= 1060,
	AVP_EPC_Packet_Filter_Information					= 1061,
	AVP_EPC_Packet_Filter_Operation						= 1062,
 	
/** TS 29.272 - selection */
	AVP_EPC_Subscription_Data							= 1400,
	AVP_EPC_Context_Identifier							= 1423,
	AVP_EPC_Operator_Determined_Barring					= 1425,
	AVP_EPC_Access_Restriction_Data						= 1426,
	AVP_EPC_APN_OI_Replacement							= 1427,
	AVP_EPC_APN_Configuration_Profile					= 1429,
	AVP_EPC_APN_Configuration							= 1430,
	AVP_EPC_EPS_Subscribed_QoS_Profile					= 1431,
	AVP_EPC_VPLMN_Dynamic_Address_Allowed				= 1432,
	AVP_EPC_AMBR										= 1435,
	AVP_EPC_PDN_Gw_Allocation_Type						= 1438,
	AVP_EPC_PDN_Type									= 1456,
	AVP_EPC_PDN_Context									= 1469,
	
/** Not yet allocated */	
	AVP_EPC_Service_Selection							= 42001, /**< http://tools.ietf.org/html/draft-ietf-dime-mip6-split-17#section-6.2 */
	AVP_EPC_PDN_Gw_Address								= 42002, 
	AVP_EPC_PDN_Gw_Name									= 42003, 
	AVP_EPC_PDN_Gw_Identity								= 42004, 
};

/** 3GPP TS 29.212	*/
enum {
	AVP_EPC_Metering_Method_Duration			=0,
	AVP_EPC_Metering_Method_Volume				=1,
	AVP_EPC_Metering_Method_Duration_Volume		=2
};

enum {
	AVP_EPC_Offline_Disable 		=0,
	AVP_EPC_Offline_Enable 			=1
};


enum {
	AVP_EPC_Online_Disable 			=0,
	AVP_EPC_Online_Enable 			=1
};

enum {
	AVP_EPC_Reporting_Level_Serving_Identifier 	=0,
	AVP_EPC_Reporting_Level_Rating_Group		=1
};

enum {
	AVP_EPC_Pcc_Rule_Status_Active					=0,
	AVP_EPC_Pcc_Rule_Status_Inactive				=1,
	AVP_EPC_Pcc_Rule_Status_Temporarily_Inactive 	=2
};
enum {
	AVP_EPC_Bearer_Usage_General 		=0,
	AVP_EPC_Bearer_Usage_IMS_Signaling 	=1
};
enum {
	AVP_EPC_Bearer_Operation_Termination 		=0,
	AVP_EPC_Bearer_Operation_Establishment 		=1,
	AVP_EPC_Bearer_Operation_Modification		=2
};
enum {
	AVP_EPC_Bearer_Control_Mode_UE_Only			=0,
	AVP_EPC_Bearer_Control_Mode_Reserved		=1,
	AVP_EPC_Bearer_Control_Mode_UE_NW			=2
};
enum {
	AVP_EPC_Network_Request_Support_Not_Supported			=0,
	AVP_EPC_Network_Request_Support_Supported				=1
};

/** IP-CAN Type TS 29.212 */ 
enum {
	AVP_EPC_IPCAN_Type_3GPP_GPRS 	= 0,
	AVP_EPC_IPCAN_Type_DOCSIS 		= 1,
	AVP_EPC_IPCAN_Type_xDSL 		= 2,
	AVP_EPC_IPCAN_Type_WiMAX		= 3,
	AVP_EPC_IPCAN_Type_3GPP2		= 4,
	AVP_EPC_IPCAN_Type_3GPP_EPS		= 5
};

enum {
	AVP_EPC_QoS_Negotiation_No			=0,
	AVP_EPC_QoS_Negotiation_Supported	=1
};

enum {
	AVP_EPC_QoS_Upgrade_No				=0,
	AVP_EPC_QoS_Upgrade_Supported		=1
};

enum {
	AVP_EPC_RAT_Type_WLAN				=0,
	AVP_EPC_RAT_Type_UTRAN				=1000,
	AVP_EPC_RAT_Type_GERAN				=1001,
	AVP_EPC_RAT_Type_GAN				=1002,
	AVP_EPC_RAT_Type_HSPA_Evolution		=1003,
	AVP_EPC_RAT_Type_EUTRAN				=1004,
	AVP_EPC_RAT_Type_CDMA2000_1X		=2000,
	AVP_EPC_RAT_Type_HRPD				=2001,
	AVP_EPC_RAT_Type_UMB				=2002 //deprecated
};


enum {
	AVP_EPC_Rule_Failure_Code_Unknown_Rule_Name				=1,
	AVP_EPC_Rule_Failure_Code_Rating_Group_Error			=2,
	AVP_EPC_Rule_Failure_Code_Service_Identifier_Error		=3,
	AVP_EPC_Rule_Failure_Code_GW_PCEF_Malfunction			=4,
	AVP_EPC_Rule_Failure_Code_Resources_Limitation			=5,
	AVP_EPC_Rule_Failure_Code_Max_Nr_Bearers_Reached		=6,
	AVP_EPC_Rule_Failure_Code_Unkown_Bearer_Id				=7,
	AVP_EPC_Rule_Failure_Code_Missing_Bearer_Id				=8,
	AVP_EPC_Rule_Failure_Code_Missing_Flow_Description		=9,
	AVP_EPC_Rule_Failure_Code_Resource_Allocation_Failure	=10,
	AVP_EPC_Rule_Failure_Code_Unsuccessful_QoS_Validation	=11
};

enum {
	AVP_EPC_Session_Release_Cause_Unespecified				=0,
	AVP_EPC_Session_Release_Cause_UE_Subscription			=1,
	AVP_EPC_Session_Release_Cause_Server_Resources			=2
};

enum {
	AVP_EPC_PreEmption_Capability_Enabled			=0,
	AVP_EPC_PreEmption_Capability_Disabled			=1
};

enum {
	AVP_EPC_PreEmption_Vulnerability_Enabled			=0,
	AVP_EPC_PreEmption_Vulnerability_Disabled			=1
};

/** Event-Trigger TS 29.212*/
enum {	
	AVP_EPC_Event_Trigger_SGSN_CHANGE 							=0, 
	AVP_EPC_Event_Trigger_QOS_CHANGE 							=1,
	AVP_EPC_Event_Trigger_RAT_CHANGE 							=2,	
	AVP_EPC_Event_Trigger_TFT_CHANGE 							=3,
	AVP_EPC_Event_Trigger_PLMN_CHANGE 							=4,
	AVP_EPC_Event_Trigger_LOSS_OF_BEARER 						=5,
	AVP_EPC_Event_Trigger_RECOVERY_OF_BEARER					=6,
	AVP_EPC_Event_Trigger_IP_CAN_CHANGE							=7,
	AVP_EPC_Event_Trigger_GW_PCEF_MALFUNCTION 					=8, //Release 7
	AVP_EPC_Event_Trigger_RESOURCES_LIMITATION 					=9, //Release 7
	AVP_EPC_Event_Trigger_MAX_NR_BEARERS_REACHED 				=10, //Release 7
	AVP_EPC_Event_Trigger_QOS_CHANGE_EXCEEDING_AUTHORIZATION	=11,
	AVP_EPC_Event_Trigger_RAI_CHANGE							=12, 
	AVP_EPC_Event_Trigger_USER_LOCATION_CHANGE 					=13,
	AVP_EPC_Event_Trigger_NO_EVENT_TRIGGER						=14,	 
	AVP_EPC_Event_Trigger_OUT_OF_CREDIT							=15, 
	AVP_EPC_Event_Trigger_RELLOCATION_OF_CREDIT					=16,
	AVP_EPC_Event_Trigger_REVALIDATION_TIMEOUT					=17,
	AVP_EPC_Event_Trigger_IP_ADDRESS_ALLOCATE					=18,
	AVP_EPC_Event_Trigger_IP_ADDRESS_RELEASE					=19,
	AVP_EPC_Event_Trigger_DEFAULT_EPS_BEARER_QOS_CHANGE			=20,
	AVP_EPC_Event_Trigger_AN_GW_CHANGE							=21,
	AVP_EPC_Event_Trigger_Successful_Resource_Allocation		=22,
	AVP_EPC_Event_Trigger_Resource_Modification_Request			=23
};

enum {
	AVP_EPC_Packet_Filter_Operation_Deletion					=0,
	AVP_EPC_Packet_Filter_Operation_Addition					=1,
	AVP_EPC_Packet_Filter_Operation_Modification				=2
};

/* This extends the AVP_IMS_Data_Reference_* for Sh to Sp which is not yet standardized */
enum {
	AVP_EPC_Data_Reference_Subscription_Id_MSISDN				= 101,
	AVP_EPC_Data_Reference_Subscription_Id_IMSI					= 102,
	AVP_EPC_Data_Reference_Subscription_Id_IMPU					= 103,
	AVP_EPC_Data_Reference_Subscription_Data					= 104,
	AVP_EPC_Data_Reference_APN_Configuration					= 105,
	
};

/* from RFC4006 */
enum {
	AVP_EPC_Subscription_Id_Type_End_User_E164					= 0,
	AVP_EPC_Subscription_Id_Type_End_User_IMSI					= 1,
	AVP_EPC_Subscription_Id_Type_End_User_SIP_URI				= 2,
	AVP_EPC_Subscription_Id_Type_End_User_NAI					= 3,	
	AVP_EPC_Subscription_Id_Type_End_User_Private				= 4,	
};

enum {
	RC_EPC_DIAMETER_QOS_RULE_EVENT								= 5145,
	RC_EPC_DIAMETER_BEARER_EVENT								= 5146,
	RC_EPC_DIAMETER_PCC_RULE_EVENT								= 5142

};
#endif /*DIAMETER_EPC_H_*/
