/*
 * $Id$
 *
 * Virtual Queue queue functions
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
#include "sip.h"
#include "queue.h"
#include "vq_queueid.h"
#include "vq_stats.h"

extern struct timeval tv_timeout;	
extern struct vq_to *timeout;

int vq_check_retransmission (queueIndex_t *index, queueID_t *id)
{
  queueNode_t *inqueue;	// check retransmission if the same node is already in the queue
  
  // lookup the id in the queue
  inqueue = vq_get_node_by_id_safe (index, id);
  
  if (inqueue) {
    INFO ("Detected a packet retransmission for \"%.*s\"\n", 2*HASHLEN, id->strid);
    // check whether the scheduled time corresponds to 'now'
    // if yes, route to the normal route -> return TRUE
    // if not, send another 503
  }

  return 1;
}


/** initializes a queue index */
queueIndex_t * vq_init_index (char *name)
{
  queueIndex_t *index = NULL;
  
  index = shm_malloc (sizeof(queueIndex_t));
  if (!index) {
    LOG (L_ERR, "ERROR:"M_NAME": Could not allocate memory\n");
    return NULL;
  }  
  
  memset (index, 0, sizeof(queueIndex_t));
  memset (index->queueID, 0, MAXNAME);
  
  // create and init queue lock
  index->lock = lock_alloc();
  if (index->lock == 0){
	  ERR ("E_OUT_OF_MEM while allocating queue index lock");
	  shm_free (index);
	  return NULL;
  }
  
  if (lock_init (index->lock) == 0 ){
	  lock_dealloc (index->lock);
	  shm_free (index);
	  return NULL;
  }
  
  // TODO: write in the name of the queue from the cfg file
  if (!name) {
    strcpy (index->queueID, "queue1");
  } else {
    strncpy (index->queueID, name, MAXNAME-1);
  }  
  
  index->first = NULL;
  index->last = NULL;
  qtimerclear (index->totalM);
  qtimerclear (index->totalN);
  index->nodes = 0;
  index->nodesM = 0;
  index->nodesN = 0;

  gettimeofday(&index->now, NULL);
  struct timeval aux = index->now;
  LOG (L_INFO, "INFO:"M_NAME": Queue index initialized at %ld.%06ld\n", aux.tv_sec, aux.tv_usec);
  
  return index;
}

/** returns the first element on the queue and deletes it from the queue */
queueNode_t * vq_pop_node (queueIndex_t *ix)
{
  queueNode_t *aux;
  queueNode_t *node;
  queueID_t *ident;
    
//  lock_get (ix->lock);
  
  node = ix->first;
  ident = &node->id;
  
  LOG (L_INFO, "INFO:"M_NAME": pop_node: popping node id: %s, next: %p, prev: %p\n", 
	  ident->strid, node->next, node->prev);
	  
  aux = node->next;
  
  if (aux) {
    LOG (L_INFO, "INFO:"M_NAME": pop_node: next node id: %*.s, next: %p, prev: %p\n", HASHLEN, aux->id.id, aux->next, aux->prev);
    aux->prev = NULL;
  } else {
    LOG (L_INFO, "INFO:"M_NAME": removing the only node in the queue\n");
  }

  ix->first = aux;

//  lock_release (ix->lock);
  
  return node;
}


/** updates values in the queue index after deleting a queue node 
 *
 * NOTE: the calling function has already locked the queue 
 */
void vq_decrease_index_values (queueIndex_t **index, queueNode_t *oldnode)
{
  struct timeval result;
  queueIndex_t *ix = *(index);
  
  //LOG (L_INFO, "INFO:"M_NAME": vq_decrease_index_values... ");

  qtimerclear (result);

  ix->nodes--;

  switch (oldnode->prio) {

    case EMERGENCY:
      qtimersub (&ix->totalM, &oldnode->time, &result);
      ix->totalM.tv_sec = result.tv_sec;
      ix->totalM.tv_usec = result.tv_usec;
      ix->nodesM = ix->nodesM--;
      break;

    case NORMAL:
      qtimersub (&ix->totalN, &oldnode->time, &result);
      ix->totalN.tv_sec = result.tv_sec;
      ix->totalN.tv_usec = result.tv_usec;
      ix->nodesN = ix->nodesN--;
      break;

    default:
      LOG (L_ERR, "INFO:"M_NAME": Wrong type of queue specified: %d\n", oldnode->prio);
      break;
  }

  LOG (L_INFO, "INFO:"M_NAME": vq_decrease_index_value nodes: %d = nodesM: %d + nodesN: %d\n", ix->nodes, ix->nodesM, ix->nodesN);
      
  return;
} 

/** 
 * Initializes a new queue node: copies the nodeID and sets the time for the call
 * 
 * @param *ident pointer to the queueID_t 
 * @param prio call priority
 * @param type type of call 
 */
queueNode_t * vq_init_node (queueID_t *ident, int prio, int type)
{
  queueNode_t *n = NULL;
  queueID_t *tmp;
  n = shm_malloc (sizeof(queueNode_t));
  if (!n) {
    ERR ("vq_init_node: Could not allocate memory for new call\n");
    return NULL;
  }
  
  memset (n, 0, sizeof(queueNode_t));
  
  n->next = NULL;
  n->prev = NULL;
  tmp = &n->id;
  memcpy (tmp, ident, sizeof(queueID_t));
  n->prio = prio;
  n->type = type;

  // updates the arrival time in the node
  memcpy (&n->arrival, &ident->time, sizeof (struct timeval));

  return n;
}

//
// Procedures for queue call provisioning 
//

/**
 * Adds a prioritized node to the specified queue 
 *
 * @param *ix pointer to the queue index
 * @param **node double pointer to the node to be added
 *
 * TODO: make it generic for any kind of priority: 
 * provided that if P1 > P2 
 * P1 has a greater priority than P2
 * then T_P1 < T_P2
 *
 */
int vq_put_emergency_call (queueIndex_t *ix, queueNode_t **node)
{
  int do_res = 0;	// do re-scheduling

  queueNode_t *newnode = *(node);
  queueNode_t *aux = ix->first;
  queueNode_t *next = aux->next;
  queueNode_t *prev;
  queueID_t *tmp;

  // TODO: add nodes with multiple priorities
  int priority = newnode->prio;	// for comparing with aux->prio
  DBG ("newnode priority: %d\n", priority);

  DBG ("First node priority: %d\n", aux->prio);

  INFO ("vq_put_emergency_call: Adding Emergency node\n");

  //
  // First we try to set the 'aux' pointer to the right place 
  // where the Emergency node will be placed
  //
  
  // if first node is non-prioritized
  if (aux->prio == NORMAL) {
    INFO ("Incoming emergency node after -first- non-prioritized node in the queue\n");
    // if a normal node is being served let it finish
    // add the new emergency node at the end of the emergency calls
    if (next) {
      aux = aux->next;
    }
  } 

  DBG ("Current node priority: %d\n", aux->prio);

  if (aux->prio == EMERGENCY) {
    while (aux->prio == EMERGENCY && aux->next != NULL) {	
      DBG ("going to the last prioritized node: aux: %p aux->next: %p\n", aux, aux->next);
      aux = aux->next;
    }
  } 

  // now 'aux' is in the "correct place"
  prev = aux->prev;
  next = aux->next;
  DBG ("*** aux: %p, newnode: %p\n", aux, newnode);
  DBG ("*** aux->prev: %p ", prev);
  DBG ("aux->next: %p\n", next);
  DBG ("*** first: %p ", ix->first);
  DBG ("last: %p\n", ix->last);
  
  // adding a generic emergency node
  DBG ("add emergency node !\n");

  // add node: update pointers

  // check whether we are at the end of the queue
  if (next) {

    DBG ("adding in the middle of the queue\n");
    prev->next = newnode;
    newnode->next = aux;
    newnode->prev = prev;
    if (prev)
      aux->prev = newnode;

    // check re-scheduling
    if (next->prio == NORMAL) {
      DBG ("Marking nodes for re-scheduling\n");
      do_res = 1;
    }
    
  } else {

    DBG ("adding at the end of the queue\n");
    aux->next = (queueNode_t *)newnode;
    newnode->next = NULL;
    newnode->prev = (queueNode_t *)aux;
    newnode->next = NULL;
    ix->last = newnode;

  }

  printf ("pointers updated !\n");
  tmp = &newnode->id;
  printf ("Emergency node id: %.*s has been added to the queue\n", HASHLEN, tmp->id);

  // schedule call
  vq_schedule_call (ix, &newnode);

  vq_increase_index_values (&ix, newnode);

  vq_print_queue_status (ix);

  //
  // after adding an emergency node check for re-scheduling
  //

  if (do_res) {
    vq_reschedule_call (ix, newnode);
  }

  return 0;
}

int vq_put_normal_call (queueIndex_t *ix, queueNode_t **node)
{
  queueNode_t *newnode = *(node);
  queueNode_t *aux = ix->last;

  printf ("Adding Normal node...\n");
 
  // add at the end 
  aux->next = (queueNode_t *)newnode;
  newnode->next = NULL;
  newnode->prev = (queueNode_t *)aux;

  // update 'last' pointer
  ix->last = newnode;

  // schedule call
  vq_schedule_call (ix, &newnode);

  // update index
  vq_increase_index_values (&ix, newnode);
  
  vq_print_queue_status (ix);

  if (newnode->moved > 0) {
    printf ("$$$$$$$$$$$$$$$$$$$$ added re-scheduled normal node\n");
  } else {
    printf ("$$$$$$$$$$$$$$$$$$$$ finished adding normal call\n");
  }

  return 0;

}

/** 
 *  Adds a node to the list:
 * 
 *  First emergency nodes, then normal nodes.
 *  A new emergency node will be placed as the last of the emergency nodes.
 *  Normal nodes are put at the end of normal nodes. 
 *
 *  This procedure is also used to schedule the calls.
 *
 */
int vq_put_node_safe (queueIndex_t *ix, queueNode_t *newnode)
{
  //check what time it is now or leave it synchronized with VQ_update_queue ?
  //struct timeval now;
  struct timeval limit;

  // re-initialize the 'next' pointer to NULL
  newnode->next = NULL;

  gettimeofday (&ix->now, NULL);
  
  if ( ix->first == NULL) { 	// adding first node
    INFO ("Adding first node to the queue...\n");

    ix->first = newnode;
    ix->last = newnode;

    newnode->prev = NULL;
    newnode->next = NULL;
    
    // schedule call
    vq_schedule_call (ix, &newnode);

    // update index
    vq_increase_index_values (&ix, newnode);

  } else {

    switch (newnode->prio) {

      case EMERGENCY:
	      vq_put_emergency_call (ix, &newnode);
	      break;
      case NORMAL:
	      vq_put_normal_call (ix, &newnode);
	      break;
      default:
	      DBG ("Unknown priority type: %d\n", newnode->prio);
	      break;
    }

  }

  // if the scheduled time is greater than 'now' + 'max delay' we have to send a response
  DBG ("Call scheduled in %ld.%06ld\n", newnode->scheduled.tv_sec, newnode->scheduled.tv_usec);
  qtimerclear (limit);
  qtimeradd (&ix->now, &tv_timeout, &limit);
  
  DBG ("now: %ld.%06ld\tlimit: %ld.%06ld\tscheduled: %ld.%06ld\n",
       ix->now.tv_sec, ix->now.tv_usec,
       limit.tv_sec, limit.tv_usec,
       newnode->scheduled.tv_sec, newnode->scheduled.tv_usec);
       
  if (qtimecmp (&limit, &newnode->scheduled) > 0) {
      INFO ("Exceeded queue timeout !\n");    
      INFO ("Retry in %ld seconds\n", (newnode->scheduled.tv_sec - ix->now.tv_sec));
  
      // update time information for the reply
      timeout->time = newnode->scheduled.tv_sec - ix->now.tv_sec;
      
      // update field in newnode
      // TBD

      return CSCF_RETURN_FALSE;
      
  } else {
      INFO ("Node added to the queue\n");
  }  
  
  return CSCF_RETURN_TRUE;

}

/** updates values in the queue index after inserting a new node 
 *
 * NOTE: the calling function has already the lock in the queue
 */
void vq_increase_index_values (queueIndex_t **index, queueNode_t *newnode)
{
  struct timeval result;
  queueIndex_t *ix = *(index);
  
  //DBG ("increase_index_values... \n");

  qtimerclear (result);

  ix->nodes++;

  switch (newnode->prio) {

    case EMERGENCY:
      qtimeradd (&ix->totalM, &newnode->time, &result);
      ix->totalM.tv_sec = result.tv_sec;
      ix->totalM.tv_usec = result.tv_usec;
      ix->nodesM = ix->nodesM++;
      break;

    case NORMAL:
      qtimeradd (&ix->totalN, &newnode->time, &result);
      ix->totalN.tv_sec = result.tv_sec;
      ix->totalN.tv_usec = result.tv_usec;
      ix->nodesN = ix->nodesN++;
      break;

    default:
      WARN ("Wrong type of queue specified: %d\n", newnode->prio);
      break;
  }

  INFO ("increase_index_values: %d total nodes  = %d nodesM  + %d nodesN\n", ix->nodes, ix->nodesM, ix->nodesN);

  return;
} 

/** 
 * Deletes one node from the queue. The node itself must be freed outside this 
 * funtion.
 */
int vq_delete_node (queueIndex_t *ix, queueNode_t *node)
{
  queueNode_t *aux;
  queueNode_t *prev;
  queueNode_t *next;
  queueID_t *tmp;
  
  //NOTE: no lock inside this function since it is called from a 'locking' function.
 
  tmp = &node->id;
  DBG ("delete_node id: %.*s...\n", 2*HASHLEN, tmp->strid);

  if (ix->first == node)
  {
    DBG ("Node is the first\n");
    if (ix->nodes == 1)
    {                                               // deleting the only node in [index]
	DBG ("Only one node in the queue\n");
	ix->first = NULL;

    } else {
	DBG ("deleting the first node");
	ix->first = (queueNode_t *)node->next;       // deleting the first node in [index]
	aux = (queueNode_t *)node->next;
	aux->prev = NULL;
    }

  } else {        

      // deleting one node
      DBG ("looking for the node in the queue...\n");
      aux = ix->first;
      DBG ("aux: %p, aux->next: %p, aux->prev: %p || node:  %p\n", aux, aux->next, aux->prev, node);                
      
      do { //while (aux != NULL) {
	aux = aux->next;
	DBG ("advanced --> aux: %p, aux->next: %p, aux->prev: %p\n", aux, aux->next, aux->prev);
	DBG ("aux: %p == node: %p\n", aux, node);
	if (aux == node) { 
	  DBG ("node found :-)");
	  tmp = &aux->id;
 	  DBG (" node id: %*.s\n", HASHLEN, tmp->id);
	  break;
	} else {
	  DBG ("not yet found :-(\n");
	}
      } while (aux->next != NULL); 
      
      if (!aux) { 
	LOG (L_ERR, "ERROR: delete_node: Could not find node !\n");
	return 1;
      }

      prev = aux->prev;
      next = aux->next;
      prev->next = next;

      if (next) {
	next->prev = prev;
      } else {
	prev->next = NULL;
      }
  }

  return 0;
}

/** 
 * Returns the address of the node from the queue that matches the identifier 
 * provided. 
 *
 * @return pointer to queue node or NULL if not found
 * @param *ix pointer to the queue index
 * @param *id pointer to the binary form of the id of HASHLEN bytes 
 */
queueNode_t * vq_get_node_by_id_safe(queueIndex_t *ix, queueID_t *id)
{
  queueNode_t *aux;

  DBG ("searching node id: %.*s...\n", 2*HASHLEN, id->strid);

  aux = ix->first;
  if (!aux) {
    DBG ("vq_get_node_by_id_safe: empty queue\n");
    return NULL;
  }
  
  do {
      
      if (!memcmp (&aux->id.id, id->id, HASHLEN)) {
	DBG ("vq_get_node_by_id_safe: node id found\n");
	return aux;
      }
      aux = (queueNode_t *)aux->next;	
      
  } while (aux);
  
  WARN ("vq_get_node_by_id_safe: node id NOT found\n");
  
  return NULL;
}


