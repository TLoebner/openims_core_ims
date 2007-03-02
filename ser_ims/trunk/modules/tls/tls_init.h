/*
 * $Id$
 * 
 * TLS module - OpenSSL initialization funtions
 *
 * Copyright (C) 2001-2003 FhG FOKUS
 * Copyright (C) 2004,2005 Free Software Foundation, Inc.
 * Copyright (C) 2005,2006 iptelorg GmbH
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

#ifndef _TLS_INIT_H
#define _TLS_INIT_H

#include <openssl/ssl.h>
#include "../../ip_addr.h"
#include "tls_domain.h"

#ifndef OPENSSL_NO_KRB5
/* enable workarround for openssl kerberos wrong malloc bug
 * (kssl code uses libc malloc/free/calloc instead of OPENSSL_malloc & 
 * friends)*/
#define TLS_KSSL_WORKARROUND
extern int openssl_kssl_malloc_bug; /* is openssl bug #1467 present ? */
#endif
extern int openssl_mem_threshold1; /* low memory threshold for connect */
extern int openssl_mem_threshold2; /* like above but for other tsl operations */


extern int tls_disable_compression; /* by default enabled */
extern int tls_force_run; /* by default disabled */

extern const SSL_METHOD* ssl_methods[];


/*
 * just once, initialize the tls subsystem 
 */
int init_tls_h(void);


/*
 * just once before cleanup 
 */
void destroy_tls_h(void);


/*
 * for each socket 
 */
int tls_h_init_si(struct socket_info *si);

/*
 * Make sure that all server domains in the configuration have corresponding
 * listening socket in SER
 */
int tls_check_sockets(tls_cfg_t* cfg);

#endif /* _TLS_INIT_H */
