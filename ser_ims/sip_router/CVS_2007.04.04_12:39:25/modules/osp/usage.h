/*
 * ser osp module. 
 *
 * This module enables ser to communicate with an Open Settlement 
 * Protocol (OSP) server.  The Open Settlement Protocol is an ETSI 
 * defined standard for Inter-Domain VoIP pricing, authorization
 * and usage exchange.  The technical specifications for OSP 
 * (ETSI TS 101 321 V4.1.1) are available at www.etsi.org.
 *
 * Uli Abend was the original contributor to this module.
 * 
 * Copyright (C) 2001-2005 Fhg Fokus
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

#ifndef OSP_USAGE_MOD_H
#define OSP_USAGE_MOD_H

#include "osp/osp.h"
#include "osp_mod.h"
#include "../../sr_module.h"

/* This module reports originating and terminating call set up and duration usage
 * for OSP transactions.
 *
 * Call set-up usage is reported based on the osp_dest structures stored as AVPs.
 * It includes OSP transaction id, response codes, start time, alert time,
 * connect time, etc.
 *
 * Duration usage is reported based on the OSP cookie recorded into the route set
 * (using add_rr_param) after requesting routing/authorization on the originating
 * side, and validating authorization on the terminating side.  It include 
 * OSP transaction id, duration, stop time, etc.
 * 
 * Actual conversation duration maybe calculated using connect time (from the call
 * set up usage) and stop time (from the duration usage). 
 */
void record_orig_transaction(struct sip_msg* msg, OSPTTRANHANDLE transaction, char* uac, char* from, char* to, time_t time_auth);
void record_term_transaction(struct sip_msg* msg, OSPTTRANHANDLE transaction, char* uac, char* from, char* to, time_t time_auth);
int  reportospusage(struct sip_msg* msg, char* ignore1, char* ignore2);
void reportOrigCallSetUpUsage();
void reportTermCallSetUpUsage();

#endif
