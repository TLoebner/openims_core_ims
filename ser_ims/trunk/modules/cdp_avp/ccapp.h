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
 * CDiameter AVP Operations modules - CC APP 4006 AVPs
 * 
 * 
 *  \author Dragos Vingarzan dragos dot vingarzan -at- fokus dot fraunhofer dot de
 * 
 */ 

/* 
 * RFC 4006 Diameter Credit Control Application
 * 
 * http://tools.ietf.org/html/rfc4006
 * 
 */

#include "macros.h"

#undef CDP_AVP_MODULE
#define CDP_AVP_MODULE ccapp

#if !defined(CDP_AVP_DECLARATION) && !defined(CDP_AVP_EXPORT) && !defined(CDP_AVP_INIT) && !defined(CDP_AVP_REFERENCE)
	#ifndef _CDP_AVP_CCAPP_H_1
	#define _CDP_AVP_CCAPP_H_1

		#include "../cdp/cdp_load.h"

	#else

		/* undo the macros definition if this was re-included */
		#define CDP_AVP_EMPTY_MACROS
			#include "macros.h"
		#undef CDP_AVP_EMPTY_MACROS

	#endif
#endif //_CDP_AVP_CCAPP_H_1	

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
 *  			OctetString 	- str
 *  			Integer32		- int32_t
 *  			Integer64 		- int64_t
 *  			Unsigned32 		- uint32_t
 *  			Unsigned64 		- uint64_t
 *  			Float32 		- float
 *  			Float64 		- double
 *  			Grouped 		- AAA_AVP_LIST
 *  
 *  			Address 		- ip_address
 *  			Time 			- time_t
 *  			UTF8String 		- str
 *  			DiameterIdentity- str
 *  			DiameterURI		- str
 *  			Enumerated		- int32_t
 *  			IPFilterRule	- str
 *  			QoSFilterRule	- str
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
 *  avp_name_group - a value of AVP_<avp_name_group> must resolve to the AVP code of the group
 *  
 *  vendor_id_group - an int value
 *  
 *  avp_name_N	- the name of the Nth parameter. 
 *  	Previously, a cdp_avp_get(<avp_name_N>,<vendor_id_N>,<avp_type_N>,<data_type_N>) must be defined!
 *  
 *  data_type_N	- the respective data type for avp_type_N (same as <data_type_N) 
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


cdp_avp			(CC_Request_Number,				0,	AAA_AVP_FLAG_MANDATORY,	Unsigned32,			uint32_t)

cdp_avp			(CC_Request_Type,				0,	AAA_AVP_FLAG_MANDATORY,	Enumerated,			int32_t)

cdp_avp			(Final_Unit_Action,				0,	AAA_AVP_FLAG_MANDATORY,	Enumerated,			int32_t)

cdp_avp_add_ptr	(Final_Unit_Indication,			0,	AAA_AVP_FLAG_MANDATORY,	Grouped,			AAA_AVP_LIST*)
cdp_avp_get		(Final_Unit_Indication,			0,	AAA_AVP_FLAG_MANDATORY,	Grouped,			AAA_AVP_LIST)

cdp_avp			(Rating_Group,					0,	AAA_AVP_FLAG_MANDATORY,	Unsigned32,			uint32_t)

cdp_avp			(Service_Identifier,			0,	AAA_AVP_FLAG_MANDATORY,	Unsigned32,			uint32_t)

cdp_avp			(Subscription_Id_Type,			0,	AAA_AVP_FLAG_MANDATORY,	Enumerated,			int32_t)

cdp_avp_ptr		(Subscription_Id_Data,			0,	AAA_AVP_FLAG_MANDATORY,	UTF8String,			str)

cdp_avp_add_ptr	(Subscription_Id,				0,	AAA_AVP_FLAG_MANDATORY,	Grouped,			AAA_AVP_LIST*)
cdp_avp_get		(Subscription_Id,				0,	AAA_AVP_FLAG_MANDATORY,	Grouped,			AAA_AVP_LIST)

cdp_avp			(User_Equipment_Info_Type,		0,	AAA_AVP_FLAG_MANDATORY,	Enumerated,			int32_t)

cdp_avp_ptr		(User_Equipment_Info_Value,		0,	AAA_AVP_FLAG_MANDATORY,	OctetString,		str)

cdp_avp_add_ptr	(User_Equipment_Info,			0,	AAA_AVP_FLAG_MANDATORY,	Grouped,			AAA_AVP_LIST*)
cdp_avp_get		(User_Equipment_Info,			0,	AAA_AVP_FLAG_MANDATORY,	Grouped,			AAA_AVP_LIST)

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
	
	int cdp_avp_add_Subscription_Id_Group(AAA_AVP_LIST *list,int32_t type, str data,AVPDataStatus data_do)
	{
		AAA_AVP_LIST list_grp={0,0};	
		if (!cdp_avp_add_Subscription_Id_Type(&list_grp,type)) goto error;
		if (!cdp_avp_add_Subscription_Id_Data(&list_grp,data,data_do)) goto error;
		return cdp_avp_add_to_list(list,
				cdp_avp_new_Grouped(
						AVP_Subscription_Id,
						AAA_AVP_FLAG_MANDATORY,
						0,
						&list_grp,
						AVP_FREE_DATA));	
		error:
			if (data_do==AVP_FREE_DATA && data.s) shm_free(data.s);
			cdp->AAAFreeAVPList(&list_grp);
		return 0;
	}

	int cdp_avp_get_Subscription_Id_Group(AAA_AVP_LIST list,int32_t *type,str *data,AAA_AVP **avp_ptr)
	{
		AAA_AVP_LIST list_grp={0,0};
		AAA_AVP *avp = cdp_avp_get_next_from_list(list,
				AVP_EPC_Subscription_Id,
				0,
				avp_ptr?*avp_ptr:0);
		if (avp_ptr) *avp_ptr= avp; 
		if (!avp) goto error;
		if (!cdp_avp_get_Grouped(avp,&list_grp)) goto error;
		if (!cdp_avp_get_Subscription_Id_Type(list_grp,type,0)) goto error;
		if (!cdp_avp_get_Subscription_Id_Data(list_grp,data,0)) goto error;
		cdp->AAAFreeAVPList(&list_grp);
		return 1;
	error:
		if (type) *type = 0;
		if (data) {data->s=0;data->len=0;}
		cdp->AAAFreeAVPList(&list_grp);
		return 0;
	}


	int cdp_avp_add_User_Equipment_Info_Group(AAA_AVP_LIST *list,int32_t type, str data,AVPDataStatus data_do)
	{
		AAA_AVP_LIST list_grp={0,0};	
		if (!cdp_avp_add_User_Equipment_Info_Type(&list_grp,type)) goto error;
		if (!cdp_avp_add_User_Equipment_Info_Value(&list_grp,data,data_do)) goto error;
		return cdp_avp_add_to_list(list,
				cdp_avp_new_Grouped(
						AVP_EPC_User_Equipment_Info,
						AAA_AVP_FLAG_MANDATORY,
						0,
						&list_grp,
						AVP_FREE_DATA));	
	error:
		if (data_do==AVP_FREE_DATA && data.s) shm_free(data.s);
		cdp->AAAFreeAVPList(&list_grp);
		return 0;
	}

	int cdp_avp_get_User_Equipment_Info_Group(AAA_AVP_LIST list,int32_t *type,str *data,AAA_AVP **avp_ptr)
	{
		AAA_AVP_LIST list_grp={0,0};
		AAA_AVP *avp = cdp_avp_get_next_from_list(list,
				AVP_EPC_User_Equipment_Info,
				0,
				avp_ptr?*avp_ptr:0);
		if (avp_ptr) *avp_ptr= avp;  
		if (!avp) goto error;
		if (!cdp_avp_get_Grouped(avp,&list_grp)) goto error;
		if (!cdp_avp_get_User_Equipment_Info_Type(list_grp,type,0)) goto error;
		if (!cdp_avp_get_User_Equipment_Info_Value(list_grp,data,0)) goto error;
		cdp->AAAFreeAVPList(&list_grp);
		return 1;
	error:
		if (type) *type = 0;
		if (data) {data->s=0;data->len=0;}
		cdp->AAAFreeAVPList(&list_grp);
		return 0;
	}
	
#elif defined(CDP_AVP_EXPORT)

	/*
	 * Put here your supplimentary exports in the format: 
	 * 	<function_type1> <nice_function_name1>; 
	 *  <function_type2> <nice_function_name1>;
	 *  ...
	 *  
	 */
	cdp_avp_add_Subscription_Id_Group_f		add_Subscription_Id_Group;
	cdp_avp_get_Subscription_Id_Group_f		get_Subscription_Id_Group;
	
	cdp_avp_add_User_Equipment_Info_Group_f		add_User_Equipment_Info_Group;
	cdp_avp_get_User_Equipment_Info_Group_f		get_User_Equipment_Info_Group;
	
#elif defined(CDP_AVP_INIT)

	/*
	 * Put here your supplimentary inits in the format: 
	 * 	<function1>,
	 *  <function2>,
	 *  ...
	 * 
	 * Make sure you keep the same order as in export!
	 * 
	 */
	cdp_avp_add_Subscription_Id_Group,
	cdp_avp_get_Subscription_Id_Group,

	cdp_avp_add_User_Equipment_Info_Group,
	cdp_avp_get_User_Equipment_Info_Group,
	
#elif defined(CDP_AVP_REFERENCE)
	/*
	 * Put here what you want to get in the reference. Typically:
	 * <function1>
	 * <function2>
	 * ... 
	 * 
	 */
int CDP_AVP_MODULE.add_Subscription_Id_Group(AAA_AVP_LIST *list,int32_t type,str data,AVPDataStatus data_do);
int CDP_AVP_MODULE.get_Subscription_Id_Group(AAA_AVP_LIST list,int32_t *type,str *data,AAA_AVP **avp_ptr);
		
int CDP_AVP_MODULE.add_User_Equipment_Info_Group(AAA_AVP_LIST *list,int32_t type,str data,AVPDataStatus data_do);
int CDP_AVP_MODULE.get_User_Equipment_Info_Group(AAA_AVP_LIST list,int32_t *type,str *data,AAA_AVP **avp_ptr);
	
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

	
	#ifndef _CDP_AVP_CCAPP_H_2
	#define _CDP_AVP_CCAPP_H_2

	
		int cdp_avp_add_Subscription_Id_Group(AAA_AVP_LIST *list,int32_t type, str data,AVPDataStatus data_do);
		typedef int (*cdp_avp_add_Subscription_Id_Group_f)(AAA_AVP_LIST *list,int32_t type, str data,AVPDataStatus data_do);

		int cdp_avp_get_Subscription_Id_Group(AAA_AVP_LIST list,int32_t *type,str *data,AAA_AVP **avp_ptr);
		typedef int (*cdp_avp_get_Subscription_Id_Group_f)(AAA_AVP_LIST list,int32_t *type,str *data,AAA_AVP **avp_ptr);
		

		int cdp_avp_add_User_Equipment_Info_Group(AAA_AVP_LIST *list,int32_t type, str data,AVPDataStatus data_do);
		typedef int (*cdp_avp_add_User_Equipment_Info_Group_f)(AAA_AVP_LIST *list,int32_t type, str data,AVPDataStatus data_do);

		int cdp_avp_get_User_Equipment_Info_Group(AAA_AVP_LIST list,int32_t *type,str *data,AAA_AVP **avp_ptr);
		typedef int (*cdp_avp_get_User_Equipment_Info_Group_f)(AAA_AVP_LIST list,int32_t *type,str *data,AAA_AVP **avp_ptr);
		
	#endif //_CDP_AVP_CCAPP_H_2
	
#endif



#define CDP_AVP_UNDEF_MACROS
	#include "macros.h"
#undef CDP_AVP_UNDEF_MACROS
	



