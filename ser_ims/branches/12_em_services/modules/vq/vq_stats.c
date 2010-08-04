/*
 * $Id$
 *
 * Virtual Queue module headers
 *
 * Copyright (C) 2009-2010 FhG FOKUS
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
 * author 2009-2010 Jordi Jaen Pallares
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mod.h"
#include "vq_time.h"
#include "vq_stats.h"

// pointer to the statistics for emergency calls 
static vq_timeinfo_t *vq_stats;

// file descriptor used for the stats file
static int fd;

// TODO: pointer to the statistics for non-emergency calls
static vq_timeinfo_t *vq_stats_n;

/* number methods as power of two to allow bitmap matching */
/*
enum request_method { METHOD_UNDEF=0, METHOD_INVITE=1, METHOD_CANCEL=2, METHOD_ACK=4,
	METHOD_BYE=8, METHOD_INFO=16, METHOD_REGISTER=32, METHOD_SUBSCRIBE=64,
        METHOD_NOTIFY=128, METHOD_OTHER=256 };
*/

/** number of methods considered for the statistics */
#define MAXMETHOD 10
/** complete size in memory for the statistics object */
#define STATSIZE MAXMETHOD*sizeof(vq_timeinfo_t)

int
vq_init_vq_stats (void)
{
  int ret = 0;
  char line[120];
  char name[32] = "";
  
  vq_stats = shm_malloc (STATSIZE);
  if (!vq_stats) {
    ERR ("Could not allocate memory for the vq_stats\n");
    return -1;
  }
  
  vq_stats_n = shm_malloc (STATSIZE);
  if (!vq_stats_n) {
    ERR ("Could not allocate memory for the vq_stats\n");
    return -1;
  }
  
  strncpy (name, "/tmp/statlogXXXXXX", sizeof(name));
  fd = mkstemp (name);
  if (fd == -1) {
    LOG (L_ERR, "could not open statistics log file !\n");
    return 1;
  }
  
  // now it comes to the initialization of the statistics
  // until now, we will initialize at zero
  memset (vq_stats, 0, STATSIZE);
  memset (vq_stats_n, 0, STATSIZE);
  
  LOG (L_INFO, "Successfully allocated memory for 'vq_stats'\n");
  
  // fill the header of the file
  memset (line, 0, sizeof(line));
  strncpy (line, "UNDEF(0), INVITE(1), CANCEL(2), ACK(4), BYE(8), INFO(16), REGISTER(32), SUBSCRIBE(64), NOTIFY(128), OTHER(256)\n", sizeof(line));
  ret = write (fd, line, strlen(line));
  memset (line, 0, sizeof(line));
  strncpy (line, "method;total_calls;average time in the queue;\n", sizeof(line));
  ret = write (fd, line, strlen(line));  
 
  return 1;
}

void
vq_write_vq_stats (void)
{
  int i;
  int ret = 0;
  
  char line[220];
  vq_timeinfo_t *aux;	// for emergency
  vq_timeinfo_t *aux_n;	// for non-emergency
  
  //
  // Write to a file the statistics
  //
  
  for (i = 0 ; i < MAXMETHOD; i++) {
    aux = &vq_stats[i];
    aux_n = &vq_stats_n[i];
    //DBG ("dumping stats: aux at %p and %p\n", aux, aux_n);
    memset (line, 0, sizeof(line));
    snprintf (line, sizeof(line), "%d;%ld;%ld.%06ld;%ld.%06ld||||%d;%ld;%ld.%06ld;%ld.%06ld\n", 
	      aux->call_type, aux->total_calls, aux->average.tv_sec, 
	      aux->average.tv_usec, aux->error.tv_sec, aux->error.tv_usec,
	      aux_n->call_type, aux_n->total_calls, aux_n->average.tv_sec, 
	      aux_n->average.tv_usec, aux_n->error.tv_sec, aux_n->error.tv_usec	      
	      );	      
    ret = write (fd, line, strlen(line));
    if (ret<=0) {
      WARN ("Write error: ret = %d\n", ret);
    }
  }

  memset (line, 0, sizeof(line));
  strcpy (line, "\n-------\n");
  ret = write (fd, line, strlen(line));
  if (ret<=0) {
    WARN ("Write error: ret = %d\n", ret);
  }

  return;

}

void
vq_close_vq_stats (void)
{
  // close stats file
  close(fd);
  
  // freee allocated memory
  shm_free (vq_stats);
  INFO ("Freed vq_stats memory\n");
  
  return;
}

int 
vq_find_stat_index (int method)
{
  int ret = 0;
  
  if (method <= 1) {
    DBG ("method index is %d\n", method);
    return method;
  }
  
  switch (method) {
    
    case 2: ret = 2;
      break;      
    case 4: ret = 3;
      break;
    case 8: ret = 4;
      break;
    case 16: ret = 5;
      break;
    case 32: ret = 6;
      break;
    case 64: ret = 7;
      break;
    case 128: ret = 8;
      break;
    case 256: 
    default: ret = 9;
      break;    
    
  }
  
  DBG ("method index is %d\n", ret);
  
  return ret;

}

int 
vq_get_call_time ()
{
  LOG( L_INFO, "INFO:"M_NAME": - vq_get_call_time\n" );
  return CSCF_RETURN_TRUE;
}

int 
vq_update_stat (queueIndex_t *index, queueNode_t *node)
{
  int i;
  vq_timeinfo_t *ptr; 
  
  struct timeval res;
  struct timeval error;
  
  LOG( L_INFO, "INFO:"M_NAME": - vq_update_stats\n" );
  
  // update 'estimation' field for the node
  // we are NOT in sync with the queue index time
  gettimeofday (&index->now, NULL);
  // update the out value in the call information
  memcpy (&node->out, &index->now, sizeof(struct timeval));
  //memcpy (&reference, &index->now, sizeof(struct timeval));  // use this to keep in sync
  
  // get index
  i = vq_find_stat_index (node->type);
  
  // calculate 'out' - 'in' time --> real time
  qtimerclear (res);
  qtimersub (&index->now, &node->arrival, &res);  
  
  // calculate prediction error 
  qtimerclear (error);
  // we calculate its module
  if ( qtimecmp (&node->time, &res) > 0 ) {
    qtimersub (&res, &node->time, &error);
  } else {
    qtimersub (&node->time, &res, &error);
  }

  if (error.tv_sec < 0) {
   BUG ("error calculating the 'abs(error)'\n");    
  }

  LOG (L_INFO, "Call of type %d processed in %ld.%06ld\n", node->type, res.tv_sec, res.tv_usec);
  LOG (L_INFO, "Estimation error: %ld.%06ld\n", error.tv_sec, error.tv_usec);
  
  // update stats
  if (node->prio == EMERGENCY) {
    // select the emergency struct
    DBG ("Updating emergency stats\n");
    ptr = &vq_stats[i];
  } else {
    // use the non-emergency struct
    DBG ("Updating non-emergency stats\n");
    ptr = &vq_stats_n[i];
  }

  DBG ("Call statistics at %p\n", ptr);
  
  /* Check sanity */
  if (res.tv_sec < 0) {
    WARN ("Error obtaining call time: t < 0\n");
    DBG ("Setting node->stat to true\n");
    node->stat = 1;
    if (ptr->total_calls > 0) {
      //ptr->total_calls--;
    }
    
    return CSCF_RETURN_TRUE;
  }  

  vq_update_average (&ptr, &res, &error, node->type);
  vq_write_vq_stats ();
  
  DBG ("Setting node->stat to true\n");
  node->stat = 1;
  
  return CSCF_RETURN_TRUE;
}

/**
 * Funtion that provides the average time in the queue for each type of calls.
 * 
 * TODO: make this values depend on the current load.
 * TODO: get some initialization values
 *
 */
int 
vq_set_call_time (queueNode_t **node)
{
  int index;
  vq_timeinfo_t *stats = vq_stats;
  queueNode_t *n = *(node);
  
  if (!n) {
    LOG (L_INFO, "INFO:"M_NAME" :set_call_time: Wrong pointer !\n");
    return -1;
  }

  LOG (L_INFO, "INFO:"M_NAME" :Setting call time for type %d\n", n->type);

  // find the index in the vector with the stats
  index = vq_find_stat_index (n->type);
  
  // update pointer
  if (n->prio == EMERGENCY) {
    DBG ("vq_set_call_time: selected emergency statistics\n");
    stats = (vq_timeinfo_t *)&vq_stats[index];
  } else {
    DBG ("vq_set_call_time: selected non-emergency statistics\n");
    stats = (vq_timeinfo_t *)&vq_stats_n[index];
  }

  if (stats->total_calls == 0) {
    // provide standard value
    DBG ("vq_set_call_time: 'average' data empty ---> setting long term prediction for first call\n");
    n->time.tv_sec = 0;
    n->time.tv_usec = 7500;
  } else {
    // provide average value
    DBG ("vq_set_call_time: setting average value for time call\n");
    memcpy (&n->time, &stats->average, sizeof(struct timeval));    
  }   
  
  stats->total_calls = stats->total_calls+1;
  INFO ("current stats->total_calls = %ld\n", stats->total_calls);  

  LOG (L_INFO, "INFO:"M_NAME" :set_call_time: New call time is %ld.%06ld\n\n", n->time.tv_sec, n->time.tv_usec);

  return 1;

}

int
vq_update_average (vq_timeinfo_t **ptr, struct timeval *res, struct timeval *error, int calltype)
{
  vq_timeinfo_t *info = *(ptr);
  struct timeval updated;
  
  INFO ("vq_update_average: ---> total_calls : %ld\n", info->total_calls);
  
  // call initialization: use 'res' as a estimation for the scheduled time
  if (info->total_calls == 0) {
    DBG ("initializing stats for new call type\n");
    gettimeofday (&info->start, NULL);
    info->call_type = calltype;
    memcpy (&info->average, res, sizeof (struct timeval));   
    INFO ("Finished vq_update_average\n");
    return 1;
  }
  
  qtimerclear (updated);
  DBG ("timeinfo at %p\tcall type: %d\ttotal calls: %ld\n", info, info->call_type, info->total_calls);

  DBG ("Former average time: %ld.%06ld\n", info->average.tv_sec, info->average.tv_usec);
  
#define JORDI 1
  
#ifdef JORDI  
  INFO ("Calculating weighted statistics\n");
  // averaging 'a la' jordi
  // sum = sum + res
  // average = sum / total_calls
  qtimeradd (&info->sum, res, &updated);
  DBG ("vq_update_average: dividing: %ld.%06ld through %ld...\n", updated.tv_sec, updated.tv_usec, info->total_calls);
  qtimediv (&updated, info->total_calls, &info->average);
  memcpy (&info->sum, &updated, sizeof(struct timeval));
#else 
  INFO ("Calculating short term average\n");
  // averaging 'a la' yacine
  // average = ( average + res ) 1/2
  qtimeradd (&info->average, res, &updated);
  qtimediv (&updated, 2, &info->average);
#endif

  memcpy (&info->error, error, sizeof(struct timeval));
  DBG ("Updated average time: %ld.%06ld\n", info->average.tv_sec, info->average.tv_usec);
  DBG ("Total calls: %ld\n", info->total_calls);

  INFO ("Finished vq_update_average\n");
  
  return 1;  
  
}
