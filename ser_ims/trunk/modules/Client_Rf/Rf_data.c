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
 * Client_Rf module - Rf Data structures
 * 
 * 
 *  \author Dragos Vingarzan dragos dot vingarzan -at- fokus dot fraunhofer dot de
 * 
 */ 


#include "Rf_data.h"

void event_type_free(event_type_t *x)
{
	if (!x) return;
	str_free_ptr(x->sip_method,pkg);
	str_free_ptr(x->event,pkg);
	mem_free(x->expires,pkg);
	mem_free(x,pkg);	
}

void time_stamps_free(time_stamps_t *x)
{
	if (!x) return;
	mem_free(x->sip_request_timestamp,pkg);
	mem_free(x->sip_request_timestamp_fraction,pkg);
	mem_free(x->sip_response_timestamp,pkg);
	mem_free(x->sip_response_timestamp_fraction,pkg);
	mem_free(x,pkg);	
}

void ims_information_free(ims_information_t *x)
{
	if (!x) return;

	event_type_free(x->event_type);
	
	mem_free(x->role_of_node,pkg);
	str_free_ptr(x->user_session_id,pkg);
	str_free_ptr(x->outgoing_session_id,pkg);
	
	WL_FREE_ALL(&(x->calling_party_address),str_list_t,pkg);
	str_free_ptr(x->called_party_address,pkg);
	WL_FREE_ALL(&(x->called_asserted_identity),str_list_t,pkg);
	str_free_ptr(x->requested_party_address,pkg);
	
	time_stamps_free(x->time_stamps);
	
	WL_FREE_ALL(&(x->as_info),as_info_list_t,pkg);
	
	WL_FREE_ALL(&(x->ioi),ioi_list_t,pkg);
	str_free_ptr(x->icid,pkg);
	
	str_free_ptr(x->service_id,pkg);
	
	WL_FREE_ALL(&(x->service_specific_info),service_specific_info_list_t,pkg);
	
	mem_free(x->cause_code,pkg);
	
	mem_free(x,pkg);	
}

void service_information_free(service_information_t *x)
{
	if (!x) return;

	//WL_FREE_ALL(&(x->subscription_id),subscription_id_list_t,pkg);
	ims_information_free(x->ims_information);
	
	mem_free(x,pkg);
}

void Rf_free_ACR(Rf_ACR_t *x)
{
	if (!x) return;

	str_free(x->origin_host,pkg);
	str_free(x->origin_realm,pkg);
	str_free(x->destination_realm,pkg);
	
	str_free_ptr(x->user_name,pkg);
	mem_free(x->acct_interim_interval,pkg);
	mem_free(x->origin_state_id,pkg);
	mem_free(x->event_timestamp,pkg);
	
	str_free_ptr(x->service_context_id,pkg);
	
	service_information_free(x->service_information);
	
	mem_free(x,pkg);
}


void Rf_free_ACA(Rf_ACA_t *x)
{
	if (!x) return;
	
	str_free_ptr(x->user_name,pkg);
	mem_free(x->acct_interim_interval,pkg);
	mem_free(x->origin_state_id,pkg);
	mem_free(x->event_timestamp,pkg);	
	
	mem_free(x,pkg);
}

Rf_ACA_t* Rf_new_ACA_from_ACR(Rf_ACR_t *r)
{
	Rf_ACA_t *a=0;
	mem_new(a,sizeof(Rf_ACA_t),pkg);
	
	a->acct_record_type = r->acct_record_type;
	a->acct_record_number = r->acct_record_number;
	str_dup_ptr_ptr(a->user_name,r->user_name,pkg);
	mem_dup(a->acct_interim_interval,r->acct_interim_interval,sizeof(uint32_t),pkg);
	mem_dup(a->origin_state_id,r->origin_state_id,sizeof(uint32_t),pkg);
	mem_dup(a->event_timestamp,r->event_timestamp,sizeof(uint32_t),pkg);

	return a;
out_of_memory:
	Rf_free_ACA(a);
	return 0;
}

