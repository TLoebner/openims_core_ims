/**
 * $Id$
 *   
 * Copyright (C) 2004-2007 FhG Fokus
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
 * \file
 * 
 * CDiameterPeer Session Handling - Authorization State Machine
 * 
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * \author Shengyao Chen shc -at- fokus dot fraunhofer dot de
 * \author Joao Filipe Placido joao-f-placido -at- ptinovacao dot pt
 * 
 */

#include <time.h>
 
#include "authstatemachine.h"

char *auth_states[]={"Idle","Pending","Open","Discon"};
char *auth_events[]={};


int get_result_code(AAAMessage* msg)
{
        if (!msg) goto error;
        AAA_AVP* rc = AAAFindMatchingAVP(msg, 0, AVP_Result_Code, 0, 0);
        if (!rc) goto error;
        return get_4bytes(rc->data.s);

error:
        LOG(L_ERR, "ERR:get_result_code(): no AAAMessage or Result Code not found\n");
        return -1;
}

int get_auth_session_state(AAAMessage* msg)
{
        if (!msg) goto error;
        AAA_AVP* rc = AAAFindMatchingAVP(msg, 0, AVP_Auth_Session_State, 0, 0);
        if (!rc) goto error;
        return get_4bytes(rc->data.s);

error:
        LOG(L_ERR, "ERR:get_auth_session_state(): no AAAMessage or Auth Session State not found\n");
        return -1;
}


/**
 * stateful client state machine
 * @param auth - AAAAuthSession which uses this state machine
 * @param ev   - Event
 * @param req  - AAAMessage
 */
inline void auth_client_statefull_sm_process(cdp_session_t* s, int event, AAAMessage* msg)
{
	cdp_auth_session_t *x;
	if (!s) return;
	x = &(s->u.auth);
	switch(x->state){
		case AUTH_ST_IDLE:
			switch (event) {
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_PENDING:
			switch (event) {
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_OPEN:
			switch (event) {
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_DISCON:
			switch (event) {
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		default:
			LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received event %d while in invalid state %d!\n",
				event,x->state);
	}
}


/**
 * Authorization Server State-Machine - Statefull
 */
inline void auth_server_statefull_sm_process(cdp_session_t* s, int event, AAAMessage* msg)
{
	cdp_auth_session_t *x;
	if (!s) return;
	x = &(s->u.auth);
	switch(x->state){
		case AUTH_ST_IDLE:
			switch (event) {
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_OPEN:
			switch (event) {
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		
		case AUTH_ST_DISCON:
			switch (event) {
				default:
					LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;

		default:
			LOG(L_ERR,"ERR:auth_client_statefull_sm_process(): Received event %d while in invalid state %d!\n",
				event,x->state);
	}	
}




/** 
 * Authorization Client State-Machine - Stateless
 */ 
inline void auth_client_stateless_sm_process(cdp_session_t* s, int event, AAAMessage *msg)
{
	cdp_auth_session_t *x;
	int rc;
	if (!s) return;
	x = &(s->u.auth);
	switch(x->state){
		case AUTH_ST_IDLE:
			switch(event){
				case AUTH_EV_SEND_REQ:
					x->state = AUTH_ST_PENDING;
					break;
				default:
					LOG(L_ERR,"ERR:auth_client_stateless_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		case AUTH_ST_PENDING:
			if (!is_req(msg)){
				rc = get_result_code(msg);
				if (rc>=2000 && rc<3000 && get_auth_session_state(msg)==NO_STATE_MAINTAINED) 
					event = AUTH_EV_RECV_ANS_SUCCESS;
				else
					event = AUTH_EV_RECV_ANS_UNSUCCESS;
			}
			switch(event){
				case AUTH_EV_RECV_ANS_SUCCESS:
					x->state = AUTH_ST_OPEN;
					break;
				case AUTH_EV_RECV_ANS_UNSUCCESS:
					x->state = AUTH_ST_IDLE;
					break;					
				default:
					LOG(L_ERR,"ERR:auth_client_stateless_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		case AUTH_ST_OPEN:
			switch(event){
				case AUTH_EV_SESSION_TIMEOUT:
					x->state = AUTH_ST_IDLE;
					break;
				case AUTH_EV_SERVICE_TERMINATED:
					x->state = AUTH_ST_IDLE;
					break;					
				default:
					LOG(L_ERR,"ERR:auth_client_stateless_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_states[x->state]);				
			}
			break;
		default:
			LOG(L_ERR,"ERR:auth_client_stateless_sm_process(): Received event %d while in invalid state %d!\n",
				event,x->state);
	}	
}

/**
 * Authorization Server State-Machine - Stateless
 */
inline void auth_server_stateless_sm_process(cdp_session_t* auth, int event, AAAMessage* msg)
{
	/* empty - no state change, anyway */
/*
	cdp_auth_session_t *x;
	int rc;
	if (!s) return;
	x = &(s->u.auth);
	switch(x->state){
		case AUTH_ST_IDLE:
			switch(event){				
				default:
					LOG(L_ERR,"ERR:auth_server_stateless_sm_process(): Received invalid event %d while in state %s!\n",
						event,auth_state[x->state]);				
			}
			break;
		default:
			LOG(L_ERR,"ERR:auth_server_stateless_sm_process(): Received event %d while in invalid state %d!\n",
				event,x->state);
	}
*/
}


