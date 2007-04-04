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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../mem/mem.h"
#include "parse_def.h"
#include "parse_identityinfo.h"
#include "parser_f.h"  /* eat_space_end and so on */


char* parse_identityinfo(char *buffer, char *end, struct identityinfo_body *ii_b)
{
	int status = II_START;
	int mainstatus = II_M_START;
	char *p;


	if (!buffer || !end || !ii_b) return NULL;

	ii_b->error = PARSE_ERROR;

	for(p = buffer; p < end; p++) {
		switch(*p) {
			case '<':
				if (status == II_START) {
					status=II_URI_BEGIN;
					mainstatus = II_M_URI_BEGIN;
					ii_b->uri.s = p + 1;
				} else
					goto parseerror;
					break;
			case 'h':
				case 'H': /* "http://" or "https://" part  */
					switch (status) {
						case II_URI_BEGIN:
							if (end - p <= 8 || strncasecmp(p,"http",strlen("http")))
								goto parseerror;
							p+=4;
							if (*p == 's' || *p == 'S') p++;
							if (memcmp(p,"://",strlen("://")))
								goto parseerror;
							p+=2;
							status = II_URI_DOMAIN;
							break;
						case II_URI_DOMAIN:
							status = II_URI_IPV4;
						case II_URI_IPV4:
						case II_URI_IPV6:
						case II_URI_PATH:
						case II_TOKEN:
						case II_TAG:
							break;
						case II_EQUAL:
							status = II_TOKEN;
							mainstatus = II_M_TOKEN;
							ii_b->alg.s = p;
							break;
						default:
							goto parseerror;
					}
					break;
			case '/':
				switch(status){
					case II_URI_IPV4:
						ii_b->domain.len = p - ii_b->domain.s;
						status = II_URI_PATH;
						break;
					case II_URI_PATH:
						break;
					case II_URI_IPV6:
					default:
						goto parseerror;
				}
				break;
			case '>':
				if (status == II_URI_PATH) {
					ii_b->uri.len = p - ii_b->uri.s;
					status = II_URI_END;
					mainstatus = II_M_URI_END;
				} else
					goto parseerror;
					break;
			case ' ':
			case '\t':
				switch (status) {
					case II_EQUAL:
					case II_TAG:
					case II_SEMIC:
					case II_URI_END:
						status = II_LWS;
						break;
					case II_LWS:
						break;
					case II_LWSCRLF:
						status = II_LWSCRLFSP;
						break;
					default:
						goto parseerror;
				}
				break;
			case '\r':
				switch (status) {
					case II_TOKEN:
						ii_b->alg.len = p - ii_b->alg.s;
						status = II_ENDHEADER;
						break;
					case II_EQUAL:
					case II_TAG:
					case II_SEMIC:
					case II_URI_END:
					case II_LWS:
						status = II_LWSCR;
						break;
					default:
						goto parseerror;
				}
				break;
			case '\n':
				switch (status) {
					case II_EQUAL:
					case II_TAG:
					case II_SEMIC:
					case II_URI_END:
					case II_LWS:
					case II_LWSCR:
						status = II_LWSCRLF;
						break;
						case II_TOKEN: /* if there was not '\r' */
							ii_b->alg.len = p - ii_b->alg.s;
					case II_ENDHEADER:
						p=eat_lws_end(p, end);
						/*check if the header ends here*/
						if (p>=end) {
							LOG(L_ERR, "ERROR: parse_date: strange EoHF\n");
							goto parseerror;
						}
						if (*p=='\r' && p+1<end && *(p+1)=='\n') {
							ii_b->error=PARSE_OK;
							return p+2;
						}
						if (*p=='\n') {
							ii_b->error=PARSE_OK;
							return p+1;
						}
						LOG(L_ERR, "ERROR: Date EoL expected\n");
						goto error;
						break;
					default:
						goto parseerror;
				}
				break;
			case ';':
				switch (status) {
					case II_URI_END:
					case II_LWS:
					case II_LWSCRLFSP:
						if (mainstatus == II_M_URI_END) {
							status = II_SEMIC;
							mainstatus = II_M_SEMIC;
						} else
							goto parseerror;
						break;
					default:
						goto parseerror;
				}
				break;
				case 'a': /* tag part of 'alg' parameter */
			case 'A':
				switch (status) {
					case II_LWS:
					case II_LWSCRLFSP:
					case II_SEMIC:
						if (mainstatus == II_M_SEMIC) {
							mainstatus = II_M_TAG;
							status = II_TAG;
							if (end - p <= 3 || strncasecmp(p,"alg",strlen("alg")))
								goto parseerror;
							p+=2;
						} else
							goto parseerror;
						break;
					case II_URI_DOMAIN:
						status = II_URI_IPV4;
					case II_URI_IPV4:
					case II_URI_IPV6:
					case II_URI_PATH:
					case II_TOKEN:
						break;
					case II_EQUAL:
						status = II_TOKEN;
						mainstatus = II_M_TOKEN;
						ii_b->alg.s = p;
						break;
					default:
						goto parseerror;
				}
				break;
			case '=':
				switch (status) {
					case II_TAG:
					case II_LWS:
					case II_LWSCRLFSP:
						if (mainstatus == II_M_TAG) {
							status = II_EQUAL;
							mainstatus = II_M_EQUAL;
						} else
							goto parseerror;
						break;
					case II_URI_PATH:
						break;
					default:
						goto parseerror;
				}
				break;
			case '[':
				switch (status) {
					case II_URI_DOMAIN:
						status = II_URI_IPV6;
						ii_b->domain.s = p + 1;
						break;
					default:
						goto parseerror;
				}
				break;
			case ']':
				switch (status) {
					case II_URI_IPV6:
						ii_b->domain.len = p - ii_b->domain.s;
						status = II_URI_PATH;
						break;
					case II_URI_IPV4:
					case II_URI_PATH:
						goto parseerror;
				}
				break;
			case ':':
				if (status == II_URI_IPV4) {
					ii_b->domain.len = p - ii_b->domain.s;
					status = II_URI_PATH;
				}
				break;
			default:
				switch (status) {
					case II_EQUAL:
					case II_LWS:
					case II_LWSCRLFSP:
						if (mainstatus == II_M_EQUAL) {
							status = II_TOKEN;
							mainstatus = II_M_TOKEN;
							ii_b->alg.s = p;
						} else
							goto parseerror;
						break;
					case II_TOKEN:
						break;
					case II_URI_DOMAIN:
						ii_b->domain.s = p;
						status = II_URI_IPV4;
					case II_URI_IPV4:
					case II_URI_IPV6:
						if (isalnum(*p)
						    || *p == '-'
						    || *p == '.'
						    || *p == ':' )
							break;
					case II_START:
						goto parseerror;
				}
				break;
		}
	}

parseerror:
		LOG( L_ERR , "ERROR: parse_identityinfo: "
		"unexpected char [%c] in status %d: <<%.*s>> .\n",
		*p,status, (int)(p-buffer), ZSW(buffer));
error:
		return p;
}


void free_identityinfo(struct identityinfo_body *ii_b)
{
	pkg_free(ii_b);
}
