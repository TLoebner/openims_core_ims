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
 * I/S-CSCF Module - Cx AVP Operations 
 * 
 * Scope:
 * - Defining Operations between I/S-CSCF <-> HSS
 *   
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 * Copyright (C) 2005 FhG Fokus
 * 
 */

#include "cx_avp.h"
#include "../../mem/shm_mem.h"

extern struct cdp_binds cdpb;            /**< Structure with pointers to cdp funcs 		*/

/**
 * Main method for adding diverse entities as AVP.
 * Many methods below with adding functions are the wrapper of
 * this functionality.
 * \param m  Diameter Response
 * \param d String 
 * \param len 
 * \param avp_code AVP Code
 * \param flags for the avp code
 * \param vendorid
 * \param data_do
 * \param func 
 * \return
 */
static inline int Cx_add_avp(AAAMessage *m,char *d,int len,int avp_code,
	int flags,int vendorid,int data_do,const char *func)
{
	AAA_AVP *avp;
	if (vendorid!=0) flags |= AAA_AVP_FLAG_VENDOR_SPECIFIC;
	avp = cdpb.AAACreateAVP(avp_code,flags,vendorid,d,len,data_do);
	if (!avp) {
		LOG(L_ERR,"ERR:"M_NAME":%s: Failed creating avp\n",func);
		return 0;
	}
	if (cdpb.AAAAddAVPToMessage(m,avp,m->avpList.tail)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERR:"M_NAME":%s: Failed adding avp to message\n",func);
		cdpb.AAAFreeAVP(&avp);
		return 0;
	}
	return 1;
}

/**
 * Adds an Attribute Value Pair to the list
 * \param m  Diameter Response
 * \param d String 
 * \param len 
 * \param avp_code AVP Code
 * \param flags for the avp code
 * \param vendorid
 * \param data_do
 * \param func 
 * \return
 */
static inline int Cx_add_avp_list(AAA_AVP_LIST *list,char *d,int len,int avp_code,
	int flags,int vendorid,int data_do,const char *func)
{
	AAA_AVP *avp;
	if (vendorid!=0) flags |= AAA_AVP_FLAG_VENDOR_SPECIFIC;
	avp = cdpb.AAACreateAVP(avp_code,flags,vendorid,d,len,data_do);
	if (!avp) {
		LOG(L_ERR,"ERR:"M_NAME":%s: Failed creating avp\n",func);
		return 0;
	}
	if (list->tail) {
		avp->prev=list->tail;
		avp->next=0;	
		list->tail->next = avp;
		list->tail=avp;
	} else {
		list->head = avp;
		list->tail = avp;
		avp->next=0;
		avp->prev=0;
	}
	
	return 1;
}

/**
 * Gets Information of AVP field given in avp_code
 * \param msg Diameter Response
 * \param avp_code query field of Information
 * \param vendor_id
 * \param func 
 * \return data as String on success, 0 on fail
 */
static inline str Cx_get_avp(AAAMessage *msg,int avp_code,int vendor_id,
							const char *func)
{
	AAA_AVP *avp;
	str r={0,0};
	
	avp = cdpb.AAAFindMatchingAVP(msg,0,avp_code,vendor_id,0);
	if (avp==0){
		LOG(L_INFO,"INFO:"M_NAME":%s: Failed finding avp\n",func);
		return r;
	}
	else 
		return avp->data;
}




/**
 * Creates an AVP with User Name
 * \param msg  Diameter Response
 * \param data String 
 * \return 1 on success 0 on fail
 */
inline int Cx_add_user_name(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_User_Name,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates an AVP with public Identity
 * \param msg  Diameter Response
 * \param data String 
 * \return 1 on success 0 on fail
 */
inline int Cx_add_public_identity(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_IMS_Public_Identity,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates an AVP with Network ID (visited)
 * \param msg  Diameter Response
 * \param data String 
 * \return 1 on success 0 on fail
 */
inline int Cx_add_visited_network_id(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_IMS_Visited_Network_Identifier,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates an AVP with authorization type
 * \param msg  Diameter Response
 * \param data 
 * \return 1 on success 0 on fail
 */
inline int Cx_add_authorization_type(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_IMS_User_Authorization_Type,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}


/**
 * Adds a server name
 * \param msg  Diameter Response
 * \param data 
 * \return Cx_add_avp
 */
inline int Cx_add_server_name(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_IMS_Server_Name,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Adding sip numbers of authorized entities
 * \param msg  Diameter Response
 * \param data sip number
 * \return 1 on success 0 on fail
 */
inline int Cx_add_sip_number_auth_items(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_IMS_SIP_Number_Auth_Items,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}



/**
 * Adds sip authorization data 
 * \param msg  Diameter Response
 * \param auth_scheme Authentication Scheme 
 * \paramauth Authorization
 * \return 1 on success 0 on fail
 */
inline int Cx_add_sip_auth_data_item_request(AAAMessage *msg,str auth_scheme,str auth)
{
	AAA_AVP_LIST list;
	str group;
	list.head=0;list.tail=0;
		
	if (auth_scheme.len){
		Cx_add_avp_list(&list,
			auth_scheme.s,auth_scheme.len,
			AVP_IMS_SIP_Authentication_Scheme,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}	
	if (auth.len){
		Cx_add_avp_list(&list,
			auth.s,auth.len,
			AVP_IMS_SIP_Authorization,
			AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
			IMS_vendor_id,
			AVP_DONT_FREE_DATA,
			__FUNCTION__);
	}
	if (!list.head) return 1;
	group = cdpb.AAAGroupAVPS(list);
	
	cdpb.AAAFreeAVPList(list);
	
	return 
	Cx_add_avp(msg,group.s,group.len,
		AVP_IMS_SIP_Auth_Data_Item,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id,
		AVP_FREE_DATA,
		__FUNCTION__);
}

/**
 * Adds an Assignment type.
 * \param msg  Diameter Response
 * \param data
 * \return 1 on success 0 on fail
 */
inline int Cx_add_server_assignment_type(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_IMS_Server_Assignment_Type,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Adds an Attribute Value Pair for availibility of userdaata
 * \param msg  Diameter Response
 * \param data
 * \return 1 on success 0 on fail
 */
inline int Cx_add_userdata_available(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_IMS_User_Data_Already_Available,
		AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,
		IMS_vendor_id,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Adds an Attribute Value Pair with result code
 * \param msg  Diameter Response
 * \param data 
 * \return 1 on success 0 on fai
 */
inline int Cx_add_result_code(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_Result_Code,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Experimental Result Codes
 * \param msg  Diameter Response
 * \param data 
 * \return 1 on success 0 on fail
 */
inline int Cx_add_experimental_result_code(AAAMessage *msg,unsigned int data)
{
	AAA_AVP_LIST list;
	str group;
	char x[4];
	list.head=0;list.tail=0;
		
	set_4bytes(x,data);
	Cx_add_avp_list(&list,
		x,4,
		AVP_IMS_Experimental_Result_Code,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
	set_4bytes(x,IMS_vendor_id);
	Cx_add_avp_list(&list,
		x,4,
		AVP_IMS_Vendor_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
	
	group = cdpb.AAAGroupAVPS(list);
	
	cdpb.AAAFreeAVPList(list);
	
	return 
	Cx_add_avp(msg,group.s,group.len,
		AVP_IMS_Experimental_Result,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_FREE_DATA,
		__FUNCTION__);
}

/**
 * Adds Specific application_id
 * \param msg  Diameter Response
 * \param vendor_id
 * \param auth_id Authorization Application ID
 * \param acct_id Accounting Application ID
 * \return 
 */
inline int Cx_add_vendor_specific_appid(AAAMessage *msg,unsigned int vendor_id,
	unsigned int auth_id,unsigned int acct_id)
{
	AAA_AVP_LIST list;
	str group;
	char x[4];

	list.head=0;list.tail=0;
		
	set_4bytes(x,vendor_id);
	Cx_add_avp_list(&list,
		x,4,
		AVP_Vendor_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);

	if (auth_id) {
		set_4bytes(x,auth_id);
		Cx_add_avp_list(&list,
			x,4,
			AVP_Auth_Application_Id,
			AAA_AVP_FLAG_MANDATORY,
			0,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
	}
	if (acct_id) {
		set_4bytes(x,acct_id);
		Cx_add_avp_list(&list,
			x,4,
			AVP_Acct_Application_Id,
			AAA_AVP_FLAG_MANDATORY,
			0,
			AVP_DUPLICATE_DATA,
			__FUNCTION__);
	}	
	
	group = cdpb.AAAGroupAVPS(list);
	
	cdpb.AAAFreeAVPList(list);
	
	return 
	Cx_add_avp(msg,group.s,group.len,
		AVP_Vendor_Specific_Application_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_FREE_DATA,
		__FUNCTION__);
}


/**
 * Adds an AVP with session state of Authorization
 * \param msg Diameter Response
 * \param data 
 * \return 1 on success 0 on fail
 */
inline int Cx_add_auth_session_state(AAAMessage *msg,unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Cx_add_avp(msg,x,4,
		AVP_Auth_Session_State,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Adds an Attribute Value Pair of user destionation realm
 * \param msg  Diameter Response
 * \param data String 
 * \return 1 on success 0 on fail
 */
inline int Cx_add_destination_realm(AAAMessage *msg,str data)
{
	return 
	Cx_add_avp(msg,data.s,data.len,
		AVP_Destination_Realm,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}


/**
 * Gets the session id for that call
 * \param msg  Diameter Response
 * \return Session_Id on success 0 on fail
 */
inline str Cx_get_session_id(AAAMessage *msg)
{
	return Cx_get_avp(msg,
		AVP_Session_Id,
		0,
		__FUNCTION__);
}


/**
 * get the User Name from the Response
 * \param msg  Diameter Response(pointer)
 * \return str as User_Name
 * \return user_name on success 0 on fail
 */
inline str Cx_get_user_name(AAAMessage *msg)
{
	return Cx_get_avp(msg,
		AVP_User_Name,
		0,
		__FUNCTION__);
}

/**
 * Get IMS Public Identity
 * \param msg  Diameter Response
 * \return identity on success 0 on fail
 */
inline str Cx_get_public_identity(AAAMessage *msg)
{
	return Cx_get_avp(msg,
		AVP_IMS_Public_Identity,
		IMS_vendor_id,
		__FUNCTION__);
}

/**
 * Finds out the next public Identity from response
 * \param msg  Diameter Response
 * \param pos  position 
 * \param avp_code AVP Code
 * \param vendor_id
 * \param func function
 * \return Attribute-Value-Pair
 */
inline AAA_AVP* Cx_get_next_public_identity(AAAMessage *msg,AAA_AVP* pos,int avp_code,int vendor_id,const char *func){		
{
	AAA_AVP *avp;
	
	avp = cdpb.AAAFindMatchingAVP(msg,pos,avp_code,vendor_id,0);
	if (avp==0){
		LOG(L_INFO,"INFO:"M_NAME":%s: Failed finding avp\n",func);
		return avp;
	}
	else 
		return avp;
}
	
	
}

/**
 * Get the Identifier of a visited network
 * \param msg  Diameter Response
 * \return the network ID on success 0 on fail
 */
inline str Cx_get_visited_network_id(AAAMessage *msg)
{
	return Cx_get_avp(msg,
		AVP_IMS_Visited_Network_Identifier,
		IMS_vendor_id,
		__FUNCTION__);
}

/**
 * Getter for Authorization Type of IMS User
 * \param msg Diameter Response
 * \param d Authorization Type
 * \return 1 on success 0 on fail
 */
inline int Cx_get_authorization_type(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_IMS_User_Authorization_Type,
		IMS_vendor_id,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}

/**
 * Adds an Attribute Value Pair to the list
 * \param msg  Diameter Response
 * \param data Assignment Type
 * \return 1 on success 0 on fail
 */
inline int Cx_get_server_assignment_type(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_IMS_Server_Assignment_Type,
		IMS_vendor_id,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}

/**
 * Checks out if the userdata is availible
 * \param msg  Diameter Response
 * \param data Availibility
 * \return 1 on success 0 on fail
 */
inline int Cx_get_userdata_available(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_IMS_User_Data_Already_Available,
		IMS_vendor_id,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}


/**
 * Gets the result code
 * \param msg  Diameter Response
 * \param data integer pointer
 * \return 1 on success 0 on fail
 */
inline int Cx_get_result_code(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_Result_Code,
		0,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}

/**
 * Get experimental results
 * \param msg  Diameter Response
 * \param data pointer to int
 * \return 1 on success 0 on fail
 */
inline int Cx_get_experimental_result_code(AAAMessage *msg, int *data)
{
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	grp = Cx_get_avp(msg,
		AVP_IMS_Experimental_Result,
		0,
		__FUNCTION__);
	if (!grp.s) return 0;

	list = cdpb.AAAUngroupAVPS(grp);
	
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Experimental_Result_Code,0,0);
	if (!avp||!avp->data.s) {
		cdpb.AAAFreeAVPList(list);
		return 0;
	}

	*data = get_4bytes(avp->data.s);
	cdpb.AAAFreeAVPList(list);

	return 1;
}

/**
 * Get the IMS Server Name
 * \param msg Diameter Response
 * \return IMS Server Name on successs 0 on fail
 */
inline str Cx_get_server_name(AAAMessage *msg)
{	
	return Cx_get_avp(msg,
		AVP_IMS_Server_Name,
		IMS_vendor_id,
		__FUNCTION__);
}


/**
 * Gets IMS Server Mandatory/Optional Capabilities
 * \param msg  Diameter Response
 * \param int m Memory in bytes needed for mandatory caps.
 * \param int m_cnt mandatory cap. counter
 * \param int o Memory in bytes needed for opt.caps.
 * \param int o_cnt optional cap. counter
 * \return 1 on success 0 on fail
 */
inline int Cx_get_capabilities(AAAMessage *msg,int **m,int *m_cnt,int **o,int *o_cnt)
{
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	grp = Cx_get_avp(msg,
		AVP_IMS_Server_Capabilities,
		IMS_vendor_id,
		__FUNCTION__);
	if (!grp.s) return 0;

	list = cdpb.AAAUngroupAVPS(grp);
	
	avp = list.head;
	*m_cnt=0;
	*o_cnt=0;
	while(avp){
		if (avp->code == AVP_IMS_Mandatory_Capability) (*m_cnt)++;
		if (avp->code == AVP_IMS_Optional_Capability) (*o_cnt)++;		
		avp = avp->next;
	}
	avp = list.head;
	*m=shm_malloc(sizeof(int)*(*m_cnt));
	*o=shm_malloc(sizeof(int)*(*o_cnt));
	*m_cnt=0;
	*o_cnt=0;
	while(avp){
		if (avp->code == AVP_IMS_Mandatory_Capability) 
			(*m)[(*m_cnt)++]=get_4bytes(avp->data.s);
		if (avp->code == AVP_IMS_Optional_Capability)		
			(*o)[(*o_cnt)++]=get_4bytes(avp->data.s);
		avp = avp->next;
	}
	cdpb.AAAFreeAVPList(list);
	return 1;
}

/**
 * Get sip numbers of authorized entities
 * \param msg  Diameter Response
 * \param data 
 * \return 1 on success 0 on fail.
 */
inline int Cx_get_sip_number_auth_items(AAAMessage *msg, int *data)
{
	str s;
	s = Cx_get_avp(msg,
		AVP_IMS_SIP_Number_Auth_Items,
		IMS_vendor_id,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}

/**
 * Gets a matching Authentication Scheme from the AVP list
 * \param msg  Diameter Response
 * \param auth_scheme as  Authentication Scheme - String 
 * \param authorization as String
 * \return 1 on success 0 on fail
 */
inline int Cx_get_auth_data_item_request(AAAMessage *msg,
		 str *auth_scheme, str *authorization)
{
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	grp = Cx_get_avp(msg,
		AVP_IMS_SIP_Auth_Data_Item,
		IMS_vendor_id,
		__FUNCTION__);
	if (!grp.s) return 0;

	list = cdpb.AAAUngroupAVPS(grp);
	
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authentication_Scheme,
		IMS_vendor_id,0);
	if (!avp||!avp->data.s) {
		cdpb.AAAFreeAVPList(list);
		return 0;
	}
	*auth_scheme = avp->data;
	
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authorization,
		IMS_vendor_id,0);
	if (avp) *authorization = avp->data;
	else {authorization->s=0;authorization->len=0;}		

	cdpb.AAAFreeAVPList(list);
	return 1;
}

/**
 * Get Authentication Data Item
 * \param msg  Diameter Response
 * \param auth_data Authentication Data(double pointer)
 * \param item_number 
 * \param auth_scheme
 * \param authenticate 
 * \param authorization
 * \param ck
 * \param ik
 * \return 1 on success 0 on fail
 */
int Cx_get_auth_data_item_answer(AAAMessage *msg, AAA_AVP **auth_data,
	int *item_number,str *auth_scheme,str *authenticate,str *authorization,
	str *ck,str *ik)
{
	AAA_AVP_LIST list;
	AAA_AVP *avp;
	str grp;
	
	*auth_data = cdpb.AAAFindMatchingAVP(msg,*auth_data,AVP_IMS_SIP_Auth_Data_Item,
		IMS_vendor_id,0);
	if (!*auth_data) return 0;
		
	grp = (*auth_data)->data;
	if (!grp.len) return 0;

	list = cdpb.AAAUngroupAVPS(grp);

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Item_Number,
		IMS_vendor_id,0);
	if (!avp||!avp->data.len==4) *item_number=0;
	else *item_number = get_4bytes(avp->data.s);
	
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authentication_Scheme,
		IMS_vendor_id,0);
	if (!avp||!avp->data.s) {auth_scheme->s=0;auth_scheme->len=0;}
	else *auth_scheme = avp->data;
	
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authenticate,
		IMS_vendor_id,0);
	if (!avp||!avp->data.s) {authenticate->s=0;authenticate->len=0;}
	else *authenticate = avp->data;
		
	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_SIP_Authorization,
		IMS_vendor_id,0);
	if (!avp||!avp->data.s) {authorization->s=0;authorization->len=0;}
	else *authorization = avp->data;

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Confidentiality_Key,
		IMS_vendor_id,0);
	if (!avp||!avp->data.s) {ck->s=0;ck->len=0;}
	else *ck = avp->data;

	avp = cdpb.AAAFindMatchingAVPList(list,0,AVP_IMS_Integrity_Key,
		IMS_vendor_id,0);
	if (!avp||!avp->data.s) {ik->s=0;ik->len=0;}
	else *ik = avp->data;

	cdpb.AAAFreeAVPList(list);
	return 1;
}


/**
 * Delivers the destination of the host from the response
 * \param msg  Diameter Response
 * \return location on success 0 on fail
 */
inline str Cx_get_destination_host(AAAMessage *msg)
{	
	return Cx_get_avp(msg,
		AVP_Destination_Host,
		0,
		__FUNCTION__);
}

/**
 * Delivers the user data from HSS Response
 * \param msg  Diameter Response
 * \return the user data on success 0 on fail
 */
inline str Cx_get_user_data(AAAMessage *msg)
{	
	return Cx_get_avp(msg,
		AVP_IMS_User_Data,
		IMS_vendor_id,
		__FUNCTION__);
}

/**
 * Chops out the Information for charging purposes
 * \param msg  Diameter Response
 * \return charging_info on success 0 on fail
 */
inline str Cx_get_charging_info(AAAMessage *msg)
{	
	return Cx_get_avp(msg,
		AVP_IMS_Charging_Information,
		IMS_vendor_id,
		__FUNCTION__);
}
