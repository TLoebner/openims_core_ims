/*
 * $Id$
 *
 *
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of ser, a free SIP server.
 *
 * ser is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the ser software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * ser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * History:
 * --------
 *  2003-02-18  added proto to various function prototypes (andrei)
 */


#ifndef _T_FWD_H
#define _T_FWD_H

#include "defs.h"

#include "../../proxy.h"
#include "h_table.h"

typedef int (*tfwd_f)(struct sip_msg* p_msg , struct proxy_l * proxy );
typedef int (*taddblind_f)( /*struct cell *t */ );

void t_on_branch(unsigned int go_to);
unsigned int get_on_branch();
int t_replicate(struct sip_msg *p_msg, struct proxy_l * proxy, int proto);
/*  -- not use outside t_fwd.c for noe
char *print_uac_request( struct cell *t, struct sip_msg *i_req,
    int branch, str *uri, unsigned int *len, struct dest_info *dst);
*/
void e2e_cancel( struct sip_msg *cancel_msg, struct cell *t_cancel, struct cell *t_invite );
int e2e_cancel_branch( struct sip_msg *cancel_msg, struct cell *t_cancel, struct cell *t_invite, int branch );
int add_uac(	struct cell *t, struct sip_msg *request, str *uri, str* next_hop,
				struct proxy_l *proxy, int proto );
#ifdef USE_DNS_FAILOVER
int add_uac_dns_fallback( struct cell *t, struct sip_msg* msg, 
									struct ua_client* old_uac,
									int lock_replies);
#endif
int add_blind_uac( /* struct cell *t */ );
int t_forward_nonack( struct cell *t, struct sip_msg* p_msg,
						struct proxy_l * p, int proto);
int t_forward_ack( struct sip_msg* p_msg );
int t_send_branch( struct cell *t, int branch, struct sip_msg* p_msg ,
					struct proxy_l * proxy, int lock_replies);


#endif


