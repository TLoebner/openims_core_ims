/*
 * $Id$
 *
 * Copyright (C) 2005-2006 iptelorg GmbH
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
 * History:
 * --------
 *	2006-06-16  static buffer for select results (mma)
 *	            each process owns a separate space
 *	            each request starts using the buffer from the start
 *
 */

#ifndef SELECT_BUFFER_H
#define SELECT_BUFFER_H

/*
 * Request for space from buffer
 *
 * Returns:  NULL  memory allocation failure (no more space)
 *           pointer to the space on success
 */

char* get_static_buffer(int req_size);

/* Internal function - called before request is going to be processed
 *
 * Reset offset to unused space
 */

int reset_static_buffer();

int str_to_static_buffer(str* res, str* s);
int int_to_static_buffer(str* res, int val);
int uint_to_static_buffer(str* res, unsigned int val);
int uint_to_static_buffer_ex(str* res, unsigned int val, int base, int pad);

#endif /* SELECT_BUFFER_H */
