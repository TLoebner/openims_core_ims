/*
 * $Id: vq_logging.h 579 2008-08-25 15:24:33Z vingarzan $
 *
 * Virtual Queue module headers
 *
 * Copyright (C) 2009-2010 Jordi Jaen Pallares
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


#ifndef VQ_LOGGING_H
#define VQ_LOGGING_H

#include "vq_mod.h"
#include "vq_time.h"
#include "../../dprint.h"
#include "../../lock_alloc.h"
#include "vq_mod.h"

void vq_set_initial_log_time (void);

int vq_open_logfile (void);

int vq_add_log_message (queueIndex_t *index);

void vq_close_logfile (void);

#endif /* VQ_LOGGING_H */
