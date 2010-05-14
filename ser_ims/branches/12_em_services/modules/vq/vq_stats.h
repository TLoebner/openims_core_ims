/*
 * $Id: vq_stats.h 579 2008-08-25 15:24:33Z vingarzan $
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


#ifndef VQ_STATS_H
#define VQ_STATS_H

#include "mod.h"
#include "vq_time.h"
#include "../../parser/msg_parser.h"

struct vq_timeinfo {
  int call_type;		/// SIP method
  long total_calls;		/// Total number of calls
  struct timeval sum;		/// Keeps the sum of all times to calculate the average
  struct timeval start;		/// Start time
  struct timeval average;	/// time average
  struct timeval average10;	/// time average last 10 calls
  struct timeval average100;	/// time average last 100 calls
  struct timeval average1000;	/// time average last 1000 calls
  struct timeval error;		/// estimation error measure = abs ('out time' - 'scheduled time')
};

typedef struct vq_timeinfo vq_timeinfo_t;

int vq_get_call_time ();
int vq_update_stat (queueIndex_t *index, queueNode_t *node);
int vq_set_call_time (queueNode_t **node);
int vq_find_stat_index (int method);
void vq_close_vq_stats (void);
int vq_init_vq_stats (void);
int vq_update_average (vq_timeinfo_t **ptr, struct timeval *res, struct timeval *error, int calltype);

#endif // VQ_STATS_H

