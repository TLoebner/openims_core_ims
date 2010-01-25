/*
 * Copyright (C) 2008-2009 FhG Fokus
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
 * \file release_call.h
 * 
 *	E-CSCF initiated call release (for confirmed dialogs and QoS relevant cases)
 * 
 *  \author Alberto Diez     albertowhiterabbit at yahoo dot es
 *  \author Ancuta Onofrei   andreea dot ancuta dot onofrei -at- fokus dot fraunhofer dot de
 */


#ifndef RELEASE_CALL_H_
#define RELEASE_CALL_H_
#include "../../modules/tm/tm_load.h"
#include "dlg_state.h"
#include "sip.h"

#ifdef SER_MOD_INTERFACE
	#include "../../modules_s/dialog/dlg_mod.h"
#else
	#include "../../modules/dialog/dlg_mod.h"
#endif


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
