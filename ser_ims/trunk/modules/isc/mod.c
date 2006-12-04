/**
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
 * \file
 * 
 * IMS Service Control - SER module interface
 * 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "mod.h"
#include "../tm/tm_load.h"
#include "../scscf/scscf_load.h"

#include "sip.h"
#include "checker.h"
#include "mark.h"
#include "isc.h"

MODULE_VERSION

static int isc_init( void );
static int isc_child_init( int rank );
static int isc_exit( void );

int isc_appserver_forward(struct sip_msg *msg,char *str1,char *str2 );

int ISC_match_filter(struct sip_msg *msg,char *str1,char *str2);
int ISC_from_AS(struct sip_msg *msg,char *str1,char *str2);
int ISC_is_session_continued(struct sip_msg *msg,char *str1,char *str2);

/**
 *	Parameters
 */

char *isc_my_uri_c = "scscf.open-ims.test:6060";				/**< Uri of myself to loop the message 		*/
str isc_my_uri={0,0};				/**< Uri of myself to loop the message in str	*/
str isc_my_uri_sip={0,0};			/**< Uri of myself to loop the message in str with leading "sip:" */

/**
 *  Global vars
 */

struct tm_binds isc_tmb;            /**< Structure with pointers to tm funcs 		*/
struct scscf_binds isc_scscfb;      /**< Structure with pointers to S-CSCF funcs 		*/

/**
 * Exported functions. This is the API available for use from other SER modules.
 * - isc_appserver_forward() - Simple app server like behaviour. Just returns the message 
 * back with an added header. Params (uri of SCSCF to default to if not found, header to add to msg) 
 * <p>
 * - ISC_match_filter() - check if a message matches any IFC and forward to AS if it does.
 * 	Params (char *direction) "orig"/"term" 
 * - ISC_from_AS() - Finds if the message returns from the AS.
 * - ISC_is_session_continued() - Finds if the failed fwd should be continued or session should be broken 
 */
static cmd_export_t isc_cmds[] = {
	{"isc_appserver_forward", isc_appserver_forward, 2, 0, REQUEST_ROUTE},	
	
	{"ISC_match_filter", ISC_match_filter, 1, 0, REQUEST_ROUTE},
	{"ISC_from_AS", ISC_from_AS, 1, 0, REQUEST_ROUTE},
	{"ISC_is_session_continued", ISC_is_session_continued, 0, 0, FAILURE_ROUTE},
	
	{ 0, 0, 0, 0, 0 }
};


/**
 * Exported parameters.
 * - my_uri - URI pointing to myself. Default value in #isc_my_uri_c.
 */
static param_export_t isc_params[] = {
	{"my_uri",  		STR_PARAM, & isc_my_uri_c},		/**< SIP Uri of myself for getting the messages back */
	{ 0, 0, 0 }
};


/**
 * Exported module interface
 */
struct module_exports exports = {
	"isc",
	isc_cmds,                       /**< Exported functions */
	0,
	isc_params,                     /**< Exported parameters */
	isc_init,                   /**< Module initialization function */
	(response_function) 0,
	(destroy_function) isc_exit,
	0,
	(child_init_function) isc_child_init /**< per-child init function */
};




/** 
 * Module init function.
 * Initializes the ISC structures
 */
static int isc_init( void )
{
	load_tm_f load_tm;
	load_scscf_f load_scscf;
	LOG( L_INFO, "INFO:"M_NAME": - init\n" );

	/* import the TM auto-loading function */
	if (!(load_tm = (load_tm_f)find_export("load_tm",NO_SCRIPT,0))) {
		LOG(L_ERR,"ERROR:"M_NAME":isc_init: cannot import load_tm\n");
		goto error;
	}
	/* let the auto-loading function load all TM stuff */
	if (load_tm(&isc_tmb) == -1)
		goto error;

	/* import the SCSCF auto-loading function */
	if (!(load_scscf = (load_scscf_f)find_export("load_scscf",NO_SCRIPT,0))) {
		LOG(L_ERR,"ERROR:"M_NAME":isc_init: cannot import load_scscf\n");
		goto error;
	}
	/* let the auto-loading function load all TM stuff */
	if (load_scscf(&isc_scscfb) == -1)
		goto error;

	/* Init the isc_my_uri parameter */
	if (!isc_my_uri_c) {
		LOG( L_CRIT, "ERROR:"M_NAME":isc_init: mandatory parameter \"isc_my_uri\" found empty\n" );
		goto error;
	}
	isc_my_uri.s = isc_my_uri_c;
	isc_my_uri.len = strlen(isc_my_uri_c);	
	isc_my_uri_sip.len = 4+isc_my_uri.len;
	isc_my_uri_sip.s = shm_malloc(isc_my_uri_sip.len+1);	
	memcpy(isc_my_uri_sip.s,"sip:",4);
	memcpy(isc_my_uri_sip.s+4,isc_my_uri.s,isc_my_uri.len);	
	isc_my_uri_sip.s[isc_my_uri_sip.len]=0;	
	return 0;
error:
	return -1;
}


/**
 *	Child init function.
 * Nothing done here.
 */
static int isc_child_init( int rank )
{
	LOG( L_INFO, "INFO:"M_NAME": - child init [%d]\n", rank );

/* don't do anything for main process and TCP manager process */
	if ( rank == PROC_MAIN || rank == PROC_TCP_MAIN )
		return 0;

	// also nothing for the rest of processes
	return 0;
}


/**
 *	Module termination function.
 * Destroy the ISC structures. 
 */
static int isc_exit( void )
{
	LOG( L_INFO, "INFO:"M_NAME": - child exit\n" );
	return 0;
}






/**
 * Send the request back simulating an AS. 
 * - Inserts a Header with the contents of str2
 * - Tries to find the Route: appserver_uri, s-cscf_uri header and removes appserver_uri from it
 * - It forwards the message to s-scsf_uri, or if none found to str1
 *	@param msg - The sip message
 *	@param str1 - The uri of the SCSCF to default to if Route header is not found
 *	@param str2 - The Header to add to the message - if empty none will be added
 *	@returns	- 0 to cancel further script processing 
 */
int isc_appserver_forward(struct sip_msg *msg,char *str1,char *str2 )
{
	struct hdr_field *last,*hdr;
	struct lump* anchor;
	str header_mark;
	rr_t *rr;

	LOG(L_DBG,"DEBUG:"M_NAME":isc_appserver_forward: Forward-back request reached\n");
	parse_headers(msg,HDR_EOH_F,0);
	last = msg->headers;
	while(last->next)
		last = last->next;

	LOG(L_INFO,"INFO:"M_NAME":isc_appserver_forward: New header: [%s]\n%.*s",str2,msg->len,msg->buf);
	
    /* Put header marking */

	if (strlen(str2)){
	
		header_mark.s = pkg_malloc(256);//36 should be enough
		sprintf(header_mark.s,"%.*s\n",strlen(str2),str2);
		header_mark.len =strlen(header_mark.s);
	
		anchor = anchor_lump(msg, last->name.s + last->len - msg->buf, 0 , 0);
		if (anchor == NULL) {
				LOG(L_ERR, "ERROR:"M_NAME":isc_appserver_forward: anchor_lump failed\n");
		}
	
		if (!insert_new_lump_before(anchor, header_mark.s,header_mark.len,0)){
				LOG( L_ERR, "ERROR:"M_NAME":isc_appserver_forward: error creting lump for header_mark\n" );
		}
		//pkg_free(header_mark.s);
	}
	LOG(L_ERR, "INFO:"M_NAME":isc_appserver_forward: Searching Route header to rewrite\n");
	/* Search for the Route */
	hdr = msg->headers;
	while(hdr){
		if (hdr->type == HDR_ROUTE_T){
			if (!hdr->parsed){
				if (parse_rr(hdr) < 0) {
					LOG(L_ERR, "ERROR:"M_NAME":isc_appserver_forward: Error while parsing Route HF\n");
					continue;
				}
			}
			rr = (rr_t*)hdr->parsed;
			while(rr){
				if (rr->nameaddr.uri.len >= ISC_MARK_USERNAME_LEN &&
					strncasecmp(rr->nameaddr.uri.s,ISC_MARK_USERNAME,ISC_MARK_USERNAME_LEN)==0)	{
						LOG(L_INFO,"DEBUG:"M_NAME":isc_appserver_forward: Found S-CSCF marking <%.*s> \n",rr->nameaddr.uri.len,rr->nameaddr.uri.s);
						/* delete the header */					
						if (!del_lump(msg, hdr->name.s - msg->buf, hdr->len, 0)) {
							LOG(L_ERR, "ERROR:"M_NAME":isc_appserver_forward: Can't remove Route HF\n");
						}  
										
						/* add the new header */
						anchor = anchor_lump(msg, msg->headers->name.s - msg->buf, 0 , 0);
						header_mark.s = pkg_malloc(256);//36 should be enough
						sprintf(header_mark.s,"Route: <%.*s>\n",rr->nameaddr.uri.len,rr->nameaddr.uri.s);
						header_mark.len =strlen(header_mark.s);	
						if (!insert_new_lump_before(anchor, header_mark.s,header_mark.len,0)){
							LOG( L_ERR, "ERROR:"M_NAME":isc_appserver_forward: error creting lump for route header\n" );
						}	
					/* send the message */
					msg->dst_uri.s = pkg_malloc(rr->nameaddr.uri.len);
					memcpy(msg->dst_uri.s,rr->nameaddr.uri.s,rr->nameaddr.uri.len);
					msg->dst_uri.len=rr->nameaddr.uri.len;
					isc_tmb.t_relay(msg,0,0);
					return 0;
				}					
				rr = rr->next;
			}			
		}
		hdr = hdr->next;
	}
	
	/* In case no suitable route header found, just fwd to the given parameter */
	msg->dst_uri.len = strlen(str1);
	msg->dst_uri.s = str1;
	isc_tmb.t_relay(msg,0,0);
	LOG(L_DBG,"DEBUG:"M_NAME":isc_appserver_forward: Mirror request finished\n");
	return 0;
}


/**
 * Returns the direction of the dialog as int dialog_direction from a string.
 * @param direction - "orig" or "term"
 * @returns DLG_MOBILE_ORIGINATING, DLG_MOBILE_TERMINATING if successful, or 
 * DLG_MOBILE_UNKNOWN on error
 */
static inline enum dialog_direction get_dialog_direction(char *direction)
{
	switch(direction[0]){
		case 'o':
		case 'O':
		case '0':
			return DLG_MOBILE_ORIGINATING;
		case 't':
		case 'T':
		case '1':
			return DLG_MOBILE_TERMINATING;
		default:
			LOG(L_ERR,"ERR:"M_NAME":get_dialog_direction(): Unknown direction %s",direction);
			return DLG_MOBILE_UNKNOWN;
	}
}

/**
 * Checks if there is a match.
 * Inserts route headers and set the dst_uri
 * @param msg - the message to check
 * @param str1 - the direction of the request orig/term
 * @param str2 - not used
 * @returns #ISC_RETURN_TRUE if found, #ISC_RETURN_FALSE if not, #ISC_RETURN_BREAK on error
 */
int ISC_match_filter(struct sip_msg *msg,char *str1,char *str2)
{
	int k;
	isc_match *m;
	str s={0,0};
	int ret = ISC_RETURN_FALSE;
	isc_mark new_mark,old_mark;
	
	enum dialog_direction dir = get_dialog_direction(str1);
	
	LOG(L_INFO,"INFO:"M_NAME":ISC_match_filter(%s): Checking triggers\n",str1);
	
	if (dir==DLG_MOBILE_UNKNOWN) 
		return ISC_RETURN_BREAK;
	
	if (!isc_is_initial_request(msg)) return ISC_RETURN_FALSE;
		
	/* starting or resuming? */
	memset(&old_mark,0,sizeof(isc_mark));
	if (isc_mark_get_from_msg(msg,&old_mark)){		
		LOG(L_INFO,"INFO:"M_NAME":ISC_match_filter(%s): Message returned s=%d;h=%d;d=%d\n",
			str1,old_mark.skip,old_mark.handling,old_mark.direction);
	}
	else {
		LOG(L_INFO,"INFO:"M_NAME":ISC_match_filter(%s): Starting triggering\n",str1);				
	}

	/* originating leg */
	if (dir==DLG_MOBILE_ORIGINATING){
		k = isc_get_originating_user(msg,&s);
		if (k){
			k = isc_is_registered(&s);
			if (k==NOT_REGISTERED) return ISC_MSG_NOT_FORWARDED;
			
			LOG(L_INFO,"INFO:"M_NAME":ISC_match_filter(%s): Orig User <%.*s> [%d]\n",str1,
				s.len,s.s,k);
			m = isc_checker_find(s,old_mark.direction,old_mark.skip,msg);
			if (m){
				new_mark.direction = IFC_ORIGINATING_SESSION;
				new_mark.skip = m->index+1;
				new_mark.handling = m->default_handling;
				ret = isc_forward(msg,m,&new_mark);
				isc_free_match(m);
				return ret;
			}
		}
		return ret;
	}

	/* terminating leg */
	if (dir==DLG_MOBILE_TERMINATING){
		k = isc_get_terminating_user(msg,&s);
		if (k){
			k = isc_is_registered(&s);
			if (k==REGISTERED) {
				new_mark.direction = IFC_TERMINATING_SESSION;
			} else {
				new_mark.direction = IFC_TERMINATING_UNREGISTERED;
			}
			LOG(L_INFO,"INFO:"M_NAME":ISC_match_filter(%s): Orig User <%.*s> [%d]\n",str1,
				s.len,s.s,k);
			m = isc_checker_find(s,new_mark.direction,old_mark.skip,msg);
			if (m){
				new_mark.skip = m->index+1;
				new_mark.handling = m->default_handling;
				ret = isc_forward(msg,m,&new_mark);
				isc_free_match(m);
				if (s.s) pkg_free(s.s);	
				return ret;
			}
		}
		if (s.s) pkg_free(s.s);	
		return ret;
	}					
		
	return ret;
}

/**
 * Check if the message is from the AS.
 * Inserts route headers and set the dst_uri
 * @param msg - the message to check
 * @param str1 - the direction of the request orig/term
 * @param str2 - not used
 * @returns #ISC_RETURN_TRUE if from AS, #ISC_RETURN_FALSE if not, #ISC_RETURN_BREAK on error
 */
int ISC_from_AS(struct sip_msg *msg,char *str1,char *str2)
{
	int ret = ISC_RETURN_FALSE;
	isc_mark old_mark;
	
	enum dialog_direction dir = get_dialog_direction(str1);
		
	if (dir==DLG_MOBILE_UNKNOWN) 
		return ISC_RETURN_BREAK;
	
	if (!isc_is_initial_request(msg)) return ISC_RETURN_FALSE;
		
	/* starting or resuming? */
	if (isc_mark_get_from_msg(msg,&old_mark)){		
		LOG(L_INFO,"INFO:"M_NAME":ISC_from_AS(%s): Message returned s=%d;h=%d;d=%d\n",
			str1,old_mark.skip,old_mark.handling,old_mark.direction);
		if (old_mark.direction==IFC_ORIGINATING_SESSION && dir!=DLG_MOBILE_ORIGINATING)
			ret = ISC_RETURN_FALSE;
		else
		if ((old_mark.direction==IFC_TERMINATING_SESSION||old_mark.direction==IFC_TERMINATING_UNREGISTERED)
			 && dir!=DLG_MOBILE_TERMINATING)
			ret = ISC_RETURN_FALSE;
		else
			ret = ISC_RETURN_TRUE;
	} else {
		ret = ISC_RETURN_FALSE;
	}
	return ret;
}

/**
 * Check if the message is from the AS.
 * Inserts route headers and set the dst_uri
 * @param msg - the message to check
 * @param str1 - the direction of the request orig/term
 * @param str2 - not used
 * @returns #ISC_RETURN_TRUE if session should continue, #ISC_RETURN_FALSE if not, #ISC_RETURN_BREAK on error
 */
int ISC_is_session_continued(struct sip_msg *msg,char *str1,char *str2)
{
	int ret = ISC_RETURN_FALSE;
	isc_mark old_mark;
	struct sip_msg *req;

	req = cscf_get_request_from_reply(msg);
	
	if (!req) {
		LOG(L_ERR,"ERR:"M_NAME":ISC_is_session_continued(): There is no transaction \n");			
		return ISC_RETURN_FALSE;
	}
	
	if (!isc_is_initial_request(req)) {
		LOG(L_ERR,"ERR:"M_NAME":ISC_is_session_continued(): This is no initial request \n");			
		return ISC_RETURN_FALSE;
	}
		
	/* starting or resuming? */
	if (isc_mark_get_from_lump(req,&old_mark)){		
		LOG(L_INFO,"INFO:"M_NAME":ISC_is_session_continued(): Message returned handling [%d] \n",old_mark.handling);
		if (old_mark.handling == IFC_SESSION_CONTINUED) 
			ret = ISC_RETURN_TRUE;
		else 
			ret = ISC_RETURN_FALSE;
	} else {
		LOG(L_ERR,"ERR:"M_NAME":ISC_is_session_continued(): mark not found in lump \n");					
		ret = ISC_RETURN_ERROR;
	}
	return ret;
}

