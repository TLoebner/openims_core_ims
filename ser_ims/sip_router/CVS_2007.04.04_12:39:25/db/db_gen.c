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
#include "db_gen.h"

int db_payload_idx = 0;


/*
 * Initialize a db_gen structure and make space for the data
 * from n database drivers
 */
int db_gen_init(db_gen_t* gen)
{
	memset(gen, '\0', sizeof(db_gen_t));
	return 0;
}


/*
 * Free all memory allocated by a db_gen structure
 */
void db_gen_free(db_gen_t* gen)
{
	int i;

	/* Dispose all the attached data structures */
	for(i = 0; i < DB_PAYLOAD_MAX && gen->data[i]; i++) {
		if (gen->data[i]) gen->data[i]->free(gen, gen->data[i]);
	}
}

/** @} */
