/**
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
 * CDiameterPeer Diameter AVP Codes
 
 *  \note This file is mostly taken from DISC http://developer.berlios.de/projects/disc/
 *  
 *  \author Dragos Vingarzan dragos dot vingarzan -at- fokus dot fraunhofer dot de
 * 
 */ 
#ifndef DIAMETER_CODE_AVP_H_
#define DIAMETER_CODE_AVP_H_




/** Standard AVP Codes */
typedef enum {
	
	/* RFC 3588 */
	
	AVP_User_Name                     =    1,
	AVP_Framed_IP_Address             =	   8,	
	AVP_Class                         =   25,
	AVP_Session_Timeout               =   27,
	AVP_Called_Station_Id			  =   30,
	AVP_Proxy_State                   =   33,
	AVP_Acct_Session_Id				  =   44,
	AVP_Acct_Multi_Session_Id		  =   50,
	AVP_Event_Timestamp               =   55,
	AVP_NAS_Port_Type                 =   61,
	AVP_Acct_Interim_Interval         =   85,	
	AVP_Framed_Interface_Id           =   96,	
	AVP_Framed_IPv6_Prefix            =   97,
	AVP_Host_IP_Address               =  257,
	AVP_Auth_Application_Id           =  258,
	AVP_Acct_Application_Id           =  259,	
	AVP_Vendor_Specific_Application_Id=  260,
	AVP_Redirect_Host_Usage			  =  261,
	AVP_Redirect_Max_Cache_Time       =  262,
	AVP_Session_Id                    =  263,
	AVP_Origin_Host                   =  264,
	AVP_Supported_Vendor_Id           =  265,
	AVP_Vendor_Id                     =  266,
	AVP_Firmware_Revision             =  267,
	AVP_Result_Code                   =  268,
	AVP_Product_Name                  =  269,
	AVP_Session_Binding               =  270,
	AVP_Session_Server_Failover		  =  271,
	AVP_Multi_Round_Time_Out          =  272,
	AVP_Disconnect_Cause              =  273,
	AVP_Auth_Request_Type             =  274,
	AVP_Auth_Grace_Period             =  276,
	AVP_Auth_Session_State            =  277,
	AVP_Origin_State_Id               =  278,
	AVP_Failed_AVP					  =  279,
	AVP_Proxy_Host                    =  280,
	AVP_Error_Message                 =  281,
	AVP_Route_Record                  =  282,
	AVP_Destination_Realm             =  283,
	AVP_Proxy_Info                    =  284,
	AVP_Re_Auth_Request_Type          =  285,
	AVP_Accounting_Sub_Session_Id	  =  287,
	AVP_Authorization_Lifetime        =  291,
	AVP_Redirect_Host                 =  292,
	AVP_Destination_Host              =  293,
	AVP_Error_Reporting_Host		  =  294,
	AVP_Termination_Cause             =  295,
	AVP_Origin_Realm                  =  296,
	AVP_Experimental_Result			  =  297,
	AVP_Experimental_Result_Code      =  298,
	AVP_Inband_Security_Id			  =  299,
	AVP_E2E_Sequence				  =  300,
	
	/* RFC 4004 */
	AVP_MIP_Reg_Request               =  320, 
	AVP_MIP_Reg_Reply                 =  321, 
	AVP_MIP_MN_AAA_Auth               =  322, 
	AVP_MIP_Mobile_Node_Address       =  333, 
	AVP_MIP_Home_Agent_Address        =  334, 
	AVP_MIP_Candidate_Home_Agent_Host =  336, 
	AVP_MIP_Feature_Vector            =  337, 
	AVP_MIP_Auth_Input_Data_Length    =  338, 
	AVP_MIP_Authenticator_Length      =  339, 
	AVP_MIP_Authenticator_Offset      =  340, 
	AVP_MIP_MN_AAA_SPI                =  341, 
	AVP_MIP_Filter_Rule               =  342, 
	AVP_MIP_FA_Challenge              =  344, 
	AVP_MIP_Originating_Foreign_AAA   =  347, 
	AVP_MIP_Home_Agent_Host           =  348, 
	
	/* RFC 4006 */
	AVP_CC_Request_Number             =  415, 
	AVP_CC_Request_Type               =  416, 
	AVP_Final_Unit_Indication		  =  430, 
	AVP_Rating_Group				  =  432, 
	AVP_Service_Identifier			  =  439, 
	AVP_Subscription_Id				  =  443, 
	AVP_Subscription_Id_Data		  =  444, 
	AVP_Final_Unit_Action			  =  449, 
	AVP_Subscription_Id_Type		  =  450, 
	AVP_User_Equipment_Info	          =  458, 
	AVP_User_Equipment_Info_Type      =  459, 
	AVP_User_Equipment_Info_Value	  =  460, 
	
	
	AVP_Accounting_Record_Type        =  480,
	AVP_Accounting_Realtime_Required  =  483,
	AVP_Accounting_Record_Number      =  485,
	AVP_MIP6_Agent_Info				  =  486, //RFC5447
	
}AAA_AVPCodeNr;


typedef enum {
        Permanent_Termination   = 0,
        New_Server_Assigned     = 1,
        Server_Change           = 2,
        Remove_S_CSCF           = 3,
}AAA_AVPReasonCode;

typedef enum {
	STATE_MAINTAINED			= 0,
	NO_STATE_MAINTAINED			= 1
} AAA_AVP_Auth_Session_State;


/** Accounting message types */
typedef enum {
	AAA_ACCT_EVENT = 1,
	AAA_ACCT_START = 2,
	AAA_ACCT_INTERIM = 3,
	AAA_ACCT_STOP = 4
} AAAAcctMessageType;


#endif /*DIAMETER_H_*/
