/*
 * $Id: release_call.c 220 2007-04-05 19:26:00Z vingarzan $
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
 * \file  ims_pm.c
 * 
 *	X-CSCF IMS Performance Management
 * 
 * Scope: logs raw data for computing metrics as in TS 32.409
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

//#define WITH_IMS_PM

#ifdef WITH_IMS_PM

#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "ims_pm.h"

#include "../../script_cb.h"
#include "../tm/tm_load.h"

#include "sip.h"
#include "registrar.h"
#include "cx_avp.h"

str log_prefix;

str log_header={"#\n#\n# Time,Node Type, Node Name, Event, String parameter 1, Integer parameter 1, Integer parameter 2\n",0};

int fd_log=0;
FILE *f_log=0;

str ims_pm_event_types[] = {
	{"OP.NOP",6},
	{"OP.NodeStart",12},
	{"OP.NodeStop",11},
	
	{"UR.AttInitReg",13},
	{"UR.SuccInitReg",14},
	{"UR.FailInitReg",14},
	{"UR.MeanInitRegSetupTime",23},

	{"UR.AttReReg",11},
	{"UR.SuccReReg",13},
	{"UR.FailReReg",13},
	{"UR.MeanReRegSetupTime",21},

	{"UR.AttDeRegUe",13},
	{"UR.SuccDeRegUe",14},
	{"UR.FailDeRegUe",14},
	{"UR.MeanDeRegUeSetupTime",23},

	{"UR.AttDeRegHss",14},
	{"UR.SuccDeRegHss",15},
	{"UR.FailDeRegHss",15},
	{"UR.MeanDeRegHssSetupTime",24},

	{"UR.AttDeRegCscf",15},
	{"UR.SuccDeRegCscf",16},
	{"UR.FailDeRegCscf",16},
	{"UR.MeanDeRegCscfSetupTime",25},
	
	{0,0}
};

extern struct tm_binds tmb;							/**< Structure with pointers to tm funcs 				*/

static str zero={0,0};

void ims_pm_init(str node_name,char* type, char *file_name)
{
	log_prefix.len = 1+strlen(type)+1+node_name.len+1;
	log_prefix.s = pkg_malloc(log_prefix.len);
	if (!log_prefix.s) {
		LOG(L_ERR,"ERR:"M_NAME":ims_pm_init(): Error alocating %d bytes\n",log_prefix.len);
		log_prefix.len=0;
		return;
	}
	log_prefix.len=0;
	
	log_prefix.s[log_prefix.len++]=',';
	
	memcpy(log_prefix.s+log_prefix.len,type,strlen(type));			
	log_prefix.len+=strlen(type);
	
	log_prefix.s[log_prefix.len++]=',';
	
	memcpy(log_prefix.s+log_prefix.len,node_name.s,node_name.len);
	log_prefix.len+=node_name.len;
	
	log_prefix.s[log_prefix.len++]=',';
	
	fd_log = open(file_name,O_WRONLY|O_APPEND|O_CREAT|O_NONBLOCK);
	if (fd_log<0){
		LOG(L_ERR,"ERR:"M_NAME":ims_pm_init(): Error opening fd logger file %s : %s\n",file_name,strerror(errno));
		fd_log = 0;
		return;		
	}
	f_log = fdopen(fd_log,"a");	
	if (!f_log){
		LOG(L_ERR,"ERR:"M_NAME":ims_pm_init(): Error opening logger file %s : %s\n",file_name,strerror(errno));
		close(fd_log);
		fd_log=0;
		return;
	}
	
	log_header.len = strlen(log_header.s);
	
	if (write(fd_log,log_header.s,log_header.len)<0){
		LOG(L_ERR,"ERR:"M_NAME":ims_pm_init(): Error writing to IMS PM log file: %s\n",strerror(errno));
	}
	IMS_PM_LOG(OP_NodeStart,zero,0,0);	
	
	register_script_cb(ims_pm_pre_script,PRE_SCRIPT_CB|REQ_TYPE_CB|RPL_TYPE_CB,0);
	register_script_cb(ims_pm_post_script,POST_SCRIPT_CB|REQ_TYPE_CB|RPL_TYPE_CB,0);
}

void ims_pm_destroy()
{
	IMS_PM_LOG(OP_NodeStop,zero,0,0);	
	if (fd_log) close(fd_log);
}

void ims_pm_log(enum _ims_pm_event_types event,str ps1,int pi1,int pi2)
{
	fprintf(f_log,"%u%.*s%.*s,%.*s,%d,%d\n",
		(unsigned int)time(0),
		log_prefix.len,log_prefix.s,
		ims_pm_event_types[event].len,ims_pm_event_types[event].s,
		ps1.len,ps1.s,
		pi1,
		pi2);
	LOG(L_CRIT,"%u%.*s%.*s,%.*s,%d,%d\n",
		(unsigned int)time(0),
		log_prefix.len,log_prefix.s,
		ims_pm_event_types[event].len,ims_pm_event_types[event].s,
		ps1.len,ps1.s,
		pi1,
		pi2);
	#if IMS_PM_DEBUG
		fflush(f_log);
	#endif
}


static str s_register={"REGISTER",8};

int ims_pm_get_registration_type(struct sip_msg *msg)
{
	str public_identity = cscf_get_public_identity(msg);
	if (r_is_registered_id(public_identity)){
		if (cscf_get_expires_hdr(msg)!=0) return 1;
		else return 2;		
	}
	else
		return 0;
}

void ims_pm_register_cb(struct cell* t, int type, struct tmcb_params* ps)
{
	int k = (int) *ps->param;
	int code = ps->code; 
	switch(k){
		case 0:
			if (code>=200 && code<300) 
				IMS_PM_LOG(UR_SuccInitReg,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
			else if (code>=300) IMS_PM_LOG(UR_FailInitReg,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
			break;
		case 1:
			if (code>=200 && code<300) IMS_PM_LOG(UR_SuccReReg,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),0);
			else if (code>=300) IMS_PM_LOG(UR_FailReReg,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
			break;
		case 2:
			if (code>=200 && code<300) IMS_PM_LOG(UR_SuccDeRegUe,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),0);
			else if (code>=300) IMS_PM_LOG(UR_FailDeRegUe,cscf_get_call_id(ps->req,0),cscf_get_cseq(ps->req,0),code);
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
						IMS_PM_LOG(UR_AttInitReg,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0),0);
						break;
					case 1:
						IMS_PM_LOG(UR_AttReReg,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0),0);
						break;
					case 2:
						IMS_PM_LOG(UR_AttDeRegUe,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0),0);
						break;
				}
				cscf_get_transaction(msg,&x,&y);
				tmb.register_tmcb(msg,0,TMCB_RESPONSE_OUT,ims_pm_register_cb,(void*)k);
		}
		
	}else{
		unsigned int code = msg->first_line.u.reply.statuscode;
		method = cscf_get_cseq_method(msg,0);		
		LOG(L_CRIT,">>> %.*s\n",method.len,method.s);
		/* REGISTER */
		if (method.len==s_register.len &&
			strncasecmp(method.s,s_register.s,s_register.len)==0){
				k = ims_pm_get_registration_type(msg);
				switch(k){
					case 0:
						if (code>=200 && code<300) 
							IMS_PM_LOG(UR_SuccInitReg,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0),code);
						else if (code>=300) IMS_PM_LOG(UR_FailInitReg,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0),code);
						break;
					case 1:
						if (code>=200 && code<300) IMS_PM_LOG(UR_SuccReReg,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0),0);
						else if (code>=300) IMS_PM_LOG(UR_FailReReg,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0),code);
						break;
					case 2:
						if (code>=200 && code<300) IMS_PM_LOG(UR_SuccDeRegUe,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0),0);
						else if (code>=300) IMS_PM_LOG(UR_FailDeRegUe,cscf_get_call_id(msg,0),cscf_get_cseq(msg,0),code);
						break;
				}
		}
		
	}
	return 1;
}



int ims_pm_post_script(struct sip_msg *msg,void *param)
{
	return 1;
}


void ims_pm_diameter_request(AAAMessage *msg)
{	
	switch(msg->applicationId){
    	case IMS_Cx:
			switch(msg->commandCode){				
				case IMS_RTR:														
					IMS_PM_LOG(UR_AttDeRegHss,Cx_get_session_id(msg),msg->endtoendId,0);
					return ;
					break;
			}
	}	
}

void ims_pm_diameter_answer(AAAMessage *msg)
{
	int code=-1;
	if (!Cx_get_result_code(msg,&code)) 
		Cx_get_experimental_result_code(msg,&code);
	switch(msg->applicationId){
    	case IMS_Cx:
			switch(msg->commandCode){				
				case IMS_RTA:
					if (code>=2000 && code<3000) IMS_PM_LOG(UR_SuccDeRegHss,Cx_get_session_id(msg),msg->endtoendId,code);
					else IMS_PM_LOG(UR_FailDeRegHss,Cx_get_session_id(msg),msg->endtoendId,code);
					break;
			}
	}
}

#endif /* WITH_IMS_PM */
