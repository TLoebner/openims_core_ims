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

#ifndef __CDF_Rf_data_H
#define __CDF_Rf_data_H

#include "../cdp/diameter.h"
#include "../cdp/diameter_epc.h"

#ifndef WHARF
/**
 * Allocate and blank a memory area
 */
#define mem_new(dst,len,mem) \
do {\
	if (!(len)) (dst)=0;\
	else {\
		(dst) = mem##_malloc(len);\
		if (!(dst)) {\
			LOG(L_ERR,"Error allocating %ld bytes in %s!\n",(long int)len,#mem);\
			goto out_of_memory;\
		}\
		bzero((dst),(len));\
	}\
} while(0)

#define struct_new(dst,type,mem) \
		men_new(dst,sizeof(type),mem)


/**
 * Duplicate an arbitrary memory area
 */
#define mem_dup(dst,src,len,mem) \
do {\
	if (!(src)||!(len)) (dst)=0;\
	else {\
		(dst) = mem##_malloc(len);\
		if (!(dst)) {\
			LOG(L_ERR,"Error allocating %ld bytes in %s!\n",(long int)len,#mem);\
			goto out_of_memory;\
		}\
		memcpy((dst),(src),len);\
	}\
} while(0)


/**
 * Free an arbitrary memory area
 */
#define mem_free(x,mem) \
do {\
	if (x) {\
		mem##_free(x);\
		x=0;\
	}\
} while(0)

/**
 * Duplicate a str, safely.
 * \Note This checks if:
 *  - src was an empty string
 *  - malloc failed
 * \Note On any error, the dst values are reset for safety
 * \Note A label "out_of_memory" must be defined in the calling function to handle
 * allocation errors. 
 * @param dst - destination str
 * @param src - source src
 * @param mem - type of mem to duplicate into (shm/pkg)
 */
#define str_dup(dst,src,mem) \
do {\
	if ((src).len) {\
		(dst).s = mem##_malloc((src).len);\
		if (!(dst).s){\
			LOG(L_ERR,"Error allocating %d bytes in %s!\n",(src).len,#mem);\
			(dst).len = 0;\
			goto out_of_memory;\
		}\
		memcpy((dst).s,(src).s,(src).len);\
		(dst).len = (src).len;\
	}else{\
		(dst).s=0;(dst).len=0;\
	}\
} while (0)


#define str_dup_ptr(dst,src,mem) \
do {\
	dst = mem##_malloc(sizeof(str));\
	if ((src).len) {\
		(dst)->s = mem##_malloc((src).len);\
		if (!(dst)->s){\
			LOG(L_ERR,"Error allocating %d bytes in %s!\n",(src).len,#mem);\
			mem##_free(dst);\
			goto out_of_memory;\
		}\
		memcpy((dst)->s,(src).s,(src).len);\
		(dst)->len = (src).len;\
	}else{\
		(dst)->s=0;(dst)->len=0;\
	}\
} while (0)

#define str_dup_ptr_ptr(dst,src,mem) \
do {\
	dst = mem##_malloc(sizeof(str));\
	if ((src) && (src)->len) {\
		(dst)->s = mem##_malloc((src)->len);\
		if (!(dst)->s){\
			LOG(L_ERR,"Error allocating %d bytes in %s!\n",(src)->len,#mem);\
			mem##_free(dst);\
			goto out_of_memory;\
		}\
		memcpy((dst)->s,(src)->s,(src)->len);\
		(dst)->len = (src)->len;\
	}else{\
		(dst)->s=0;(dst)->len=0;\
	}\
} while (0)

/**     
 * Duplicate a char* safely into a str
 * \Note This actually copies also the \0 at the end, unlike str_dup!
 * @param dst - destination str
 * @param src - source char*
 * @param mem - type of memory to use for malloc (pkg/shm) 
 */
#define char_dup_str(dst,src,mem) \
do {\
        int char_dup_len = strlen((char*)src);\
        if (!char_dup_len) {(dst).s=0;(dst).len=0;}\
        else {\
                (dst).s = mem##_malloc(char_dup_len+1);\
                if (!(dst).s){\
                        LOG(L_ERR,"Error allocating %d bytes in %s!\n",(char_dup_len)+1,#mem);\
                        (dst).len=0;\
                        goto out_of_memory;\
                }\
                memcpy((dst).s,(src),char_dup_len);\
                (dst).s[char_dup_len]=0;\
                (dst).len=char_dup_len;\
        }\
} while (0)

/**
 * Frees a str content.
 * @param x - the str to free
 * @param mem - type of memory that the content is using (shm/pkg)
 */
#define str_free(x,mem) \
do {\
	if ((x).s) mem##_free((x).s);\
	(x).s=0;(x).len=0;\
} while(0)

#define str_free_ptr(x,mem) \
do {\
	if (x) {\
		if ((x)->s) mem##_free((x)->s);\
		mem##_free((x));\
	}\
} while(0)

/**
 * list operations 
 */

#define WL_NEW(el,list_type,mem)                                                                                                \
do {                                                                                                                                                    \
        (el) = mem##_malloc(sizeof(struct _##list_type##_slot));                                        \
        if (!(el)) {                                                                                                                            \
                LOG(L_ERR,"Unable to allocate %ld bytes in %s!",(long int)sizeof(struct _##list_type##_slot),#mem);\
                goto out_of_memory;                                                                                                             \
        }                                                                                                                                                       \
        bzero((el),sizeof(struct _##list_type##_slot));                                                         \
} while(0)

#define WL_APPEND(list,add)                                                      \
do {                                                                             \
  (add)->next = NULL;								\
  (add)->prev = (list)->tail;													 \
  if ((list)->tail) ((list)->tail)->next=(add);									 \
  else (list)->head = (add);                                                     \
  (list)->tail=(add);                                                        	 \
} while (0)

#define WL_DELETE(list,del)                                                    	\
do {                                                                            \
	if ((del)->prev) ((del)->prev)->next=(del)->next;							\
	else ((list)->head)=(del)->next;											\
	if ((del)->next) ((del)->next)->prev=(del)->prev;							\
	else ((list)->tail)=(del)->prev;											\
	(del)->next=0;(del)->prev=0;												\
} while (0)

#define WL_FREE(el,list_type,mem)												\
	list_type##_free(el,mem)

#define WL_FREE_ALL(list,list_type,mem)\
do {\
	struct _##list_type##_slot *el,*nel;\
	for(el=(list)->head;el;el=nel){\
		nel = el->next;\
		WL_FREE(el,list_type,mem);\
	}\
	(list)->head=0;\
	(list)->tail=0;\
} while(0)

#define WL_FOREACH(list,el)                                                      \
    for(el=(list)->head;el;el=(el)->next)


/* List of str */
#define str_equal(a,b) ((a).len==(b).len && memcmp((a).s,(b).s,(a).len)==0)

typedef struct _str_list_t_slot {
	str data;
	struct _str_list_t_slot *prev,*next;
} str_list_slot_t;

typedef struct {
	str_list_slot_t *head,*tail;
} str_list_t;

#define str_list_t_free(x,mem) \
do{\
	if (x) {\
		str_free((x)->data,mem);\
		mem##_free(x);\
		(x)=0;\
	}\
}while(0)


#define str_list_t_copy(dst,src,mem) \
	str_dup((dst)->data,(src)->data,mem)

/* List of int */

typedef struct _int_list_t_slot {
	int data;
	struct _int_list_t_slot *prev,*next;
} int_list_slot_t;

typedef struct {
	int_list_slot_t *head,*tail;
} int_list_t;

#define int_list_t_free(x,mem) \
do {\
	if (x){\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

#define int_list_t_copy(dst,src,mem) \
	dst->data = src->data


#endif /* WHARF */

typedef struct {
	str *sip_method;
	str *event;
	uint32_t *expires;
} event_type_t;

typedef struct {
	time_t 		*sip_request_timestamp;
	uint32_t 	*sip_request_timestamp_fraction;
	time_t 		*sip_response_timestamp;
	uint32_t 	*sip_response_timestamp_fraction;
} time_stamps_t;

typedef struct {
	str *application_server;
	str_list_t application_provided_called_party_address;
} as_info_t;

typedef struct _as_info_list_t_slot {
	as_info_t info;
	struct _as_info_list_t_slot *next,*prev;
} as_info_list_element_t;

typedef struct {
	as_info_list_element_t *head,*tail;
} as_info_list_t;


#define as_info_list_t_free(x,mem) \
do{\
	str_list_slot_t *y=0,*z;\
	if (x) {\
		str_free_ptr((x)->info.application_server,mem);\
		for(y=(x)->info.application_provided_called_party_address.head;y;y=z){\
			z=y->next;\
			str_free(y->data,mem);\
			mem_free(y,mem);\
		}\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

#define as_info_list_t_copy(dst,src,mem) \
do {\
	str_dup_ptr_ptr((dst)->info.application_server,(src)->info.application_server,mem);\
	WL_DUP_ALL(&((dst)->info.application_provided_called_party_address),&((src)->info.application_provided_called_party_address),str_list_t,mem);\
} while(0)

typedef struct {
	str *originating_ioi;
	str *terminating_ioi;	
} ioi_t;

typedef struct _ioi_list_t_slot {
	ioi_t info;
	struct _ioi_list_t_slot *next,*prev;
} ioi_list_element_t;

typedef struct {
	ioi_list_element_t *head,*tail;
} ioi_list_t;


#define ioi_list_t_free(x,mem) \
do{\
	if (x) {\
		str_free_ptr((x)->info.originating_ioi,mem);\
		str_free_ptr((x)->info.terminating_ioi,mem);\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

#define ioi_list_t_copy(dst,src,mem) \
do {\
	str_dup_ptr_ptr((dst)->info.originating_ioi,(src)->info.originating_ioi,mem);\
	str_dup_ptr_ptr((dst)->info.terminating_ioi,(src)->info.terminating_ioi,mem);\
} while(0)

	
typedef struct {
	str *data;
	uint32_t *type;	
} service_specific_info_t;

typedef struct _service_specific_info_list_t_slot {
	service_specific_info_t info;
	struct _service_specific_info_list_t_slot *next,*prev;
} service_specific_info_list_element_t;

typedef struct {
	service_specific_info_list_element_t *head,*tail;
} service_specific_info_list_t;


#define service_specific_info_list_t_free(x,mem) \
do{\
	if (x) {\
		str_free_ptr((x)->info.data,mem);\
		mem_free((x)->info.type,mem);\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

#define service_specific_info_list_t_copy(dst,src,mem) \
do {\
	str_dup_ptr_ptr((dst)->info.data,(src)->info.data,mem);\
	mem_dup((dst)->info.type,(src)->info.type,sizeof(uint32_t),mem);\
} while(0)

typedef struct _flow_list_t_slot {
	uint32_t media_component_number;
	int_list_t flow_number;	
	int32_t *final_unit_action;

	struct _flow_list_t_slot *next,*prev;
} flow_t;

typedef struct {
	flow_t *head,*tail;
} flow_list_t;


#define flow_list_t_free(x,mem) \
do{\
	if (x) {\
		int_list_slot_t *i,*in;\
		for(i=(x)->flow_number.head;i;i=in){\
			in = i->next;\
			mem_free(i,mem);\
		}\
		mem_free((x)->final_unit_action,mem);\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

/*
#define flow_list_t_copy(dst,src,mem) \
do {\
	str_dup_ptr_ptr((dst)->info.data,(src)->info.data,mem);\
	mem_dup((dst)->info.type,(src)->info.type,sizeof(uint32_t),mem);\
} while(0)
*/


typedef struct {
	str value;
	flow_list_t flow;
} an_charging_id_t;

#define an_charging_id_t_free(x,mem) \
do{\
	if (x) {\
		str_free((x)->value,mem);\
		flow_t *f,*fn;\
		for(f=(x)->flow.head;f;f=fn){\
			fn = f->next;\
			flow_list_t_free(f,mem);\
		}\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)


typedef struct _sdp_media_component_list_t_slot {
	str *sdp_media_name;
	str_list_t sdp_media_descriptions;
	int32_t *media_initiator_flag;
	str	*media_initiator_party;
	str *authorized_qos;
	str *tgpp_charging_id;
	an_charging_id_t *an_charging_id;
	int32_t *sdp_type;
	
	struct _sdp_media_component_list_t_slot *next,*prev;
} sdp_media_component_t;

typedef struct {
	sdp_media_component_t *head,*tail;
} sdp_media_component_list_t;

#define sdp_media_component_list_t_free(x,mem) \
do{\
	if (x) {\
		str_free_ptr((x)->sdp_media_name,mem);\
		mem_free((x)->media_initiator_flag,mem);\
		str_free_ptr((x)->media_initiator_party,mem);\
		str_free_ptr((x)->authorized_qos,mem);\
		str_free_ptr((x)->tgpp_charging_id,mem);\
		an_charging_id_t_free((x)->an_charging_id,mem);\
		mem_free((x)->sdp_type,mem);\
		str_list_slot_t *ls,*lsn;\
		for(ls = (x)->sdp_media_descriptions.head;ls;ls=lsn){\
		     lsn = ls->next;\
		     str_list_t_free(ls,mem);\
		}\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

/*
#define sdp_media_component_list_t_copy(dst,src,mem) \
do {\
	str_dup_ptr_ptr((dst)->info.data,(src)->info.data,mem);\
	mem_dup((dst)->info.type,(src)->info.type,sizeof(uint32_t),mem);\
} while(0)
*/
	

typedef struct _message_body_list_t_slot {
	str content_type;
	uint32_t content_length;	
	str	*content_disposition;
	int32_t *originator;
	
	struct _message_body_list_t_slot *next,*prev;
} message_body_t;

typedef struct {
	message_body_t *head,*tail;
} message_body_list_t;


#define message_body_list_t_free(x,mem) \
do{\
	if (x) {\
		str_free((x)->content_type,mem);\
		str_free_ptr((x)->content_disposition,mem);\
		mem_free((x)->originator,mem);\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

/*
#define message_body_list_t_copy(dst,src,mem) \
do {\
	str_dup_ptr_ptr((dst)->info.data,(src)->info.data,mem);\
	mem_dup((dst)->info.type,(src)->info.type,sizeof(uint32_t),mem);\
} while(0)
*/

typedef struct {
	time_t *sdp_offer_timestamp;
	time_t *sdp_answer_timestamp;
} sdp_timestamps_t;

#define sdp_timestamps_t_free(x,mem) \
do{\
	if (x) {\
		mem_free((x)->sdp_offer_timestamp,mem);\
		mem_free((x)->sdp_answer_timestamp,mem);\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)


typedef struct _early_media_description_list_t_slot {
	sdp_timestamps_t *sdp_timestamps;
	sdp_media_component_list_t sdp_media_component;
	str_list_t sdp_session_description;
	
	struct _early_media_description_list_t_slot *next,*prev;
} early_media_description_t;

typedef struct {
	early_media_description_t *head,*tail;
} early_media_description_list_t;


#define early_media_description_list_t_free(x,mem) \
do{\
	if (x) {\
		sdp_timestamps_t_free((x)->sdp_timestamps,mem);\
		sdp_media_component_t *s,*sn;\
		for(s = (x)->sdp_media_component.head;s;s=sn){\
			sn = s->next;\
			sdp_media_component_list_t_free(s,mem);\
		}\
		str_list_slot_t *ls,*lsn;\
		for(ls = (x)->sdp_session_description.head;ls;ls=lsn){\
			lsn = ls->next;\
			str_list_t_free(ls,mem);\
		}\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

/*
#define early_media_description_list_t_copy(dst,src,mem) \
do {\
	str_dup_ptr_ptr((dst)->info.data,(src)->info.data,mem);\
	mem_dup((dst)->info.type,(src)->info.type,sizeof(uint32_t),mem);\
} while(0)
*/

	
typedef struct {
	event_type_t *event_type;
	
	int32_t *role_of_node;
	int32_t node_functionality;
	
	str *user_session_id;
	str *outgoing_session_id;
	
	str_list_t calling_party_address;
	str *called_party_address;
	str_list_t called_asserted_identity;	
	str * requested_party_address;
	
	time_stamps_t *time_stamps;
	
	as_info_list_t as_info;
	
	ioi_list_t ioi;	
	str *icid;

	str_list_t sdp_session_description;
	sdp_media_component_list_t sdp_media_component;
	
	str *service_id;
	
	service_specific_info_list_t service_specific_info;
	
	int32_t *cause_code;
	
} ims_information_t;

typedef enum {
	Subscription_Type_MSISDN	= AVP_EPC_Subscription_Id_Type_End_User_E164,
	Subscription_Type_IMSI		= AVP_EPC_Subscription_Id_Type_End_User_IMSI,
	Subscription_Type_IMPU		= AVP_EPC_Subscription_Id_Type_End_User_SIP_URI,
	Subscription_Type_NAI		= AVP_EPC_Subscription_Id_Type_End_User_NAI,	
	Subscription_Type_IMPI 		= AVP_EPC_Subscription_Id_Type_End_User_Private,	
} subscription_id_type_e;

typedef struct _subscription_id_t {
	int32_t type;
	str id;
} subscription_id_t;

typedef struct _subscription_id_list_t_slot {
	subscription_id_t s;
	struct _subscription_id_list_t_slot *next,*prev;
} subscription_id_list_element_t;

typedef struct {
	subscription_id_list_element_t *head,*tail;
} subscription_id_list_t;


#define subscription_id_list_t_free(x,mem) \
do{\
	if (x) {\
		str_free((x)->s.id,mem);\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

#define subscription_id_list_t_copy(dst,src,mem) \
do {\
	(dst)->type = (src)->type;\
	str_dup((dst)->id,(src)->id,mem);\
} while(0)

typedef struct{
	uint32_t rating_group;
} service_data_container_t;

typedef struct{
	str * tgpp_charging_id;
	str * sgsn_address;
	service_data_container_t * service_data_container;
} ps_information_t;

typedef struct {
	subscription_id_list_t subscription_id;
	ps_information_t * ps_information;
	ims_information_t *ims_information;
	
} service_information_t;

typedef struct {
	str origin_host;
	str origin_realm;
	str destination_realm;
	
	int32_t acct_record_type;
	uint32_t acct_record_number;
	
	str *user_name;
	uint32_t *acct_interim_interval;
	uint32_t *origin_state_id;
	time_t *event_timestamp;
	
	str *service_context_id;
	
	service_information_t *service_information;
} Rf_ACR_t;

typedef struct {
	uint32_t result_code;
		
	int32_t acct_record_type;
	uint32_t acct_record_number;
	
	str *user_name;
	uint32_t *acct_interim_interval;
	uint32_t *origin_state_id;
	time_t *event_timestamp;	
} Rf_ACA_t;


event_type_t * new_event_type(str * sip_method,
				str * event,
				uint32_t * expires);

time_stamps_t * new_time_stamps(time_t	*sip_request_timestamp,
		uint32_t *sip_request_timestamp_fraction,
		time_t 	*sip_response_timestamp,
		uint32_t *sip_response_timestamp_fraction);

ims_information_t * new_ims_information(event_type_t * event_type,
					time_stamps_t * time_stamps,
					str * user_session_id, 
					str * outgoing_session_id,
					str * calling_party,
					str * called_party,
					str * icid,
					str * orig_ioi,
					str * term_ioi,
					int node_role);

service_data_container_t * new_service_data_container(uint32_t rating_group);
ps_information_t * new_ps_information(service_data_container_t * serv_data_container);

void event_type_free(event_type_t *x);
void time_stamps_free(time_stamps_t *x);
void ims_information_free(ims_information_t *x);
void service_data_container_free(service_data_container_t *x);
void ps_information_free(ps_information_t *x);
void service_information_free(service_information_t *x);



Rf_ACR_t * new_Rf_ACR(int32_t acct_record_type, uint32_t acct_record_number,
		str * user_name, ims_information_t * ims_info,
		ps_information_t * ps_info, subscription_id_t * subscription);
void Rf_free_ACR(Rf_ACR_t *x);
//void Rf_free_ACA(Rf_ACA_t *x);

typedef struct _acct_record_info_list_t_slot{
	str id;
	uint32_t acct_record_number;
	int dir;
	time_t expires;
	struct _acct_record_info_list_t_slot * next, * prev;
} acct_record_info_list_slot_t;

typedef struct _acct_record_info_list_t{
	gen_lock_t * lock;
	struct _acct_record_info_list_t_slot * head, * tail;
} acct_record_info_list_t;

#define acct_record_info_list_t_free(x,mem) \
do{\
	if (x) {\
		str_free((x)->id,mem);\
		mem##_free(x);\
		(x) = 0;\
	}\
}while(0)

int init_acct_records();
void destroy_acct_records();
int get_subseq_acct_record_nb(str id, int32_t acct_record_type, uint32_t * value, int dir, uint32_t expires);

#endif /* __CDF_Rf_data_H */
