/*
 * $Id$
 *
 * Digest Authentication Module 
 * 
 * Just the credential finding routines
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
 
/**
 * \file
 * 
 * SIP-to-IMS Gateway - Digest Authorization API
 * 
 * 
 * \note Taken from the auth_db SER module
 * 
 */
#include <string.h>
#include "auth_api.h"
#include "../../dprint.h"
#include "../../parser/digest/digest.h"
#include "../../sr_module.h"
#include "../../mem/mem.h"
#include "nonce.h"


/**
 * Find credentials with given realm in a SIP message header.
 * @param _m - the SIP message to look into
 * @param _realm - the realm to match in the authorization header
 * @param _hftype - the header type (HDR_AUTHORIZATION_T,HDR_PROXYAUTH_T)
 * @param _h - header pointer to fill with the match
 * @returns 0 on success, 1 if not found
 * \note This function is taken from the auth module
 */
inline int xfind_credentials(struct sip_msg* _m, str* _realm,
									hdr_types_t _hftype, struct hdr_field** _h)
{
	struct hdr_field** hook, *ptr, *prev;
	hdr_flags_t hdr_flags;
	int res;
	str* r;

	     /*
	      * Determine if we should use WWW-Authorization or
	      * Proxy-Authorization header fields, this parameter
	      * is set in www_authorize and proxy_authorize
	      */
	switch(_hftype) {
	case HDR_AUTHORIZATION_T: 
							hook = &(_m->authorization);
							hdr_flags=HDR_AUTHORIZATION_F;
							break;
	case HDR_PROXYAUTH_T:
							hook = &(_m->proxy_auth);
							hdr_flags=HDR_PROXYAUTH_F;
							break;
	default:				
							hook = &(_m->authorization);
							hdr_flags=HDR_T2F(_hftype);
							break;
	}

	     /*
	      * If the credentials haven't been parsed yet, do it now
	      */
	if (*hook == 0) {
		     /* No credentials parsed yet */
		if (parse_headers(_m, hdr_flags, 0) == -1) {
			LOG(L_ERR, "find_credentials(): Error while parsing headers\n");
			return -1;
		}
	}

	ptr = *hook;

	     /*
	      * Iterate through the credentials in the message and
	      * find credentials with given realm
	      */
	while(ptr) {
		res = parse_credentials(ptr);
		ptr->type = HDR_AUTHORIZATION_T;
		if (res < 0) {
			LOG(L_ERR, "find_credentials(): Error while parsing credentials\n");
			return (res == -1) ? -2 : -3;
		} else if (res == 0) {
			if (_realm->len) {
				r = &(((auth_body_t*)(ptr->parsed))->digest.realm);
	
				if (r->len == _realm->len) {
					if (!strncasecmp(_realm->s, r->s, r->len)) {
						*_h = ptr;
						return 0;
					}
				}
			}
			else {
				*_h = ptr;
				return 0;
			}
			
		}

		prev = ptr;
		if (parse_headers(_m, hdr_flags, 1) == -1) {
			LOG(L_ERR, "find_credentials(): Error while parsing headers\n");
			return -4;
		} else {
			if (prev != _m->last_header) {
				if (_m->last_header->type == _hftype) ptr = _m->last_header;
				else break;
			} else break;
		}
	}
	
	     /*
	      * Credentials with given realm not found
	      */
	return 1;
}

static str realm_par={"realm=\"",7};
/**
 * Find credentials with given realm in a SIP message header.
 * without parsing the credentials - just find the right 
 * authorize with stupid byte by byte comparison
 * @param _m - the SIP message to look into
 * @param realm - the realm to match in the authorization header
 * @param _hftype - the header type (HDR_AUTHORIZATION_T,HDR_PROXYAUTH_T)
 * @param _h - header pointer to fill with the match
 * @returns 0 on success, 1 if not found
 * \note This function is taken from the auth module 
 */
inline int xfind_credentials_noparse(struct sip_msg* _m, str* realm,
									hdr_types_t _hftype, struct hdr_field** _h)
{
	struct hdr_field** hook, *ptr, *prev;
	hdr_flags_t hdr_flags;
	int i,k;

	     /*
	      * Determine if we should use WWW-Authorization or
	      * Proxy-Authorization header fields, this parameter
	      * is set in www_authorize and proxy_authorize
	      */
	switch(_hftype) {
	case HDR_AUTHORIZATION_T: 
							hook = &(_m->authorization);
							hdr_flags=HDR_AUTHORIZATION_F;
							break;
	case HDR_PROXYAUTH_T:
							hook = &(_m->proxy_auth);
							hdr_flags=HDR_PROXYAUTH_F;
							break;
	default:				
							hook = &(_m->authorization);
							hdr_flags=HDR_T2F(_hftype);
							break;
	}

	     /*
	      * If the credentials haven't been parsed yet, do it now
	      */
	if (*hook == 0) {
		     /* No credentials parsed yet */
		if (parse_headers(_m, hdr_flags, 0) == -1) {
			LOG(L_ERR, "find_credentials(): Error while parsing headers\n");
			return -1;
		}
	}

	ptr = *hook;

	     /*
	      * Iterate through the credentials in the message and
	      * find credentials with given realm
	      */
	while(ptr) {
		k = ptr->body.len - realm_par.len - realm->len;
		for(i=0;i<k;i++)
		 if (strncasecmp(ptr->body.s+i,realm_par.s,realm_par.len)==0){
		 	if (strncasecmp(ptr->body.s+i+realm_par.len,realm->s,realm->len)==0){
				*_h = ptr;
				return 0;		
		 	} 
		 	else 
		 		break; 		
		 }
		 	
		
		prev = ptr;
		if (parse_headers(_m, hdr_flags, 1) == -1) {
			LOG(L_ERR, "find_credentials(): Error while parsing headers\n");
			return -4;
		} else {
			if (prev != _m->last_header) {
				if (_m->last_header->type == _hftype) ptr = _m->last_header;
				else break;
			} else break;
		}
	}
	
	     /*
	      * Credentials with given realm not found
	      */
	return 1;
}

extern int nonce_expire;
extern str secret;
#define _PRINT_MD5
#define QOP_PARAM	  ", qop=\"auth\""
#define QOP_PARAM_LEN	  (sizeof(QOP_PARAM)-1)
#define STALE_PARAM	  ", stale=true"
#define STALE_PARAM_LEN	  (sizeof(STALE_PARAM)-1)
#define DIGEST_NONCE	  " nonce=\""
#define DIGEST_NONCE_LEN  (sizeof(DIGEST_NONCE)-1)
#define DIGEST_MD5	  ", algorithm=MD5"
#define DIGEST_MD5_LEN	  (sizeof(DIGEST_MD5)-1)

/**
 * Create {WWW,Proxy}-Authenticate header field.
 * @param _retries - 
 * @param _stale -
 * @param _realm - 
 * @param _len - 
 * @param _qop
 */
inline char *build_auth_hf(int _retries, int _stale, str* _realm, 
				  int* _len, int _qop)
{
	char *hf, *p;

	     /* length calculation */
	*_len=DIGEST_NONCE_LEN
		+NONCE_LEN
		+1 /* '"' */
		+((_qop)? QOP_PARAM_LEN:0)
		+((_stale)? STALE_PARAM_LEN : 0)
#ifdef _PRINT_MD5
		+DIGEST_MD5_LEN
#endif
		;
	
	p=hf=pkg_malloc(*_len+1);
	if (!hf) {
		LOG(L_ERR, "ERROR: build_auth_hf: no memory\n");
		*_len=0;
		return 0;
	}

	memcpy(p, DIGEST_NONCE, DIGEST_NONCE_LEN);p+=DIGEST_NONCE_LEN;
	calc_nonce(p, time(0) + nonce_expire, &secret);
	p+=NONCE_LEN;
	*p='"';p++;
	if (_qop) {
		memcpy(p, QOP_PARAM, QOP_PARAM_LEN);
		p+=QOP_PARAM_LEN;
	}
	if (_stale) {
		memcpy(p, STALE_PARAM, STALE_PARAM_LEN);
		p+=STALE_PARAM_LEN;
	}
#ifdef _PRINT_MD5
	memcpy(p, DIGEST_MD5, DIGEST_MD5_LEN ); p+=DIGEST_MD5_LEN;
#endif
	*p=0; /* zero terminator, just in case */
	
	DBG("build_auth_hf(): '%s'\n", hf);
	return hf;
}

