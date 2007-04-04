/* 
 * $Id$ 
 *
 * Copyright (C) 2001-2003 FhG FOKUS
 * Copyright (C) 2006-2007 iptelorg GmbH
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

/** \ingroup DB_API @{ */

#include <string.h>
#include <stdlib.h>
#include "../mem/mem.h"
#include "../dprint.h"
#include "db_con.h"


/*
 * Default implementation of the connect function is noop,
 * db drivers can override the function pointer in db_con
 * structures
 */
static int db_con_connect(db_con_t* con)
{
	return 0;
}


/*
 * Default implementation of the disconnect function is noop,
 * db drivers can override the function pointer in db_con
 * structures
 */
static void db_con_disconnect(db_con_t* con)
{
}


/*
 * Create a new generic db_con structure representing a
 * database connection and call the driver specific function
 * in the driver that is associated with the structure based
 * on the scheme of uri parameter
 */
db_con_t* db_con(db_ctx_t* ctx, db_uri_t* uri)
{
    db_con_t* r;

    r = (db_con_t*)pkg_malloc(sizeof(db_con_t));
    if (r == NULL) {
		ERR("db_con: No memory left\n");
		goto error;
    }

    memset(r, '\0', sizeof(db_con_t));
	if (db_gen_init(&r->gen) < 0) goto error;

    r->uri = uri;
	r->ctx = ctx;
	r->connect = db_con_connect;
	r->disconnect = db_con_disconnect;

	/* Call db_ctx function if the driver has it */
	if (db_drv_call(&uri->scheme, "db_con", r, ctx->con_n) < 0) {
		goto error;
	}

	return r;

 error:
	if (r) {
		db_gen_free(&r->gen);
		pkg_free(r);
	}
	return NULL;
}


/*
 * Releaase all memory used by the structure
 */
void db_con_free(db_con_t* con)
{
    if (con == NULL) return;
	db_gen_free(&con->gen);
	if (con->uri) db_uri_free(con->uri);
    pkg_free(con);
}

/** @} */
