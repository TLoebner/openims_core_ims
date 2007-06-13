/*
 * $Id: ims_pm.h 220 2007-04-05 19:26:00Z vingarzan $
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
 * \file  ims_pm.h
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

#ifndef IMS_PM_H_
#define IMS_PM_H_

#include "mod.h"

#include "cx.h"

#define IMS_PM_DEBUG 1

enum _ims_pm_event_types {
	OP_NOP						= 0,

	OP_NodeStart				= 1,
	OP_NodeStop					= 2,
	
	UR_AttInitReg				= 3,
	UR_SuccInitReg				= 4,
	UR_FailInitReg				= 5,
	UR_MeanInitRegSetupTime 	= 6,

	UR_AttReReg					= 7,
	UR_SuccReReg				= 8,
	UR_FailReReg				= 9,
	UR_MeanReRegSetupTime 		= 10,

	UR_AttDeRegUe				= 11,
	UR_SuccDeRegUe				= 12,
	UR_FailDeRegUe				= 13,
	UR_MeanDeRegUeSetupTime		= 14,

	UR_AttDeRegHss				= 15,
	UR_SuccDeRegHss				= 16,
	UR_FailDeRegHss				= 17,
	UR_MeanDeRegHssSetupTime	= 18,
	
	UR_AttDeRegCscf				= 19,
	UR_SuccDeRegCscf			= 20,
	UR_FailDeRegCscf			= 21,
	UR_MeanDeRegCscfSetupTime 	= 22,
	

		
};


#define IMS_PM_LOG(event,ps1,pi1,pi2)  do {ims_pm_log(event,ps1,pi1,pi2);}while(0)  

void ims_pm_init(str node_name,char* type, char *file_name);
void ims_pm_destroy();

void ims_pm_log(enum _ims_pm_event_types event,str ps1,int pi1,int pi2);

int ims_pm_pre_script(struct sip_msg *msg,void *param);
int ims_pm_post_script(struct sip_msg *msg,void *param);

void ims_pm_diameter_request(AAAMessage *msg);
void ims_pm_diameter_answer(AAAMessage *msg);

#endif /*IMS_PM_H_*/
#else

#define IMS_PM_LOG(event,ps1,pi1,pi2) 

#endif /* WITH_IMS_PM */
