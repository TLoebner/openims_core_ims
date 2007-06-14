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
 * \file  ims_pm_scscf.c
 * 
 *	P-CSCF IMS Performance Management
 * 
 * Scope: logs raw data for computing metrics as in TS 32.409
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include "ims_pm_pcscf.h"

#ifdef WITH_IMS_PM

#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "../../script_cb.h"
#include "../tm/tm_load.h"

#include "mod.h"
#include "sip.h"
#include "registration.h"


extern struct tm_binds tmb;							/**< Structure with pointers to tm funcs 				*/

static str zero={0,0};

void ims_pm_init_pcscf()
{
	register_script_cb(ims_pm_pre_script,PRE_SCRIPT_CB|REQ_TYPE_CB|RPL_TYPE_CB,0);
	register_script_cb(ims_pm_post_script,POST_SCRIPT_CB|REQ_TYPE_CB|RPL_TYPE_CB,0);	
}

static str s_register={"REGISTER",8};

int ims_pm_get_registration_type(struct sip_msg *msg)
{
	if (P_is_registered(msg,0,0)==CSCF_RETURN_TRUE){
		if (cscf_get_expires_hdr(msg)>0) return 1;
		else return 2; 
	}else return 0;
}

void ims_pm_register_cb(struct cell* t, int type, struct tmcb_params* ps)
{
	int k = (int) *ps->param;
	int code = ps->code; 
	switch(k){
		case 0:
			if (code>=200 && code<300) 
				IMS_PM_LOG12(UR_SuccInitReg,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
			else if (code>=300) IMS_PM_LOG12(UR_FailInitReg,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
			break;
		case 1:
			if (code>=200 && code<300) IMS_PM_LOG12(UR_SuccReReg,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
			else if (code>=300) IMS_PM_LOG12(UR_FailReReg,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
			break;
		case 2:
			if (code>=200 && code<300) IMS_PM_LOG12(UR_SuccDeRegUe,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
			else if (code>=300) IMS_PM_LOG12(UR_FailDeRegUe,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
			break;
	}
}

int ims_pm_pre_script(struct sip_msg *msg,void *param)
{
	int k;
	str method={0,0};
	unsigned int x,y;
	if (msg->first_line.type == SIP_REQUEST){
		/* REGISTER */
		method = msg->first_line.u.request.method;
		if (method.len==s_register.len && strncasecmp(method.s,s_register.s,s_register.len)==0){
				k = ims_pm_get_registration_type(msg);
				switch(k){
					case 0:
						IMS_PM_LOG11(UR_AttInitReg,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0));
						break;
					case 1:
						IMS_PM_LOG11(UR_AttReReg,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0));
						break;
					case 2:
						IMS_PM_LOG11(UR_AttDeRegUe,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0));
						break;
				}
				cscf_get_transaction(msg,&x,&y);
				tmb.register_tmcb(msg,0,TMCB_RESPONSE_OUT,ims_pm_register_cb,(void*)k);
		}		
	}else{
		//unsigned int code = msg->first_line.u.reply.statuscode;
		method = cscf_get_cseq_method(msg,0);				
	}
	return 1;
}



int ims_pm_post_script(struct sip_msg *msg,void *param)
{
	return 1;
}


void ims_pm_notify_reg(r_notification *n,str call_id,int cseq)
{
	r_registration *r;
	r_regcontact *rc;
	for(r=n->registration;r;r=r->next)
		for(rc=r->contact;rc;rc=rc->next)
			switch (rc->event){
				case IMS_REGINFO_CONTACT_EXPIRED:
					IMS_PM_LOG21(UR_AttDeRegCscf,call_id,rc->uri,cseq);
					IMS_PM_LOG22(UR_SuccDeRegCscf,call_id,rc->uri,cseq,200);
					break;
				case IMS_REGINFO_CONTACT_DEACTIVATED:
				case IMS_REGINFO_CONTACT_UNREGISTERED:
				case IMS_REGINFO_CONTACT_PROBATION:
				case IMS_REGINFO_CONTACT_REJECTED:
					IMS_PM_LOG21(UR_AttDeRegHss,call_id,rc->uri,cseq);
					IMS_PM_LOG22(UR_SuccDeRegHss,call_id,rc->uri,cseq,200);
					break;
			}												
}

#endif /* WITH_IMS_PM */
