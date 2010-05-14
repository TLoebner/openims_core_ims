/*
 * $Id: vq_queueid.h 579 2008-08-25 15:24:33Z vingarzan $
 *
 * Virtual Queue Call Identity calculation 
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


#ifndef VQ_QUEUEID_H
#define VQ_QUEUEID_H

#include <sys/time.h>

#include "../../md5global.h"
#include "../../md5.h"
#include "../../parser/msg_parser.h"

#define HASHLEN 16
 
struct queueID { 
   char id[HASHLEN];		/**< id in binary */
   char strid[2*HASHLEN+1];	/**< id in hexadecimal format */
   struct timeval time;		/**< time information */
};
 
typedef struct queueID queueID_t;

queueID_t *vq_get_call_id (struct sip_msg *msg);
void vq_free_call_id (queueID_t *msg);

#endif // VQ_QUEUEID_H


