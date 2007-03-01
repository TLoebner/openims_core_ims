/*
 * Presence Agent, module interface
 *
 * $Id$
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

#ifndef PA_MOD_H
#define PA_MOD_H

#include "../../parser/msg_parser.h"
#include "../tm/tm_load.h"
#include "../../db/db.h"
#include "../dialog/dlg_mod.h"
#include "auth.h"

/* we have to use something from this module */
#include "../xcap/xcap_mod.h"

extern int default_expires;
extern int max_subscription_expiration;  /* max expires value for SUBSCRIBE */
extern int max_publish_expiration;  /* max expires value for PUBLISH */
extern double default_priority;
extern int timer_interval;

/* TM bind */
extern struct tm_binds tmb;

extern dlg_func_t dlg_func;

/* DB module bind */
extern db_func_t pa_dbf;
extern db_con_t* pa_db;
extern fill_xcap_params_func fill_xcap_params;

/* PA database */
extern int use_db;
extern int use_place_table;
extern str db_url;
extern str pa_domain;
extern char *presentity_table;
extern char *presentity_contact_table;
extern char *presentity_notes_table;
extern char *extension_elements_table;
extern char *watcherinfo_table;
extern char *place_table;
extern char *tuple_notes_table;
extern char *tuple_extensions_table;

extern int use_bsearch;
extern int use_location_package;
extern auth_params_t pa_auth_params;
extern auth_params_t winfo_auth_params;
extern int watcherinfo_notify;
extern int use_callbacks;
extern int subscribe_to_users;
extern str pa_subscription_uri;
extern int use_offline_winfo;
extern char *offline_winfo_table;
extern int ignore_408_on_notify;

extern str pres_rules_file; /* filename for XCAP queries */
db_con_t* create_pa_db_connection();
void close_pa_db_connection(db_con_t* db);

#endif /* PA_MOD_H */
