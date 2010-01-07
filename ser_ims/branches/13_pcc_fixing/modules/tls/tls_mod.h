/*
 * $Id$
 *
 * TLS module - module interface
 *
 * Copyright (C) 2001-2003 FhG FOKUS
 * Copyright (C) 2004,2005 Free Software Foundation, Inc.
 * Copyright (C) 2005 iptelorg GmbH
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


#ifndef _TLS_MOD_H
#define _TLS_MOD_H

#include "../../str.h"
#include "../../locking.h"
#include "tls_domain.h"

extern int tls_handshake_timeout;
extern int tls_send_timeout;
extern int tls_con_lifetime;
extern int tls_log;
extern int tls_session_cache;
extern str tls_session_id;

/* Current TLS configuration */
extern tls_cfg_t** tls_cfg;
extern gen_lock_t* tls_cfg_lock;

extern tls_domain_t cli_defaults;
extern tls_domain_t srv_defaults;

extern str tls_cfg_file;

#endif /* _TLS_MOD_H */
