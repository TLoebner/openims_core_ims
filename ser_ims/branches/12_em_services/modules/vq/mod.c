/*
 * $Id$
 *
 * Virtual Queue module
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


#include "queue.h"
#include "vq_logging.h"
#include "vq_stats.h"
#include "vq_queueid.h"
#include <stdio.h>
#include <stdlib.h>
#include "sip.h"
#include "../../mem/shm_mem.h"
#include "../tm/tm_load.h"

struct tm_binds tmb;            		/**< Structure with pointers to tm funcs 		*/


MODULE_VERSION

//
// exported variables
//
str vq_path_to_log_file = STR_STATIC_INIT("/tmp/"); /** path to logfile for queue operations */
int vq_max_delay = 120;		/**< maximum length of the queue in seconds */
time_t vq_timeout = 20;		/**< maximum time of a call in the queue in seconds before sending 503 or 182 */
str vq_queue_name = {NULL, 0};	/**< queue name */
str vq_queue_name_str;		/**< queue name - str version */
struct timeval tv_timeout;	/**< stores the queue timeout from 'vq_timeout'*/
struct vq_to *timeout;
queueIndex_t *queue;

/* Global variables and imported functions */
int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 

//
// procedure declaration
//
static int mod_init( void );
//static int vq_child_init( int rank );
static void mod_destroy( void );

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
	mod_init,  /* module initialization function */
	0,        /* response function*/
	mod_destroy,        /* destroy function */
	0,        /* oncancel function */
	0         /* per-child init function */
};

static int mod_init (void) 
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
static void mod_destroy (void)
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





