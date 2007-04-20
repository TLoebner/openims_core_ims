/*
 * acct.h, acct.c provides the accounting portion of Diameter based 
 * protocol.
 * 
 * \author Shengyao Chen shc -at- fokus dot fraunhofer dot de
 */
#include "acct.h"

//extern struct tm_binds tmb;          /**< Structure with pointers to tm funcs 		*/

//static inline int acct_add_avp(AAAMessage *m,char *d,int len,int avp_code,
//	int flags,int vendorid,int data_do,const char *func)
//{
//	AAA_AVP *avp;
//	if (vendorid!=0) flags |= AAA_AVP_FLAG_VENDOR_SPECIFIC;
//	avp = cdpb.AAACreateAVP(avp_code,flags,vendorid,d,len,data_do);
//	if (!avp) {
//		//LOG(L_ERR,"ERR:"M_NAME":%s: Failed creating avp\n",func);
//		return 0;
//	}
//	if (cdpb.AAAAddAVPToMessage(m,avp,m->avpList.tail)!=AAA_ERR_SUCCESS) {
//		//LOG(L_ERR,"ERR:"M_NAME":%s: Failed adding avp to message\n",func);
//		cdpb.AAAFreeAVP(&avp);
//		return 0;
//	}
//	return 1;
//}
//
///* add Accounting-Record-Type AVP */
//int acct_add_accounting_record_type(AAAMessage* msg, unsigned int data)
//{
//	char x[4];
//	set_4bytes(x,data);
//	
//	return
//	acct_add_avp(msg, x, 4,
//		AVP_Accounting_Record_Type,
//		AAA_AVP_FLAG_MANDATORY,
//		0,
//		AVP_DUPLICATE_DATA,
//		__FUNCTION__);
//}
//
///* add Accounting-Record-Number AVP */
//int acct_add_accounting_record_number(AAAMessage* msg, unsigned int data)
//{
//	char x[4];
//	set_4bytes(x,data);
//	
//	return
//	acct_add_avp(msg, x, 4,
//		AVP_Accounting_Record_Number,
//		AAA_AVP_FLAG_MANDATORY,
//		0,
//		AVP_DUPLICATE_DATA,
//		__FUNCTION__);
//}

/**
 * Create an ACR based on sessionId and accounting type. 
 * 
 * @param type 	- accounting type: start, interim, stop, event  
 * @returns the created ACR message, if error return NULL
 */
//AAAMessage* ACR_create(AAASessionId sessId, unsigned int type, struct cdp_binds* cdpb)
//{
//	AAAMessage* acr = NULL;
//	
//	AAASessionId sid = {0,0};
//	/*
//	 * if type == event 
//	 */
//	if (type == AAA_ACCT_EVENT) {
//	 	sessId = cdpb->AAACreateSession();
//		//acr = cdpb.AAACreateRequest(IMS_Rf, ACR, Flag_Proxyable, &sessId);
//		/* orgin host realm, dest host realm are added by peer */ 
//		//if (!acct_add_accounting_record_type(acr, type)) goto error;
//		//if (!acct_add_accounting_record_number(acr, 0)) goto error;
//	 }
//	 return acr;
//
//	/* if type == start, create a new accounting session. */
//	/* if type == interim, sid cannot be NULL, new ACR is created based
//	 * 		      on this session.
//	 * if type == stop, sid cannot be NULL, new ACR is created based on 
//	 * 			  this session, and the session is closed. 
//	 */
//	
//	
//
//	// TODO not forget to call this line: cdpb.AAADropSession(&sessId);
//error: return NULL; 	
//}
	
