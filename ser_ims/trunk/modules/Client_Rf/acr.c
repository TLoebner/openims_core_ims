/*
 * $Id$
 * 
 * Copyright (C) 2009 FhG Fokus
 * 
 * This file is part of the Wharf project.
 * 
 */

/**
 * \file
 * 
 * Client_Rf module - User Data Request Logic - Rf-Pull
 * 
 * 
 *  \author Andreea Onofrei Corici andreea dot ancuta dot corici -at- fokus dot fraunhofer dot de
 * 
 */  
 
#include "../cdp_avp/mod_export.h"

#include "acr.h"
#include "Rf_data.h"

extern cdp_avp_bind_t *cavpb;

int Rf_write_event_type_avps(AAA_AVP_LIST * avp_list, event_type_t * x)
{
	if (x->sip_method)
		if(!cavpb->epcapp.add_SIP_Method(avp_list, *(x->sip_method),0))
			goto error;
	if (x->event)
		if (!cavpb->epcapp.add_Event(avp_list,*(x->event),0))
			goto error;	
	if (x->expires)
		if (!cavpb->epcapp.add_Expires(avp_list, *(x->expires)))
			goto error;	

	return 1;
error:	
	return 0;
}

int Rf_write_time_stamps_avps(AAA_AVP_LIST * avp_list,  time_stamps_t* x)
{

	if (x->sip_request_timestamp)	
		if (!cavpb->epcapp.add_SIP_Request_Timestamp(avp_list, *(x->sip_request_timestamp))) 
			goto error;
	
	if (x->sip_request_timestamp_fraction)	
		if (!cavpb->epcapp.add_SIP_Request_Timestamp_Fraction(avp_list, 
					*(x->sip_request_timestamp_fraction))) 
			goto error;

	if (x->sip_response_timestamp)	
		if (!cavpb->epcapp.add_SIP_Response_Timestamp(avp_list, *(x->sip_response_timestamp))) 
			goto error;

	if (x->sip_response_timestamp_fraction)	
		if (!cavpb->epcapp.add_SIP_Response_Timestamp_Fraction(avp_list, 
					*(x->sip_response_timestamp_fraction))) 
			goto error;

	return 1;
error:
	return 0;
}

int Rf_write_ims_information_avps(AAA_AVP_LIST * avp_list, ims_information_t* x)
{
	
	if (x->event_type)
		if(!Rf_write_event_type_avps(avp_list, x->event_type))
			goto error;

	if (x->role_of_node)
		if (!cavpb->epcapp.add_Role_Of_Node(avp_list, *x->role_of_node)) goto error;

	if (!cavpb->epcapp.add_Node_Functionality(avp_list, x->node_functionality))
			goto error;

	if(x->user_session_id)
		if (!cavpb->epcapp.add_User_Session_Id(avp_list,*(x->user_session_id),0)) 
			goto error;
	if(x->outgoing_session_id)
		if (cavpb->epcapp.add_Outgoing_Session_Id(avp_list,*(x->outgoing_session_id),0))	
			goto error;

/*	avp=0;
	while(cavpb->epcapp.add_Calling_Party_Address(avp_list,&s,&avp)){
		WL_NEW(sl,str_list_t,pkg);		
		str_dup(sl->data,s,pkg);
		WL_APPEND(&(x->calling_party_address),sl);
	}
	if (cavpb->epcapp.add_Called_Party_Address(avp_list,&s,0)) str_dup_ptr(x->called_party_address,s,pkg);	
	avp=0;
	while(cavpb->epcapp.add_Called_Asserted_Identity(avp_list,&s,&avp)){
		WL_NEW(sl,str_list_t,pkg);		
		str_dup(sl->data,s,pkg);
		WL_APPEND(&(x->called_asserted_identity),sl);
	}
	if (cavpb->epcapp.add_Requested_Party_Address(avp_list,&s,0)) str_dup_ptr(x->requested_party_address,s,pkg);	
*/

	if (x->time_stamps)
		if(!Rf_write_time_stamps_avps(avp_list, x->time_stamps))
			goto error;
	
/*	avp=0;
	while(cavpb->epcapp.add_Inter_Operator_Identifier(avp_list,&ioi_list,&avp)){
		WL_NEW(ioi,ioi_list_t,pkg);		
		if (cavpb->epcapp.add_Originating_IOI(ioi_list,&s,0)) str_dup_ptr(ioi->info.originating_ioi,s,pkg);
		if (cavpb->epcapp.add_Terminating_IOI(ioi_list,&s,0)) str_dup_ptr(ioi->info.terminating_ioi,s,pkg);
		WL_APPEND(&(x->ioi),ioi);
		cavpb->data.free_Grouped(&ioi_list);
	}
	if (cavpb->epcapp.add_IMS_Charging_Identifier(avp_list,&s,0)) str_dup_ptr(x->icid,s,pkg);	
	
	if (cavpb->epcapp.add_Service_ID(avp_list,&s,0)) str_dup_ptr(x->service_id,s,pkg);

	avp=0;
	while(cavpb->epcapp.add_Service_Specific_Info(avp_list,&ss_info_list,&avp)){
		WL_NEW(ss_info,service_specific_info_list_t,pkg);		
		if (cavpb->epcapp.add_Service_Specific_Data(ss_info_list,&s,0)) str_dup_ptr(ss_info->info.data,s,pkg);
		if (cavpb->epcapp.add_Service_Specific_Type(ss_info_list,&u32,0)) {
			mem_new(ss_info->info.type,sizeof(uint32_t),pkg);
			*(ss_info->info.type) = u32;
		}
		WL_APPEND(&(x->service_specific_info),ss_info);
		cavpb->data.free_Grouped(&ss_info_list);
	}	 
	
	if (cavpb->epcapp.add_Cause_Code(avp_list,&i32,0)) {
		mem_new(x->cause_code,sizeof(int32_t),pkg);
		*(x->cause_code) = i32;
	}
*/	
	return 1;
error:
	return 0;
}

int Rf_write_service_information_avps(AAA_AVP_LIST * avp_list, service_information_t* x)
{
/*	while(cavpb->ccapp.add_Subscription_Id_Group(avp_list,&type,&id,&avp)){
		WL_NEW(subs_id,subscription_id_list_t,pkg);
		subs_id->s.type = type;
		str_dup(subs_id->s.id,id,pkg);
		WL_APPEND(&(x->subscription_id),subs_id);
	}
*/	
	if (x->ims_information)
		if(!Rf_write_ims_information_avps(avp_list, x->ims_information))
			goto error;

	return 1;
error:
	return 0;
}

AAAMessage * Rf_write_ACR_avps(AAAMessage * acr, Rf_ACR_t* x)
{
	
	if (!acr) return 0;

	if (!cavpb->base.add_Origin_Host(&(acr->avpList),x->origin_host,0)) goto error;

	if (!cavpb->base.add_Origin_Realm(&(acr->avpList),x->origin_realm,0)) goto error;
	if (!cavpb->base.add_Destination_Realm(&(acr->avpList),x->destination_realm,0)) goto error;

	if (!cavpb->base.add_Accounting_Record_Type(&(acr->avpList),x->acct_record_type)) goto error;
	if (!cavpb->base.add_Accounting_Record_Number(&(acr->avpList),x->acct_record_number)) goto error;
	
	if (x->user_name)
		if (!cavpb->base.add_User_Name(&(acr->avpList),*(x->user_name),AVP_DUPLICATE_DATA)) goto error;
	
	if (x->acct_interim_interval)
		if (!cavpb->base.add_Acct_Interim_Interval(&(acr->avpList),*(x->acct_interim_interval))) goto error;
	
	if (x->origin_state_id)
		if (!cavpb->base.add_Origin_State_Id(&(acr->avpList),*(x->origin_state_id))) goto error;
	
	if (x->event_timestamp)
		if (!cavpb->base.add_Event_Timestamp(&(acr->avpList),*(x->event_timestamp))) goto error;	
		
	if (x->service_context_id)
		if (!cavpb->ccapp.add_Service_Context_Id(&(acr->avpList),*(x->service_context_id),0)) goto error;
	
	if (x->service_information)
		if(!Rf_write_service_information_avps(&(acr->avpList), x->service_information))
			goto error;
	
	return acr;
error:
	cavpb->cdp->AAAFreeMessage(&acr);
	return 0;
}

AAAMessage *Rf_new_acr(AAASession * session, Rf_ACR_t * rf_data){

	AAAMessage * acr = 0;
	acr = cavpb->cdp->AAACreateRequest(IMS_Rf, Diameter_ACR, Flag_Proxyable, session);
	if(!acr){
		LOG(L_ERR, "could not create ACR\n");
		return 0;
	}

	acr = Rf_write_ACR_avps(acr, rf_data);

	return acr;
}


/*int Rf_read_ACA_avps(AAAMessage *aca, Rf_ACA_t *x)
{
	if (!x) return 0;
	
	if (!cavpb->base.add_Result_Code(&(aca->avpList),x->result_code)) goto error;
	
	if (!cavpb->base.add_Accounting_Record_Type(&(aca->avpList),x->acct_record_type)) goto error;
	if (!cavpb->base.add_Accounting_Record_Number(&(aca->avpList),x->acct_record_number)) goto error;
	
	if (x->user_name)
		if (!cavpb->base.add_User_Name(&(aca->avpList),*(x->user_name),AVP_DUPLICATE_DATA)) goto error;
	
	if (x->acct_interim_interval)
		if (!cavpb->base.add_Acct_Interim_Interval(&(aca->avpList),*(x->acct_interim_interval))) goto error;
	
	if (x->origin_state_id)
		if (!cavpb->base.add_Origin_State_Id(&(aca->avpList),*(x->origin_state_id))) goto error;
	
	if (x->event_timestamp)
		if (!cavpb->base.add_Event_Timestamp(&(aca->avpList),*(x->event_timestamp))) goto error;
	
	return 1;
error:
	return 1;
}
*/
