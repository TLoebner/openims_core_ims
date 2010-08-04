
/*
 * $Id$
 *
 * Virtual Queue exported functions
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

#include "mod.h"
#include "sip.h"
#include "queue.h"
#include "vq_stats.h"
#include "vq_logging.h"

extern queueIndex_t *queue;
extern struct vq_to *timeout;
extern time_t vq_timeout;

/* Global variables and imported functions */
int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 


int VQ_process_new_normal_request (struct sip_msg *rpl, char *str1, char *str2)
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

int VQ_process_new_emergency_request (struct sip_msg *rpl, char *str1, char *str2)
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

static fparam_t fp_503 = FParam_INT(503);
static fparam_t fp_se_serv_unav = FParam_STRING("Service Unavailable");

int VQ_send_503_retry_after_reply (struct sip_msg* msg, char* str1, char* str2)
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

int VQ_send_182_queued_reply (struct sip_msg* msg, char* str1, char* str2)
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

int VQ_is_empty (struct sip_msg *rpl, char *str1, char *str2)
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

int VQ_update_stats (struct sip_msg *rpl, char *str1, char *str2)
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

/** Procedure to update the queue. 
Drops calls that have been too much time in the queue.
Updates the log file. */
int VQ_update_queue (struct sip_msg *rpl, char *str1, char *str2)
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

