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
 * IMS Service Control - ISC Marking Procedures
 * 
 * The specs say that the S-CSCF should somehow mark the message on the ISC so that
 * when it comes back the S-CSCF can identify it. This should also be fairly transparent.
 * 
 * The mark is as following: 
 * \code
Route: <appserver_uri>, <sip:ifcmark@[isc_my_uri];lr;s=xxx;h=xxx;d=xxx>
  \endcode  
 *
 *		- s - skip
 * 		- h - default handling
 * 		- d - direction  
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */


#include <stdio.h>

#include "../../mem/mem.h"
#include "../../dprint.h"
#include "../../action.h"

#include "mark.h"
#include "sip.h"
#include "mod.h"

extern str isc_my_uri;				/**< Uri of myself to loop the message in str	*/


/**
 * Duplicate a isc_mark structure into shm.
 * @param m - the isc mark to duplicate
 * @returns the new isc_mark* or NULL on error
 */
inline isc_mark* isc_mark_dup(isc_mark *m)
{
	isc_mark *x;
	x = shm_malloc(sizeof(isc_mark));
	if (!x){
		LOG(L_ERR, "ERR:"M_NAME":P_enforce_dialog_routes: Error allocating %d bytes\n",sizeof(isc_mark));
		return 0;
	}
	x->skip = m->skip;
	x->handling = m->handling;
	x->direction = m->direction;
	return x;
}

/**
 * Frees an isc_mark structure
 * @param m - the isc_mark to deallocate
 */
inline void isc_mark_free(isc_mark *m)
{
	if (m) shm_free(m);
}

/**
 *	Mark the message with the given mark.
 *		- old marking attempts are deleted
 *		- marking is performed by inserting the following header
 *	@param msg - SIP mesage to mark
 *	@param match - the current IFC match
 *	@param mark - pointer to the mark
 *	@returns 1 on success or 0 on failure
 */
int isc_mark_set(struct sip_msg *msg, isc_match *match, isc_mark *mark)
{
	str route={0,0};
	str as={0,0};
	char chr_mark[256];

	/* Drop all the old Header Lump "Route: <as>, <my>" */
	isc_mark_drop_route(msg);
	
	/* Create the Marking */		
	sprintf(chr_mark,"%s@%.*s;lr;s=%d;h=%d;d=%d",
		ISC_MARK_USERNAME,
		isc_my_uri.len,isc_my_uri.s,
		mark->skip,
		mark->handling,
		mark->direction
		);
	/* Add it in a lump */
	route.s = chr_mark;
	route.len = strlen(chr_mark);
	if (match) as = match->server_name;
	isc_mark_write_route(msg,&as,&route);
	LOG(L_INFO,"INFO:"M_NAME":isc_mark_set: NEW mark <%s>\n",chr_mark);	
	
	return 1;
}


/**
 * Load the mark from a string.
 * @param x - string with the mark, as found in the Route header
 * @param mark - mark to load into
 */
void isc_mark_get(str x,isc_mark *mark)
{
	int i,j,k;
	for(i=0;i<x.len&&x.s[i]!=';';i++);
	while(i<x.len){
		if (x.s[i+1]=='=') {
			k = 0;
			for(j=i+2;j<x.len && x.s[j]!=';';j++)
				k = k*10+(x.s[j]-'0');
			switch(x.s[i]){
				case 's':
					mark->skip = k;
					break;
				case 'h':
					mark->handling = k;
					break;
				case 'd':
					mark->direction = k;
					break;
				default:
					LOG(L_ERR,"INFO:"M_NAME":isc_mark_get: unkown parameter found: %c !\n",x.s[i]);
			}
			i = j+1;								
		} 
		else i++;
	}
}

/** char to hex convertor */
#define HEX_VAL(c)	( (c<='9')?(c-'0'):(c-'A'+10) )

/**
 *	Retrieves the mark from message.
 *		- the marking should be in a header like described before
 *	@param msg - SIP mesage to mark
 *  @param mark - mark to load into
 *	@returns 1 if found, 0 if not
 */
int isc_mark_get_from_msg(struct sip_msg *msg,isc_mark *mark)
{
	struct hdr_field *hdr;
	rr_t *rr;
	str x;
	LOG(L_INFO,"INFO:"M_NAME":isc_mark_get_from_msg: Trying to get the mark from the message \n");
	
	memset(mark,0,sizeof(isc_mark));
	
	parse_headers(msg,HDR_EOH_F,0);
	hdr = msg->headers;
	while(hdr){
		if (hdr->type == HDR_ROUTE_T){
			if (!hdr->parsed){
				if (parse_rr(hdr) < 0) {
					LOG(L_ERR, "ERROR:"M_NAME":isc_mark_get_from_msg: Error while parsing Route HF\n");
					hdr = hdr->next;
					continue;
				}
			}
			rr = (rr_t*)hdr->parsed;
			while(rr){
				x = rr->nameaddr.uri;
				if (x.len >= ISC_MARK_USERNAME_LEN + 1 + isc_my_uri.len &&
					strncasecmp(x.s,ISC_MARK_USERNAME,ISC_MARK_USERNAME_LEN)==0 &&
					strncasecmp(x.s+ISC_MARK_USERNAME_LEN+1,isc_my_uri.s,isc_my_uri.len)==0)
				{
					LOG(L_INFO,"INFO:"M_NAME":isc_mark_get_from_msg: Found <%.*s>\n",x.len,x.s);						
					isc_mark_get(x,mark);
					return 1;
				}					
				rr = rr->next;
			}
		}		
		hdr = hdr->next;
	}		
	return 0;
}

/**
 *	Retrieves the mark from lump.
 *		- the marking should be in a header like described before
 * 		- this is usefull when the message was not sent yet and as such the
 * new Route header is not visible
 *	@param msg - SIP mesage to mark
 *  @param mark - mark to load into
 *	@returns 1 if found, 0 if not
 */
int isc_mark_get_from_lump(struct sip_msg *msg,isc_mark *mark)
{
	struct lump* lmp,*tmp;;
	struct lump* anchor;
	str x;

	anchor = anchor_lump(msg, msg->headers->name.s - msg->buf, 0 , 0);

	memset(mark,0,sizeof(isc_mark));
		
	LOG(L_DBG,"DEBUG:"M_NAME":isc_mark_get_from_lump: Start --------- \n");
	lmp = msg->add_rm;
	while(lmp){		
		tmp= lmp->before;
		if (tmp &&
			tmp->op==LUMP_ADD &&
			tmp->u.value &&
			strstr(tmp->u.value,ISC_MARK_USERNAME))
		{			
			LOG(L_DBG,"DEBUG:"M_NAME":isc_mark_get_from_lump: Found lump %s ... dropping\n",tmp->u.value);										
			x.s = lmp->u.value;
			x.len = lmp->len;
			isc_mark_get(x,mark);
			return 1;
		}
		lmp = lmp->next;		
	}
	LOG(L_DBG,"DEBUG:"M_NAME":isc_mark_get_from_lump: ---------- End \n");
	
	return 0;
}

/**
 *	Inserts the Route header for marking, before first header.
 * - the marking will be in a header like below
 * - if the "as" parameter is empty: \code Route: <[iscmark]> \endcode
 * - else: \code Route: <sip:as@asdomain.net;lr>, <[iscmark]> \endcode
 * 			 
 *
 *	@param msg - SIP mesage to mark
 *	@param as - SIP addres of the application server to forward to
 *	@param iscmark - the mark to write
 *	@returns 1 on success, else 0
 */
inline int isc_mark_write_route(struct sip_msg *msg,str *as,str *iscmark)
{
	struct hdr_field *first;
	struct lump* anchor;
	str route;
	
	parse_headers(msg,HDR_EOH_F,0);			
	first = msg->headers;	
	if (as && as->len) {
		route.s = pkg_malloc(21+as->len+iscmark->len);
		sprintf(route.s,"Route: <%.*s;lr>, <%.*s>\r\n",as->len,as->s,iscmark->len,iscmark->s);
	}else{
		route.s = pkg_malloc(18+iscmark->len);
		sprintf(route.s,"Route: <%.*s>\r\n",iscmark->len,iscmark->s);
	}
	
	route.len =strlen(route.s);
	LOG(L_DBG,"DEBUG:"M_NAME":isc_mark_write_route: <%.*s>\n",route.len,route.s);
	
	anchor = anchor_lump(msg, first->name.s - msg->buf, 0 , HDR_ROUTE_T);
	if (anchor == NULL) {
		LOG(L_ERR, "ERROR:"M_NAME":isc_mark_write_route: anchor_lump failed\n");
		return 0;
	}

	if (!insert_new_lump_before(anchor, route.s,route.len,HDR_ROUTE_T)){
			LOG( L_ERR, "ERROR:"M_NAME":isc_mark_write_route: error creting lump for header_mark\n" );
	}	
	return 1;
}


/**
 *	Deletes the previous marking attempts (lumps).
 *
 *	@param msg - SIP mesage to mark
 *	@returns 1 on success
 */
inline int isc_mark_drop_route(struct sip_msg *msg)
{
	struct lump* lmp,*tmp;
	struct lump* anchor;

	parse_headers(msg,HDR_EOH_F,0);			

	anchor = anchor_lump(msg, msg->headers->name.s - msg->buf, 0 , 0);
		
	LOG(L_DBG,"DEBUG:"M_NAME":ifc_mark_drop_route: Start --------- \n");
	lmp = msg->add_rm;
	while(lmp){		
		tmp= lmp->before;
		if (tmp &&
			tmp->op==LUMP_ADD &&
			tmp->u.value &&
			strstr(tmp->u.value,ISC_MARK_USERNAME))
		{			
			LOG(L_DBG,"DEBUG:"M_NAME":ifc_mark_drop_route: Found lump %s ... dropping\n",tmp->u.value);										
			//tmp->op=LUMP_NOP;			
			tmp->len = 0;
			/*lmp->before = tmp->before;
			free_lump(tmp);	*/
		}
		lmp = lmp->next;		
	}
	LOG(L_DBG,"DEBUG:"M_NAME":ifc_mark_drop_route: ---------- End \n");
	
	return 1;
}
