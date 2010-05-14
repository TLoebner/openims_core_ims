/*
 * $Id: vq_mod.h 579 2008-08-25 15:24:33Z vingarzan $
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


#ifndef VQ_MOD_H
#define VQ_MOD_H

#include "vq_time.h"
#include "vq_queueid.h"

#include <sys/time.h>
#include "../../dprint.h"
#include "../../sr_module.h"
#include "../../locking.h"

#define M_NAME "VIRTUAL_QUEUE"

// priority types
// M : Emergency ; N : Normal
#define EMERGENCY_CALL	1
#define M		1
#define NORMAL_CALL 	2
#define N		2

#define BUFSIZE 1024
#define MAXNAME 16

/** Return and break the execution of routing script */
#define CSCF_RETURN_BREAK       0 
/** Return true in the routing script */
#define CSCF_RETURN_TRUE        1
/** Return false in the routing script */
#define CSCF_RETURN_FALSE -1
/** Return error in the routing script */
#define CSCF_RETURN_ERROR -2

typedef struct queueNode queueNode_t;

/** Queue node description. */
struct queueNode {

  queueID_t id;			/// node ID
  int type;			/// type of call (SIP method)
  queueNode_t *next;		/// pointer to next node
  queueNode_t *prev;		/// pointer to previous node
  int prio;			/// type of priority

  int moved;			/// queue node has been recollocated
  int active;			/// queue node is being processed right now

  struct timeval time;		/// time info for the call
				/// depends on the type of call and the load 

  struct timeval arrival;	/// arrival time in the queue 
  struct timeval scheduled;	/// scheduled time 
  struct timeval out;		/// 'scheduled' time + 'time' duration
  
  int stat;			/// 'true' if the node has been counted for statistics
};


/** Header and index for each queue. Holds queue information. */
struct queueIndex {		

  char queueID[MAXNAME];	/// name of the queue
  gen_lock_t *lock;		/// queue lock
  
  queueNode_t *first;  		/// pointer to first node in the queue
  queueNode_t *last;		/// pointer to the last node in the queue

  struct timeval totalM;	/// total time for Emergency nodes in the queue
  struct timeval totalN;	/// total time for Normal nodes in the queue

  int nodes;			/// current total number of nodes in the queue 
  int nodesN;			/// current number of nodes with normal priority
  int nodesM;			/// current number of nodes with high priority
  int moved;			/// current number of normal nodes being reallocated

  struct timeval now;		/// current time of the queue

  int near_future;		/// 'near future' time value of the queue 
  int far_past;			/// older nodes than this time will be dismissed

  unsigned int processed;	/// total number of processed calls  
  unsigned int incoming;	/// total number of incoming calls
  unsigned int rescheduled;	/// total number of rescheduled calls
  unsigned int lost;		/// total number of lost calls

};

typedef struct queueIndex queueIndex_t;


/** Response to a call */
/*struct call_response {

  queueID_t id;			/// call queue id
  int retrans;			/// true if retransmission
  struct timeval retry;		/// time when the call will be (re-)scheduled
};

typedef struct call_response call_resp_t;
*/
//
// Some Macros
//

#define STR_SHM_DUP(dest,src,txt)\
{\
        if ((src).len==0) {\
                (dest).s=0;\
                (dest).len=0;\
        }else {\
                (dest).s = shm_malloc((src).len);\
                if (!(dest).s){\
                        LOG(L_ERR,"ERR:"M_NAME":"txt": Error allocating %d bytes\n",(src).len);\
                        (dest).len = 0;\
                        goto out_of_memory;\
                }else{\
                        (dest).len = (src).len;\
                        memcpy((dest).s,(src).s,(src).len);\
                }\
        }\
}

#define STR_PKG_DUP(dest,src,txt)\
{\
        if ((src).len==0) {\
                (dest).s=0;\
                (dest).len=0;\
        }else {\
                (dest).s = pkg_malloc((src).len);\
                if (!(dest).s){\
                        LOG(L_ERR,"ERRL:"M_NAME":"txt": Error allocating %d bytes\n",(src).len);\
                        (dest).len = 0;\
                        goto out_of_memory;\
                }else{\
                        (dest).len = (src).len;\
                        memcpy((dest).s,(src).s,(src).len);\
                }\
        }\
}

#define STR_APPEND(dst,src)\
        {memcpy((dst).s+(dst).len,(src).s,(src).len);\
        (dst).len = (dst).len + (src).len;}

/* ANSI Terminal colors */
#define ANSI_GRAY               "\033[01;30m"
#define ANSI_BLINK_RED  "\033[00;31m"
#define ANSI_RED                "\033[01;31m"
#define ANSI_GREEN              "\033[01;32m"
#define ANSI_YELLOW     "\033[01;33m"
#define ANSI_BLUE               "\033[01;34m"
#define ANSI_MAGENTA    "\033[01;35m"
#define ANSI_CYAN               "\033[01;36m"
#define ANSI_WHITE              "\033[01;37m"


#define FParam_INT(val) { \
	 .v = { \
		.i = val \
	 },\
	.type = FPARAM_INT, \
	.orig = "int_value", \
};

#define FParam_STRING(val) { \
	.v = { \
		.str = STR_STATIC_INIT(val) \
	},\
	.type = FPARAM_STR, \
	.orig = val, \
};


//
// Procedures definition
//

queueIndex_t *vq_init_index (char *name);
queueNode_t *vq_pop_node (queueIndex_t *index); 
void vq_decrease_index_values (queueIndex_t **index, queueNode_t *oldnode);
void vq_update_queue (queueIndex_t *index);
int vq_process_call (void *call, queueIndex_t *index);
queueNode_t *vq_init_node (queueID_t *id, int prio, int type);
int vq_reschedule_call (queueIndex_t *ix, queueNode_t *newnode);
void vq_increase_rescheduled_calls (queueIndex_t **index);
int vq_schedule_call (queueIndex_t *index, queueNode_t **callin);
int vq_put_node (queueIndex_t *ix, queueNode_t *newnode);
void vq_increase_index_values (queueIndex_t **index, queueNode_t *newnode);
int vq_delete_node (queueIndex_t *ix, queueNode_t *node);
queueNode_t *vq_get_node_by_id (queueIndex_t *ix, queueID_t *id);

//
// Debug functions
//
void vq_print_queue (queueIndex_t *index);
void vq_print_node_info (queueNode_t *node);
void vq_print_queue_status (queueIndex_t *ix);


#endif /* VQ_MOD_H */

