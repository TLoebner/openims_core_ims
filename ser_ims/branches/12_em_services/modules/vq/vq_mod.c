/*
 * $Id: vq_mod.c 579 2008-08-25 15:24:33Z vingarzan $
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


#include "vq_mod.h"
#include "vq_logging.h"
#include "vq_stats.h"
#include "vq_queueid.h"
#include <stdio.h>
#include <stdlib.h>
#include "sip.h"
#include "../../mem/shm_mem.h"
/*
#include "../../parser/parse_uri.h"
#include "../../db/db.h"
#include "../../sr_module.h"
#include "../../socket_info.h"
#include "../../timer.h"
#include "../../locking.h"
#include "../tm/tm_load.h"
#include "../dialog/dlg_mod.h"
*/
#include "../tm/tm_load.h"

struct tm_binds tmb;            		/**< Structure with pointers to tm funcs 		*/

/**< Structure to hold the timeout for the 503 and 182 responses */
struct vq_to {
  //struct timeval time;
  time_t time;		/**< seconds to retry after */
  gen_lock_t *lock;	/**< generic lock */
};

MODULE_VERSION

//
// exported variables
//
str vq_path_to_log_file = STR_STATIC_INIT("/tmp/"); /** path to logfile for queue operations */
int vq_max_delay = 120;		/**< maximum length of the queue in seconds */
time_t vq_timeout = 20;		/**< maximum time of a call in the queue in seconds before sending 503 or 182 */
str vq_queue_name = {NULL, 0};	/**< queue name */
str vq_queue_name_str;		/**< queue name - str version */
static struct timeval tv_timeout;	/**< stores the queue timeout from 'vq_timeout'*/
struct vq_to *timeout;

/* Global variables and imported functions */
int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 

//
// procedure declaration
//
static int vq_init( void );
//static int vq_child_init( int rank );
static void vq_destroy( void );

//
// Module functions
//
int VQ_process_new_normal_request (struct sip_msg *rpl, char *str1, char *str2);
int VQ_process_new_emergency_request (struct sip_msg *rpl, char *str1, char *str2);
int VQ_put_node (struct sip_msg *rpl, char *str1, char *str2);
int VQ_take_node (struct sip_msg *rpl, char *str1, char *str2);
int VQ_is_empty (struct sip_msg *rpl, char *str1, char *str2);
int VQ_update_queue (struct sip_msg *rpl, char *str1, char *str2);
int VQ_update_stats (struct sip_msg *rpl, char *str1, char *str2);
int VQ_send_503_retry_after_reply (struct sip_msg *rpl, char *str1, char *str2);
int VQ_send_182_queued_reply (struct sip_msg* msg, char* str1, char* str2);

static queueIndex_t *queue;
static int plus;
static int minus;

/*
 * Exported functions
 */
static cmd_export_t cmds[] = {
	{"VQ_process_new_normal_request", VQ_process_new_normal_request, 0, 0, REQUEST_ROUTE},
	{"VQ_process_new_emergency_request", VQ_process_new_emergency_request, 0, 0, REQUEST_ROUTE},
	{"VQ_is_empty", VQ_is_empty, 0, 0,		REQUEST_ROUTE},
	{"VQ_update_queue", VQ_update_queue, 0, 0,	REQUEST_ROUTE},
	{"VQ_update_stats", VQ_update_stats, 0, 0,	REQUEST_ROUTE},
	{"VQ_send_503_retry_after_reply", VQ_send_503_retry_after_reply, 0, 0,	REQUEST_ROUTE},
	{"VQ_send_182_queued_reply", VQ_send_182_queued_reply, 0, 0,	REQUEST_ROUTE},
	{0, 0, 0, 0, 0}
};

/*
 * Exported parameters
 */
static param_export_t params[] = {
        {"vq_path_to_log_file",  PARAM_STR, &vq_path_to_log_file},
	{"vq_max_delay", PARAM_INT, &vq_max_delay},
	{"vq_queue_name",  PARAM_STR, &vq_queue_name},
	{"vq_timeout", PARAM_INT, &vq_timeout},
	{0, 0, 0}
};

/*
 * Module parameter variables
 */
struct module_exports exports = {
	"vq",
	cmds,     /* Exported functions */
	0,        /* RPC method */
	params,   /* Exported parameters */
	vq_init,  /* module initialization function */
	0,        /* response function*/
	vq_destroy,        /* destroy function */
	0,        /* oncancel function */
	0         /* per-child init function */
};

static int 
vq_init (void) 
{
  queue = NULL;
  load_tm_f load_tm;

  LOG (L_INFO, "INFO:"M_NAME": - init\n" );
  
  /* bind to the tm module */
  if (!(load_tm = (load_tm_f)find_export("load_tm",NO_SCRIPT,0))) {
	  LOG(L_ERR, "ERR:"M_NAME":mod_init: Can not import load_tm. This module requires tm module\n");
	  goto error;
  }
  if (load_tm(&tmb) == -1)
	  goto error;
  
  /* load the send_reply function from sl module */
  sl_reply = find_export("sl_send_reply", 2, 0);
  if (!sl_reply) {
	LOG(L_ERR, "ERR"M_NAME":mod_init: This module requires sl module\n");
	goto error;
  }

  vq_queue_name_str.s = vq_queue_name.s;
  vq_queue_name_str.len = strlen(vq_queue_name.s);

  //TODO: use the queue name from the cfg file
  queue = vq_init_index (vq_queue_name_str.s);

  if (!queue) 
    goto error;
 
  
  // debug variables
  plus = minus = 0;
  
  //LOG (L_INFO, "INFO:"M_NAME": - init queue at %p\n", queue);
  DBG (""M_NAME": - init queue at %p\n", queue);
  DBG (""M_NAME": - queue name: %.*s\n", vq_queue_name_str.len, vq_queue_name_str.s);
  DBG (""M_NAME": - max queue delay %d\n", vq_max_delay);
  DBG (""M_NAME": - max length of the queue %ld seconds\n", vq_timeout);
  
  // initialize tv_timeout 
  tv_timeout.tv_sec = vq_timeout;
  tv_timeout.tv_usec = 0;
  
  // init sub-modules
  vq_open_logfile (); 
  vq_init_vq_stats ();

  // init the timeout local struct
  timeout = shm_malloc (sizeof (struct vq_to));
  if (!timeout) {
      ERR ("E_OUT_OF_MEM while allocating queue timeout");
      shm_free (queue);
      goto error;
  }
  timeout->lock = (gen_lock_t *)lock_alloc();
  if (timeout->lock == NULL){
      ERR ("E_OUT_OF_MEM while allocating queue index lock");
      shm_free (queue);
      goto error;
  }  
  if (lock_init (timeout->lock) == 0 ){
      lock_dealloc (timeout->lock);
      shm_free (queue);
      goto error;
  }
  
  return 0;

error:
  return -1; 

}

/**
 * Procedure to destroy the virtual queue and free the memory 
 */
static void 
vq_destroy (void)
{
  queueNode_t *node;
  int nn = 0; 			// calls in queue
  LOG (L_INFO, "INFO:"M_NAME": - deinit\n" );
  
  DBG ("acquiŕing the queue lock...\n");
  lock_get (queue->lock);
  DBG ("got the lock !\n");
  
  // delete from the queue and free allocated memory
  if (queue->first) {
    LOG (L_INFO, "INFO:"M_NAME": deleting calls from the queue\n" );
    while (queue->first) {
      node = vq_pop_node (queue);
      shm_free (node);
      nn++;
    }
    LOG (L_INFO, "INFO:"M_NAME": total %d calls deleted from the queue\n", nn);
  } else {
    LOG (L_INFO, "INFO:"M_NAME": virtual queue empty !\n" );
  }
  
  // free lock
  lock_dealloc (queue->lock);
  
  // free index
  shm_free (queue);
  LOG (L_INFO, "INFO:"M_NAME": freed queue index\n");
  
  vq_close_vq_stats();
  vq_close_logfile();
  
  return; 
}

int 
VQ_put_node (struct sip_msg *rpl, char *str1, char *str2)
{
  LOG( L_INFO, "INFO:"M_NAME": - put node %d !!!\n" , plus);
  plus++;
  return CSCF_RETURN_TRUE;
}

int 
VQ_take_node (struct sip_msg *rpl, char *str1, char *str2)
{
  LOG( L_INFO, "INFO:"M_NAME": - take node %d !!!\n" , minus );
  minus++;
  return CSCF_RETURN_TRUE;
}

int 
VQ_is_empty (struct sip_msg *rpl, char *str1, char *str2)
{
  if (!queue) {
    LOG (L_ERR, "ERROR:"M_NAME": queue is NULL\n");
    return CSCF_RETURN_ERROR;
  }
  
  if (queue->first) {
    LOG (L_INFO, "INFO:"M_NAME": queue empty\n");
    return CSCF_RETURN_TRUE;
  }
  
  return CSCF_RETURN_FALSE;
}

int 
VQ_update_stats (struct sip_msg *rpl, char *str1, char *str2)
{
  queueIndex_t *index = queue;
  queueNode_t *node;
  queueID_t *id;
  
  LOG( L_INFO, "INFO:"M_NAME": - update stats !!!\n");
  
  //lock_get (index->lock);
  
  // get ID from call
  id = vq_get_call_id (rpl);
    
  // lookup in the queue
  node = vq_get_node_by_id (index, id);
  
  if (!node) {
   WARN ("VQ_update_stats: node id %.*s is not in the queue\n", HASHLEN*2, id->strid);
   return CSCF_RETURN_TRUE;
  }
  
  vq_print_node_info (node);
  
  vq_update_stat (index, node);
  
  //lock_release (index->lock);
  
  return CSCF_RETURN_TRUE;
}

int 
vq_check_retransmission (queueIndex_t *index, queueID_t *id)
{
  queueNode_t *inqueue;	// check retransmission if the same node is already in the queue
  
  // lookup the id in the queue
  inqueue = vq_get_node_by_id (index, id);
  
  if (inqueue) {
    INFO ("Detected a packet retransmission for \"%.*s\"\n", 2*HASHLEN, id->strid);
    // check whether the scheduled time corresponds to 'now'
    // if yes, route to the normal route -> return TRUE
    // if not, send another 503
  }

  return 1;
}

int
VQ_process_new_normal_request (struct sip_msg *rpl, char *str1, char *str2)
{
  int ret;
  queueIndex_t *index = queue;
  queueID_t *id;   
  queueNode_t *node;

  // create a queue node from the call information 
  
  // calculate the ID
  id = vq_get_call_id (rpl);
  
  // get the type of method from the sip_msg
  node = vq_init_node (id, NORMAL_CALL, rpl->REQ_METHOD);
  lock_get (index->lock);
  index->incoming++;
  lock_release (index->lock);

  // check retransmissions
  ret = vq_check_retransmission (index, id);
  
  // use the queue index to find out the queue length
  // with the scheduled time and the queue length
  // estimate the waiting time for the response
 
  return (vq_put_node (index, node));
    
}

int 
VQ_process_new_emergency_request (struct sip_msg *rpl, char *str1, char *str2)
{
  int ret;
  queueIndex_t *index = queue;
  queueID_t *id;
  queueNode_t *node;

  DBG ("Entering VQ_process_new_emergency_call...\n");
  
  // create a queue node from the call information 
  
  // calculate the ID
  id = vq_get_call_id (rpl);
  
  node = vq_init_node (id, EMERGENCY_CALL, rpl->REQ_METHOD);
  index->incoming++;

  // check retransmissions
  ret = vq_check_retransmission (index, id);
 
  // use the queue index to find out the queue length
  // with the scheduled time and the queue length
  // estimate the waiting time for the response

  return (vq_put_node (index, node));
  
}

/** initializes a queue index */
queueIndex_t *
vq_init_index (char *name)
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
queueNode_t *
vq_pop_node (queueIndex_t *ix)
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

/** Procedure to update the queue. 
Drops calls that have been too much time in the queue.
Updates the log file. */
int
VQ_update_queue (struct sip_msg *rpl, char *str1, char *str2)
{
  queueNode_t *current;
  queueNode_t *aux;
  queueIndex_t *index = queue;
//  queueID_t *id = NULL;
  
  struct timeval *now;
  time_t diff;
  
  LOG (L_INFO, "INFO:"M_NAME": **** update_queue ****\n");
  
  lock_get (index->lock);

  // check times in the queue elements until scheduled > now
  current = index->first;
  if (!current) {
    LOG (L_INFO, "INFO:"M_NAME": empty queue !\n");
    lock_release (index->lock);
    return CSCF_RETURN_TRUE;
  }

  // print info
  //vq_print_queue_status (index);
      
  // logging info
  vq_add_log_message (index);

  // update time index
  gettimeofday (&index->now, NULL);
  now = &index->now;

  INFO ("INFO:"M_NAME": queue not empty !\n");

  // while ( t_scheduled < t_now )
  while ( current && qtimecmp (&current->scheduled, now) >= 0 ) {
  
    DBG ("CDB:"M_NAME": now  %ld.%ld\n", now->tv_sec, now->tv_usec);
    DBG ("DBG:"M_NAME": current node info:\n");
  
    vq_print_node_info (current);

    // check whether node is too old
    diff = now->tv_sec - current->scheduled.tv_sec;
    DBG ("Time in the queue 'time_t diff': %ld\n", diff);
    DBG ("Current maximum timeout: %ld\n", vq_timeout);
    
    // advance to the next node
    aux = current;
    current = current->next;
    
    if (diff > vq_timeout) {
      
      DBG ("VQ_update_queue: Removing lost node\n");
     
      if (index->first == aux) {
	aux = vq_pop_node (index);
	DBG ("VQ_update_queue: After pop_node. 'aux' node at %p\n", aux);
	
      } else {
	vq_delete_node (index, aux);
	DBG ("VQ_update_queue: After deleting node. 'aux' node at %p\n", aux);
      }
      
      vq_decrease_index_values (&index, aux);
      index->lost++;
      INFO ("Lost calls count up to: %u\n", index->lost);
      shm_free (aux);

    //
    // check whether the node has finished processing
    //
      
    } else if (aux->stat == 1) {  

      DBG ("DBG:"M_NAME": call has been finished\n");
      
      if (index->first == aux) {
	aux = vq_pop_node (index);
	DBG ("VQ_update_queue: After pop_node. 'aux' node at %p\n", aux);
	
      } else {
	vq_delete_node (index, aux);
	DBG ("VQ_update_queue: After deleting node. 'aux' node at %p\n", aux);
      }
      
      vq_decrease_index_values (&index, aux);
      index->processed++;
      INFO ("Processed calls up to %u\n", index->processed);
      shm_free (aux);
    
    //
    // node is not too old and still not finished processing
    //
    
    } else {
      DBG ("Node stat: %d, diff: %ld\n", aux->stat, diff);
    }
    
  }    

  if (!current) {
    INFO ("Reached end of the queue\n"); 
  }
  
  vq_print_queue_status (index);

  lock_release (index->lock);

  LOG (L_INFO, "INFO:"M_NAME": **** finished update_queue ****\n");

  return CSCF_RETURN_TRUE;
}

/** updates values in the queue index after deleting a queue node 
 *
 * NOTE: the calling function has already locked the queue 
 */
void
vq_decrease_index_values (queueIndex_t **index, queueNode_t *oldnode)
{
  struct timeval result;
  queueIndex_t *ix = *(index);
  
  //LOG (L_INFO, "INFO:"M_NAME": vq_decrease_index_values... ");

  qtimerclear (result);

  ix->nodes--;

  switch (oldnode->prio) {

    case M:
      qtimersub (&ix->totalM, &oldnode->time, &result);
      ix->totalM.tv_sec = result.tv_sec;
      ix->totalM.tv_usec = result.tv_usec;
      ix->nodesM = ix->nodesM--;
      break;

    case N:
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
queueNode_t *
vq_init_node (queueID_t *ident, int prio, int type)
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

  // set time for the call
  vq_set_call_time (&n);

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
int
vq_put_emergency_call (queueIndex_t *ix, queueNode_t **node)
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
  if (aux->prio == N) {
    INFO ("Incoming emergency node after -first- non-prioritized node in the queue\n");
    // if a normal node is being served let it finish
    // add the new emergency node at the end of the emergency calls
    if (next) {
      aux = aux->next;
    }
  } 

  DBG ("Current node priority: %d\n", aux->prio);

  if (aux->prio == M) {
    while (aux->prio == M && aux->next != NULL) {	
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
  DBG ("add M node !\n");

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
    if (next->prio == N) {
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

int 
vq_put_normal_call (queueIndex_t *ix, queueNode_t **node)
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
int
vq_put_node (queueIndex_t *ix, queueNode_t *newnode)
{
  //check what time it is now or leave it synchronized with VQ_update_queue ?
  //struct timeval now;
  struct timeval limit;

  // re-initialize the 'next' pointer to NULL
  newnode->next = NULL;

  // get lock
  lock_get (ix->lock);
  
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

      case M:
	      vq_put_emergency_call (ix, &newnode);
	      break;
      case N:
	      vq_put_normal_call (ix, &newnode);
	      break;
      default:
	      DBG ("Unknown priority type: %d\n", newnode->prio);
	      break;
    }

  }

  // lock release
  lock_release (ix->lock);

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
void
vq_increase_index_values (queueIndex_t **index, queueNode_t *newnode)
{
  struct timeval result;
  queueIndex_t *ix = *(index);
  
  //DBG ("increase_index_values... \n");

  qtimerclear (result);

  ix->nodes++;

  switch (newnode->prio) {

    case M:
      qtimeradd (&ix->totalM, &newnode->time, &result);
      ix->totalM.tv_sec = result.tv_sec;
      ix->totalM.tv_usec = result.tv_usec;
      ix->nodesM = ix->nodesM++;
      break;

    case N:
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

/** 
 * Deletes one node from the queue. The node itself must be freed outside this 
 * funtion.
 */
int 
vq_delete_node (queueIndex_t *ix, queueNode_t *node)
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
queueNode_t * 
vq_get_node_by_id (queueIndex_t *ix, queueID_t *id)
{
  queueNode_t *aux;

  DBG ("searching node id: %.*s...\n", 2*HASHLEN, id->strid);

  lock_get (ix->lock);
  
  aux = ix->first;
  if (!aux) {
    DBG ("vq_get_node_by_id: empty queue\n");
    lock_release (ix->lock);
    return NULL;
  }
  
  do {
      
      if (!memcmp (&aux->id.id, id->id, HASHLEN)) {
	DBG ("vq_get_node_by_id: node id found\n");
	lock_release (ix->lock);
	return aux;
      }
      aux = (queueNode_t *)aux->next;	
      
  } while (aux);
  
  WARN ("vq_get_node_by_id: node id NOT found\n");
  lock_release (ix->lock);
  
  return NULL;
}

static fparam_t fp_503 = FParam_INT(503);
static fparam_t fp_se_serv_unav = FParam_STRING("Service Unavailable");

int
VQ_send_503_retry_after_reply (struct sip_msg* msg, char* str1, char* str2)
{
	str hdr = {pkg_malloc(32), 0};
	
	if (!hdr.s) {
		LOG(L_ERR, "ERR:"M_NAME":vq_send_503_retry_after_reply(): no memory for hdr\n");
		goto error;
	}

	hdr.len = snprintf(hdr.s, 31, "Retry after %ld seconds\r\n", timeout->time);

	//DBG ("release lock on timeout\n");
	//lock_release (timeout->lock);
	
	if (!cscf_add_header_rpl(msg, &hdr)) {
		LOG(L_ERR, "ERR:"M_NAME":VQ_send_503_retry_after_reply(): Can't add header\n");
		goto error;
 	}
	
	return sl_reply(msg, (char *)&fp_503, (char *)&fp_se_serv_unav);

error:
	if (hdr.s) pkg_free(hdr.s);
	return CSCF_RETURN_FALSE;
}

static fparam_t fp_182 = FParam_INT(182);
static fparam_t fp_queued = FParam_STRING("Queued");

int
VQ_send_182_queued_reply (struct sip_msg* msg, char* str1, char* str2)
{
	str hdr = {pkg_malloc(32), 0};
	
	if (!hdr.s) {
		LOG(L_ERR, "ERR:"M_NAME":VQ_send_182_queued_reply(): no memory for hdr\n");
		goto error;
	}

	hdr.len = snprintf(hdr.s, 31, "Queued please wait\r\n");

	if (!cscf_add_header_rpl(msg, &hdr)) {
		LOG(L_ERR, "ERR:"M_NAME":VQ_send_182_queued_reply(): Can't add header\n");
		goto error;
 	}
	
	return sl_reply(msg, (char *)&fp_182, (char *)&fp_queued);

error:
	if (hdr.s) pkg_free(hdr.s);
	return CSCF_RETURN_FALSE;
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
