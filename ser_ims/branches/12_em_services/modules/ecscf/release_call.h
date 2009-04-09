/**
 * \file release_call.h
 * 
 *	E-CSCF initiated call release (for confirmed dialogs and QoS relevant cases)
 * 
 *  \author Alberto Diez     albertowhiterabbit at yahoo dot es
 *  \author Ancuta Onofrei   ancuta_onofrei at yahoo dot com
 */


#ifndef RELEASE_CALL_H_
#define RELEASE_CALL_H_
#include "../tm/tm_load.h"
#include "dlg_state.h"
#include "../dialog/dlg_mod.h"
#include "sip.h"


enum release_call_situation{
	RELEASE_CALL_EARLY=0,
	RELEASE_CALL_WEIRD=1
	 /*Weird state is the technical name of the state in which a
	  * sip session is when the callee has already sent a 200 OK for INVITE
	  * and the caller hasn't yet recieved this response
	  * In Weird state the session can only be released by sending an ACK followed
	  * by a  BYE to the callee and a reply >400 to the caller
	  * a CANCEL wouldn't be understood by the callee!*/
};
#define MAX_TIMES_TO_TRY_TO_RELEASE 5


int E_release_call_onreply(struct sip_msg *msg,char *str1,char *str2);

int release_call(str callid,int reason_code,str reason_text);

int release_call_e(e_dialog *d,int reason_code,str reason_text);


#endif /*RELEASE_CALL_H_*/
