/*
 * $Id$
 *
 * Copyright (C) 2006 iptelorg GmbH
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
 *
 */

#include "../../db/db.h"
#include "permissions.h"
#include "im_db.h"
#include "im_rpc.h"

const char* im_reload_doc[2] = {
	"Reloads ipmatch cache",
	0
};

/* XML RPC function to reload impatch cache */
void im_reload(rpc_t *rpc, void *c)
{

	if (db_mode != ENABLE_CACHE) {
		rpc->fault(c, 400, "Database cache is not enabled");
		return;
	}

	/* connect to the DB */
	db_handle = perm_dbf.init(db_url);
	if (!db_handle) {
		LOG(L_ERR, "ERROR: Unable to connect to database\n");
		rpc->fault(c, 400, "Reloading failed");
		return;
	}

	if (reload_im_cache()) {
		/* error occured during the reload */
		LOG(L_ERR, "ERROR: Reloading of ipmatch cache failed\n");
		rpc->fault(c, 400, "Reloading failed");
	} else {
		/* reload is successful */
		LOG(L_INFO, "INFO: ipmatch cache is reloaded\n");
	}

	/* close DB connection */
	perm_dbf.close(db_handle);
	db_handle = 0;

}
