/*
 * $Id: vq_time.h 579 2008-08-25 15:24:33Z vingarzan $
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


#ifndef VQ_TIME_H
#define VQ_TIME_H

#include <sys/time.h>

/** Procedure to set a timer to zero 
 * @param struct timeval time 
 */
void qtimerclear (struct timeval time);

/** Procedure to calculate the addition of 'a' and 'b' and stores it in 'result' */
void qtimeradd (struct timeval *a, struct timeval *b, struct timeval *result);

/** Procedure to calculate the substraction of 'a' and 'b' and stores it in 'result' */
void qtimersub (struct timeval *a, struct timeval *b, struct timeval *result);

/** Procedure to compare two times. 
 * @param struct timeval *t1
 * @param struct timeval *t2
 * @return 0 if both are the same
 * @return < 0 if t1 > t2
 * @return > 0 if t1 < t2
 */
int qtimecmp (struct timeval *t1, struct timeval *t2);

/** Procedure to divide a tv through an integer divisor 
 * @param struct timeval *t timeval to be divided
 * @param divisor the divisor
 * @param struct timeval *result pointer to where the result will be stored
 */
void qtimediv (struct timeval *t, unsigned int divisor, struct timeval *result);


#endif /* VQ_TIME_H */
