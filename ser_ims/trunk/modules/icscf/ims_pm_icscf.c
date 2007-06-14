/*
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
 * \file  ims_pm_icscf.c
 * 
 *	I-CSCF IMS Performance Management
 * 
 * Scope: logs raw data for computing metrics as in TS 32.409
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#include "ims_pm_icscf.h"

#ifdef WITH_IMS_PM

#include <unistd.h>
#include <fcntl.h>
#include <time.h>


#include "../../script_cb.h"
#include "../tm/tm_load.h"

#include "sip.h"
#include "cx_avp.h"


extern struct tm_binds tmb;							/**< Structure with pointers to tm funcs 				*/

static str zero={0,0};

void ims_pm_init_icscf()
{
	register_script_cb(ims_pm_pre_script,PRE_SCRIPT_CB|REQ_TYPE_CB|RPL_TYPE_CB,0);
	register_script_cb(ims_pm_post_script,POST_SCRIPT_CB|REQ_TYPE_CB|RPL_TYPE_CB,0);	
}


int ims_pm_pre_script(struct sip_msg *msg,void *param)
{
	str method={0,0};

	if (msg->first_line.type == SIP_REQUEST){
		/* REGISTER */
		method = msg->first_line.u.request.method;
		
	}else{
//		unsigned int code = msg->first_line.u.reply.statuscode;
		method = cscf_get_cseq_method(msg,0);
				
	}
	return 1;
}



int ims_pm_post_script(struct sip_msg *msg,void *param)
{
	return 1;
}


void ims_pm_diameter_request(AAAMessage *msg)
{	
	if (!msg) return;
	switch(msg->applicationId){
    	case IMS_Cx:
			switch(msg->commandCode){				
				case IMS_UAR:														
					IMS_PM_LOG11(UR_AttUAR,Cx_get_session_id(msg),msg->endtoendId);
					return ;
					break;
			}
	}	
}

void ims_pm_diameter_answer(AAAMessage *msg)
{
	if (!msg) return;
	int code=-1;
	if (!Cx_get_result_code(msg,&code)) 
		Cx_get_experimental_result_code(msg,&code);
	switch(msg->applicationId){
    	case IMS_Cx:
			switch(msg->commandCode){				
				case IMS_UAA:
					if (code>=2000 && code<3000) IMS_PM_LOG12(UR_SuccUAA,Cx_get_session_id(msg),msg->endtoendId,code);
					else IMS_PM_LOG12(UR_FailUAA,Cx_get_session_id(msg),msg->endtoendId,code);
					break;
			}
	}
}

#endif /* WITH_IMS_PM */
