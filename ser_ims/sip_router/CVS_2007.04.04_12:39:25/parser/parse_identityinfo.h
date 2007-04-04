/*
 * $Id$ 
 *
 * Copyright (c) 2007 iptelorg GmbH
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


#ifndef PARSE_IDENTITYNFO
#define PARSE_IDENTITYNFO

#include "../str.h"

enum {
	II_START,
	II_URI_BEGIN,
	II_URI_DOMAIN,
	II_URI_IPV4,
	II_URI_IPV6,
	II_URI_PATH,
	II_URI_END,
	II_LWS,
	II_LWSCR,
	II_LWSCRLF,
	II_LWSCRLFSP,
	II_SEMIC,
	II_TAG,
	II_EQUAL,
	II_TOKEN,
	II_ENDHEADER
};

enum {
	II_M_START,
	II_M_URI_BEGIN,
	II_M_URI_END,
	II_M_SEMIC,
	II_M_TAG,
	II_M_EQUAL,
	II_M_TOKEN
};

#define ZSW(_c) ((_c)?(_c):"")

struct identityinfo_body {
	int error;  	/* Error code */
	str uri;    	/* URI */
	str domain; 	/* Domain part of the URI */
	str alg; 		/* Identity-Info header field MUST contain an 'alg' parameter */
};


/* casting macro for accessing IDENTITY-INFO body */
#define get_identityinfo(p_msg) ((struct identityinfo_body*)(p_msg)->identity_info->parsed)


/*
 * Parse Identity-Info header field
 */
char* parse_identityinfo(char *buffer, char* end, struct identityinfo_body *ii_b);


/*
 * Free all associated memory
 */
void free_identityinfo(struct identityinfo_body *ii_b);


#endif
