/*
 * $Id$
 *
 * Virtual Queue module - logging and CSV generation utilities
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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include "mod.h"
#include "queue.h"
#include "vq_logging.h"

#define LINELEN 120

static struct timeval initial_time;
static int fd;
//static int iter;

/** Procedure to initialize the logging initial time */
void 
vq_set_initial_log_time (void)
{
    gettimeofday (&initial_time, NULL);
    LOG (L_INFO, "Initial time for logging: %ld.%06ld\n", initial_time.tv_sec, initial_time.tv_usec);
//    iter = 1;
}

/** procedure to open the log file */
int
vq_open_logfile (void)
{
  int ret = 0;
  char name[16] = "";
  char line[LINELEN];
  
  vq_set_initial_log_time ();
  
  // TODO: add filepath parameter in the configuration of the module
  // for the time being let's use the /tmp directory
  strcpy (name, "/tmp/zlogXXXXXX");
  fd = mkstemp (name);
  if (fd == -1) {
    LOG (L_ERR, "could not open log file !\n");
    return (-1);
  }
  
  LOG (L_INFO, "Opened virtual queue's log file\n");
  
  memset (line, 0, sizeof(line));
  strncpy (line,  
	   "call#;total nodes;M nodes in queue;N nodes in queue;total rescheduled;total processed;total lost;time elapsed;PID;\n",
	   LINELEN);
	   
  ret = write (fd, line, strlen(line));
  if (ret < 1) {
    LOG (L_ERR, "ERR:"M_NAME":Error writing to vq CSV file\n");
  } else {
    LOG (L_INFO, "INFO:"M_NAME":Written %d bytes to vq CSV file\n", ret);
  }
  
  return fd;
}

int
vq_add_log_message (queueIndex_t *index)
{
  size_t len = 0;
  size_t ret = 0;
  struct timeval elapsed;
  char msg[BUFSIZE];
  
  // fill in 'msg'
  // format:
  // queueID; total nodes in queue; num of emergency nodes; num of normal nodes; num of rescheduled nodes; processed calls; elapsed time;
  qtimerclear (elapsed);
  qtimersub (&index->now, &initial_time, &elapsed);
  
  memset (msg, 0, BUFSIZE);
  
  //lock_get (index->lock);  
  
  vq_print_queue_status (index);
  
  snprintf (&msg[0], BUFSIZE, "%d;%d;%d;%d;%d;%d;%d;%ld.%06ld;<%d>;\n", 
	    //index->queueID,	// queue ID : only one queue
	    index->incoming,	// total incoming calls
	    index->nodes,	// total calls in queue
	    index->nodesM,	// total emergency nodes in queue
	    index->nodesN,	// total normal nodes in queue
	    index->rescheduled,	// total rescheduled calls
	    index->processed,	// total processed calls
	    index->lost,	// total lost calls
	    elapsed.tv_sec,	// elapsed time
	    elapsed.tv_usec,
	    getpid());		// pid for debugging purposes
	    
  //lock_release (index->lock); 
  
  len = strlen (msg);
  DBG ("Writing %d bytes to the logfile: \"%s\"", len, msg);
  ret = write (fd, msg, len);
  INFO ("written %d bytes to the vq logfile\n", ret);

//  iter++;
  
  return ret;
}

void
vq_close_logfile (void)
{
  close (fd);
  
  return;
}

//
// Procedures to show results
//

/** prints the current queue status */
void 
vq_print_queue_status (queueIndex_t *ix)
{
  struct timeval result;

  qtimerclear (result);

  //lock_get (ix->lock);

  qtimeradd (&ix->totalM, &ix->totalN, &result);

  DBG ("Total nodes: %d\n", (ix->nodesM + ix->nodesN));
  DBG ("\temergency: %d normal: %d\n", ix->nodesM, ix->nodesN);
  DBG ("\ttotal emergency time: %ld.%06ld\n", ix->totalM.tv_sec, ix->totalM.tv_usec);
  DBG ("\ttotal normal time: %ld.%06ld\n", ix->totalN.tv_sec, ix->totalN.tv_usec);
  DBG ("\ttotal time: %ld.%06ld\n\n", result.tv_sec, result.tv_usec);
  DBG ("current time: %ld.%06ld\n", ix->now.tv_sec, ix->now.tv_usec);
  DBG ("incoming calls: %u\tprocessed calls: %u\trescheduled calls: %u\tlost calls: %u\n\n", 
	ix->incoming, ix->processed, ix->rescheduled, ix->lost);
 
  //lock_release (ix->lock);
	
  return;
}

void
vq_print_node_info (queueNode_t *node)
{
  queueID_t *tmp = &node->id;
  DBG ("[DEBUG] Node Info:\n");
  DBG ("\tNode ID:      %s at %p", tmp->strid, node);
  if (node->moved) 
    DBG (" ---> Re-scheduled Node !");
  DBG ("\n");
  DBG ("\tPointers: next %p, prev %p\n", node->next, node->prev);
  DBG ("\tType: 	%d\n", node->type);
  DBG ("\tPriority:     %d\n", node->prio);
  DBG ("\tArrival time: %ld.%06ld\n", node->arrival.tv_sec, node->arrival.tv_usec);
  DBG ("\tTime info:    %ld.%06ld\n", node->time.tv_sec, node->time.tv_usec);
  DBG ("\tScheduled:    %ld.%06ld\n", node->scheduled.tv_sec, node->scheduled.tv_usec);
  DBG ("\tOut time:     %ld.%06ld\n", node->out.tv_sec, node->out.tv_usec);
  DBG ("\tStatistic:    %d\n\n", node->stat);
  
  return;
}

void
vq_print_queue (queueIndex_t *index)
{
  queueNode_t *ptr = index->first;

  DBG ("\n******** Print Queue Info:\n\n");

  DBG ("\ncurrent time: %ld.%06ld\n", index->now.tv_sec, index->now.tv_usec);
  DBG ("\nfirst: %p\tlast: %p\n\n", index->first, index->last);

  while (ptr != NULL) {
    vq_print_node_info (ptr);
    ptr = ptr->next;
  }

  vq_print_queue_status (index);

  DBG ("******** End of Queue Info\n");
}

