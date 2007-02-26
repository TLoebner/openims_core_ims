/*
 * $id$ server.c $date $author$ Dragos Vingarzan dvi vingarzan@gmail.com
 *
 * Copyright (C) 2005 Fhg Fokus
 *
 */

#include "server.h"

#include "cdp/receiver.h"
#include "cdp/peerstatemachine.h"
#include "cdp/diameter_api.h"


AAAMessage *send_unknown_request_answer(AAAMessage *req)
{
        AAAMessage *ans=0;
        char x[4];
        AAA_AVP *avp;


        /* UAA header is created based on the UAR */
        ans = AAANewMessage(req->commandCode,req->applicationId,0,req);

        if (!ans) return 0;

        set_4bytes(x,(unsigned int) DIAMETER_UNABLE_TO_COMPLY);

        avp = AAACreateAVP(AVP_Result_Code,AAA_AVP_FLAG_MANDATORY,0,x,4,AVP_DUPLICATE_DATA);
        if (!avp) {
                LOG(L_ERR,"ERR: Failed creating avp for result code\n");
                AAAFreeMessage(&ans);
                return 0;
        }
        if (AAAAddAVPToMessage(ans,avp,ans->avpList.tail)!=AAA_ERR_SUCCESS) {
                LOG(L_ERR,"ERR: Failed adding avp to message\n");
                AAAFreeAVP(&avp);
                AAAFreeMessage(&ans);
                return 0;
        }
        return ans;
}



int process_incoming(peer *p,AAAMessage *msg,void* ptr)
{
	AAAMessage *ans=0;

	switch(msg->applicationId){
		default:
	        LOG(L_ERR,"process_incoming(): Received unserviced AppID [%d]\n",msg->applicationId);
               ans = send_unknown_request_answer(msg);               	
	}
	return 1;
}
