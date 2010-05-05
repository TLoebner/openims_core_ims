/*
 * $Id: vq_time.c 579 2008-08-25 15:24:33Z vingarzan $
 *
 * Virtual Queue module
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
 *
 */

#include <stdint.h>
#include "vq_time.h"

void 
qtimerclear (struct timeval time)
{
  time.tv_sec = 0;
  time.tv_usec = 0;

  return;
}

void
qtimeradd (struct timeval *a, struct timeval *b, struct timeval *result)
{
  //printf ("adding %ld.%06ld and %ld.%06ld\n", a->tv_sec, a->tv_usec, b->tv_sec, b->tv_usec);

  result->tv_sec = 0;
  result->tv_usec = 0;

  result->tv_sec = a->tv_sec + b->tv_sec;
  result->tv_usec = a->tv_usec + b->tv_usec;

  //printf ("result->tv_usec: %06ld\n", result->tv_usec);
  if (result->tv_usec >= 1000000) {
    //printf ("Auch\n");
    result->tv_sec++;
    result->tv_usec -= 1000000;                                         
  }

  //printf ("result is: %ld.%06ld\n", result.tv_sec, result.tv_usec);
}

/**
 * Calculates 'a' - 'b' and stores the value in 'result'
 *
 */ 
void
qtimersub (struct timeval *a, struct timeval *b, struct timeval *result)
{
  result->tv_sec = a->tv_sec - b->tv_sec;
  result->tv_usec = a->tv_usec - b->tv_usec;
  if (result->tv_usec < 0) {                                              
      result->tv_sec--;                                                         
      result->tv_usec += 1000000;     
  }
}

/**
 *  Returns 0 : if the times are equal 
 *  t1 is 'before', t2 is 'after'
 *  Returns > 0 : if t2 > t1
 *  Returns < 0 : if t1 > t2
 *
 */
int
qtimecmp (struct timeval *t1, struct timeval *t2)
{
  if (t1->tv_sec == t2->tv_sec) {
    if (t1->tv_usec == t2->tv_usec) {
      return 0;
    } 
    if (t1->tv_usec > t2->tv_usec) {
      return -1;
    } else { 
      return 1;
    }
  }

  if (t1->tv_sec > t2->tv_sec) 
    return -1;
  else 
    return 1;

  // never reached
  return 0;
}

void
qtimediv (struct timeval *tv, unsigned int divisor, struct timeval *result)
{
  
  // NOTE: inspired by http://paraslash.systemlinux.org/doxygen/html/HTML/S/486.html
  
  uint64_t x = ((uint64_t)tv->tv_sec * 1000 * 1000 + tv->tv_usec) / divisor;
  
  result->tv_sec = x / 1000 / 1000;
  result->tv_usec = x % (1000 * 1000);
}
