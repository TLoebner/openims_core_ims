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
  * -------
  *  2003-02-13  added proto to struct proxy_l & to *_proxy functions (andrei)
  */


#ifndef proxy_h
#define proxy_h

#include <netdb.h>
#include "ip_addr.h"
#include "str.h"
#include "config.h"

struct proxy_l{
	struct proxy_l* next;
	str name; /* original name */
	struct hostent host; /* addresses */
	unsigned short port;
	unsigned short reserved; /*align*/
	int proto;
	
	/* socket ? */

	int addr_idx;	/* crt. addr. idx. */
	int ok; /* 0 on error */
	/*statistics*/
	int tx;
	int tx_bytes;
	int errors;
};

extern struct proxy_l* proxies;

struct proxy_l* add_proxy(str* name, unsigned short port, int proto);
struct proxy_l* mk_proxy(str* name, unsigned short port, int proto);
struct proxy_l* mk_proxy_from_ip(struct ip_addr* ip, unsigned short port,
									int proto);
void free_proxy(struct proxy_l* p);



/* returns 0 on success, -1 on error (unknown af/bug) */
inline static int proxy2su(union sockaddr_union* su, struct proxy_l* p)
{
	/* if error try next ip address if possible */
	if (p->ok==0){
		if (p->host.h_addr_list[p->addr_idx+1])
			p->addr_idx++;
		else p->addr_idx=0;
		p->ok=1;
	}
	
	return hostent2su(su, &p->host, p->addr_idx,
				(p->port)?p->port:((p->proto==PROTO_TLS)?SIPS_PORT:SIP_PORT) );
}



/* mark as proxy either as ok (err>=0) or as bad (err<0) */
inline static void proxy_mark(struct proxy_l* p, int err)
{
	if (err<0){
		p->errors++;
		p->ok=0;
	}else{
		p->tx++;
	}
}



#endif

