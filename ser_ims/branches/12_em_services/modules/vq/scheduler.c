
/*
 * $Id$
 *
 * Virtual Queue Call Scheduler functions
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
 * author Jordi Pallares
 */

#include "mod.h"

#include "queue.h"
#include "vq_stats.h"

/**
 * take new call and return the scheduling time. 
 * use this function after adding the node to the queue.
 *
 * update times in the queue done in <i>increase_index_values</i>
 *
 */
int
vq_schedule_call (queueIndex_t *index, queueNode_t **callin)
{
//  struct timeval aux;//, *ptr;
  struct timeval now;

  queueNode_t *call = (*callin);

  // node is already placed in the queue
  queueNode_t *prev = call->prev;

  qtimerclear (call->scheduled);
  qtimerclear (now);
  
  gettimeofday (&now, NULL);
  
  DBG ("vq_schedule call\n");
  
  //ptr = &now;
  //DBG ("*** Current time: %ld.%06ld\n", ptr->tv_sec, ptr->tv_usec);
  
  switch (call->prio) {
    case M:
    case N:
      if (prev) {
	// There is a previous node in the queue
	DBG ("queue not empty, scheduling call after previous call\n");
	if (prev->out.tv_sec != 0) {
	  // if the call has already a 'out' time
	  memcpy (&call->scheduled, &prev->out, sizeof (struct timeval));
	} else {
	  // if we only have the 'scheduled' value of the previous call
	  // then call->scheduled = prev->scheduled + prev->time
	  qtimeradd (&prev->scheduled, &prev->time, &call->scheduled);
	  //memcpy (&call->scheduled, &prev->scheduled, sizeof (struct timeval)); 
	}  
      } else {
	DBG ("queue empty, scheduling call now\n");
	vq_set_call_time (&call);
	memcpy (&call->scheduled, &now, sizeof (struct timeval));
      }
      
      break;

    default:
      // add as normal call or dismiss
      WARN ("Unknown type of call: %d ", call->prio);
      //WARN ("Adding as non-prioritized call\n");
      //memcpy (&call->scheduled, &prev->out, sizeof (struct timeval));
      break;    
  }

  //qtimeradd (&call->scheduled, &call->time, &call->out);

  DBG ("vq_schedule_call: call at %p:\n\tarrived:\t%ld.%06ld\n\tcall duration:\t%ld.%06ld\n\tcall scheduled:\t%ld.%06ld\n", 
	     call, call->arrival.tv_sec, call->arrival.tv_usec, call->time.tv_sec, call->time.tv_usec, 
	     call->scheduled.tv_sec, call->scheduled.tv_usec);

  LOG (L_INFO, "INFO:"M_NAME" : *** Scheduling in %ld.%06ld\n\n", call->scheduled.tv_sec, call->scheduled.tv_usec);

  return 1;
  
}

/**
 * Increase the number of rescheduled calls in the queue index 
 * and check the values for integrity 
 *
 * @param **index double pointer to the queue index
 *
 */
void 
vq_increase_rescheduled_calls (queueIndex_t **index)
{
  queueIndex_t *ix = *(index);
  
  ix->rescheduled++;
  LOG (L_INFO, "INFO:"M_NAME" :\n\n\n\n*****---> rescheduled: %d, incoming: %d\n", 
	  ix->rescheduled, ix->incoming);
  
  if (ix->rescheduled <= ix->incoming)
    LOG (L_ERR, "ERROR:"M_NAME" :rescheduled nodes greater than total nodes");
  
  //printf ("WARNING: rescheduled nodes greater than total nodes\n");
      
  return;
}

/**
 *  Re-schedule non-emergency nodes
 *
 *  @param *ix pointer to the queue index
 *  @param *newnode pointer to the emergency node 
 */
int
vq_reschedule_call (queueIndex_t *ix, queueNode_t *newnode)
{
  queueNode_t *moved;	// points to the queuenode that will be moved to the end of the queue 
  int num_nodes = 1;	// num of nodes to be rescheduled - minimum one
  int i = 0; 		// iterator
  struct timeval total;	// holds the total time to be rescheduled
  struct timeval inc;	// holds the times of the non prioritized rescheduled calls
  struct timeval temp;	// holds temporary timekly information
  queueNode_t *res = newnode->next;	// pointer to go through the queue

  LOG (L_INFO, "+++++++++++++++++ Re-scheduling of non-prioritized nodes... ++++++++++++++++++++++++++\n");

  //print_queue (ix);
  lock_get (ix->lock);
  
  DBG ("Re-scheduling nodes for node->time : %ld.%06ld\n", newnode->time.tv_sec, newnode->time.tv_usec);

  // copy to total the total time to be re-scheduled
  // the value is the time of the new prioritized node
  memcpy (&total, &newnode->time, sizeof (struct timeval));

  // copy to inc the time of the first node to be re-scheduled
  memcpy (&inc, &res->time, sizeof (struct timeval));
  
  DBG ("Entering loop: checking first neighbouring non-prioritized node\n");
  DBG ("===== comparing times: %ld.%06ld and %ld.%06ld\n", total.tv_sec, total.tv_usec, inc.tv_sec, inc.tv_usec);
    
  //
  // start a loop to find out how many nodes are to be moved to the end of the queue
  //
  while ( qtimecmp (&total, &inc) < 0 ) {
     
    num_nodes++; // increase total rescheduled nodes
    res = res->next; // advance to the next non-prioritized nodes
    if (!res) {
      DBG ("Reached the end of the queue !\n");
      break;
    }
    qtimeradd (&inc, &res->time, &temp);
    memcpy (&inc, &temp, sizeof(struct timeval));    
    DBG ("===== comparing times: %ld.%06ld and %ld.%06ld\n", total.tv_sec, total.tv_usec, inc.tv_sec, inc.tv_usec);
    
  }
  
  //
  // do the re-scheduling
  //
  DBG ("Total nodes to be re-scheduled: %d\n", num_nodes);
  moved = newnode->next;	// re-init pointer to the first non-prioritized
  for (; i < num_nodes; i++ ) {
    DBG ("loop: %d iteration\n", (i+1));
    // delete node from queue
    vq_delete_node (ix, moved);
    // update time values
    vq_decrease_index_values (&ix, moved);
    // mark node as re-scheduled
    moved->moved++;
    
    // add to queue and increase_index
    
    if (ix->nodes == 1) {  
      // if only had one node in the queue
      // the first node goes at the end of the queue
      queueNode_t *last = ix->last;
      moved->next = NULL;
      moved->prev = ix->last;
      last->next = moved;
      ix->last = moved;

      // re-schedule 
      DBG ("last->out: %ld.%06ld newnode->out: %ld.%06ld\n", last->out.tv_sec, last->out.tv_usec, newnode->out.tv_sec, newnode->out.tv_usec);
      if (!qtimecmp (&last->out, &newnode->out)) {
	memcpy (&moved->scheduled, &last->out, sizeof (struct timeval));
      } else {
	memcpy (&moved->scheduled, &newnode->out, sizeof (struct timeval));
      }
      qtimeradd (&moved->scheduled, &moved->time, &moved->out);
      DBG ("rescheduled in %ld.%06ld\n\n", moved->scheduled.tv_sec, moved->scheduled.tv_usec);
      
      // increase index
      vq_increase_index_values (&ix, moved);
      vq_increase_rescheduled_calls (&ix);
      
    } else {
      lock_release (ix->lock);
      vq_put_node (ix, moved);
      lock_get (ix->lock);
      // re-schedule call
      vq_schedule_call (ix, &moved);
      vq_increase_rescheduled_calls (&ix);
    }
    
    moved = newnode->next;

  }
  
  DBG ("loop: finished\n");
  lock_release (ix->lock);
  
  DBG ("+++++++++++++++++ finished re-scheduling +++++++++++++++\n");
  //vq_print_queue (ix);

  return 0;
}

