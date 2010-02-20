/** 
 * $Id$
 *  
 * Copyright (C) 2004-2006 FhG Fokus
 *
 * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
 * implementation
 *
 * Open IMS Core is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the Open IMS Core software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact Fraunhofer FOKUS by e-mail at the following
 * addresses:
 *     info@open-ims.org
 *
 * Open IMS Core is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * It has to be noted that this Open Source IMS Core System is not 
 * intended to become or act as a product in a commercial context! Its 
 * sole purpose is to provide an IMS core reference implementation for 
 * IMS technology testing and IMS application prototyping for research 
 * purposes, typically performed in IMS test-beds.
 * 
 * Users of the Open Source IMS Core System have to be aware that IMS
 * technology may be subject of patents and licence terms, as being 
 * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
 * standards. Thus all Open IMS Core users have to take notice of this 
 * fact and have to agree to check out carefully before installing, 
 * using and extending the Open Source IMS Core System, if related 
 * patents and licences may become applicable to the intended usage 
 * context.  
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
 
/**
 * \file
 * 
 * CDiameterPeer Worker Procedures
 * 
 * This the process pool representation that is used for processing incoming messages. 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#include <time.h> 
#include <stdlib.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/ipc.h>
#include <sys/sem.h>

#include "utils.h"
#include "globals.h"
#include "config.h"

#include "worker.h"
#include "diameter_api.h"

/* defined in ../diameter_peer.c */
int dp_add_pid(pid_t pid);
void dp_del_pid(pid_t pid);

extern dp_config *config;		/**< Configuration for this diameter peer 	*/

task_queue_t *tasks;			/**< queue of tasks */

cdp_cb_list_t *callbacks;		/**< list of callbacks for message processing */


	
	
/**
 * Gets the lock on a semaphore and blocks until it is available.
 * This procedures does not consume CPU cycles as a busy-waiting would and it is used for
 * blocking on the task queue without a big impact on performance.
 * @param sid - semaphore id
 * @returns when the sempahore is aquired or shutdown
 */
#define cdp_lock_get(sid) sem_get(sid)

/**
 * Releases the lock on a sempahore.
 * @param sid - the semaphore id
 */
#define cdp_lock_release(sid) sem_release(sid)

/**
 * Initializes the worker structures, like the task queue.
 */
void worker_init()
{
	tasks = shm_malloc(sizeof(task_queue_t));
	
	tasks->lock = lock_alloc();
	tasks->lock = lock_init(tasks->lock);
	
	sem_new(tasks->empty,0);
		
	sem_new(tasks->full,1);
		
	tasks->start = 0;
	tasks->end = 0;
	tasks->max = config->queue_length;
	tasks->queue = shm_malloc(tasks->max*sizeof(task_t));
	if (!tasks->queue) {
		LOG_NO_MEM("shm",tasks->max*sizeof(task_t));
		goto out_of_memory;
	}
	memset(tasks->queue,0,tasks->max*sizeof(task_t));
		
	callbacks = shm_malloc(sizeof(cdp_cb_list_t));
	if (!callbacks) goto out_of_memory;
	callbacks->head = 0; 
	callbacks->tail = 0;
	return;
out_of_memory:
	if (tasks){
		if (tasks->lock) {
			lock_destroy(tasks->lock);
			lock_dealloc(&(tasks->lock)); 
		}
		sem_free(tasks->full);
		sem_free(tasks->empty);
		if (tasks->queue) shm_free(tasks->queue);
		shm_free(tasks);
	}
	if (callbacks) shm_free(callbacks);
}

/**
 * Destroys the worker structures. 
 */
void worker_destroy()
{
	int i,sval=0;
	if (callbacks){
		while(callbacks->head)
			cb_remove(callbacks->head);
		shm_free(callbacks);
	}

	if (tasks) {
	//	LOG(L_CRIT,"-1-\n");
		lock_get(tasks->lock);
		for(i=0;i<tasks->max;i++){
			if (tasks->queue[i].msg) AAAFreeMessage(&(tasks->queue[i].msg));
			tasks->queue[i].msg = 0;
			tasks->queue[i].p = 0;
		}
		lock_release(tasks->lock);

		LOG(L_INFO,"Unlocking workers waiting on empty queue...\n");
		for(i=0;i<config->workers;i++)
			sem_release(tasks->empty);
		LOG(L_INFO,"Unlocking workers waiting on full queue...\n");
		i=0;
		while(sem_getvalue(tasks->full,&sval)==0)			
			if (sval<=0) {
				sem_release(tasks->full);
				i=1;
			}
			else break;
		sleep(i);
		
		lock_get(tasks->lock);
	//	LOG(L_CRIT,"-2-\n");	
		shm_free(tasks->queue);
		lock_destroy(tasks->lock);
		lock_dealloc((void*)tasks->lock);
	//	LOG(L_CRIT,"-3-\n");	
		
		//lock_release(tasks->empty);
		sem_free(tasks->full);
		sem_free(tasks->empty);
		
		shm_free(tasks);
	}
}

/*unsafe*/
int cb_add(cdp_cb_f cb,void *ptr)
{
	cdp_cb_t *x;
	x = shm_malloc(sizeof(cdp_cb_t));
	if (!x){
		LOG_NO_MEM("shm",sizeof(cdp_cb_t));
		return 0;
	}
	x->cb = cb;
	x->ptr = shm_malloc(sizeof(void*));
	if (!x->ptr){
		LOG_NO_MEM("shm",sizeof(void*));
		return 0;
	}
	*(x->ptr) = ptr;
	x->next = 0;
	x->prev = callbacks->tail;
	if (callbacks->tail) callbacks->tail->next = x;
	callbacks->tail = x;
	if (!callbacks->head) callbacks->head = x;
	return 1;	
}

/*unsafe*/
void cb_remove(cdp_cb_t *cb)
{
	cdp_cb_t *x;
	x = callbacks->head;
	while(x && x!=cb) x = x->next;
	if (!x) return;
	if (x->prev) x->prev->next = x->next;
	else callbacks->head = x->next;
	if (x->next) x->next->prev = x->prev;
	else callbacks->tail = x->prev;
	
	if (x->ptr) shm_free(x->ptr);
	shm_free(x);
}

/**
 * Adds a message as a task to the task queue.
 * This blocks if the task queue is full, untill there is space.
 * @param p - the peer that the message was received from
 * @param msg - the message
 * @returns 1 on success, 0 on failure (eg. shutdown in progress)
 */ 
int put_task(peer *p,AAAMessage *msg)
{
//	LOG(L_CRIT,"+1+\n");
	lock_get(tasks->lock);
//	LOG(L_CRIT,"+2+\n");
	while ((tasks->end+1)%tasks->max == tasks->start){
//		LOG(L_CRIT,"+3+\n");
		lock_release(tasks->lock);
//		LOG(L_CRIT,"+4+\n");
		if (*shutdownx) {
			cdp_lock_release(tasks->full);
			return 0;
		}
//		LOG(L_ERR,"+");
		cdp_lock_get(tasks->full);
//		LOG(L_CRIT,"+5+\n");
		if (*shutdownx) {
			cdp_lock_release(tasks->full);
			return 0;
		}
		lock_get(tasks->lock);
	}
	tasks->queue[tasks->end].p = p;
	tasks->queue[tasks->end].msg = msg;
	tasks->end = (tasks->end+1) % tasks->max;
	cdp_lock_release(tasks->empty);
	lock_release(tasks->lock);
	return 1;
}

/**
 * Remove and return the first task from the queue (FIFO).
 * This blocks until there is something in the queue.
 * @returns the first task from the queue or an empty task on error (eg. shutdown in progress)
 */
task_t take_task()
{
	task_t t={0,0};
//	LOG(L_CRIT,"-1-\n");
	lock_get(tasks->lock);
//	LOG(L_CRIT,"-2-\n");
	while(tasks->start == tasks->end){
//		LOG(L_CRIT,"-3-\n");
		lock_release(tasks->lock);
//		LOG(L_CRIT,"-4-\n");
		if (*shutdownx) {
			cdp_lock_release(tasks->empty);
			return t;
		}
//		LOG(L_ERR,"-");
		cdp_lock_get(tasks->empty);
//		LOG(L_CRIT,"-5-\n");
		if (*shutdownx) {
			cdp_lock_release(tasks->empty);
			return t;
		}
		
		lock_get(tasks->lock);
//		LOG(L_CRIT,"-6-\n");
	}
//	LOG(L_CRIT,"-7-\n");
	
	t = tasks->queue[tasks->start];
	tasks->queue[tasks->start].msg = 0;
	tasks->start = (tasks->start+1) % tasks->max;
	cdp_lock_release(tasks->full);
	lock_release(tasks->lock);
	return t;
}

/**
 * Poisons the worker queue.
 * Actually it just releases the task queue locks so that the workers get to evaluate
 * if a shutdown is in process and exit.
 */
void worker_poison_queue()
{
//	int i;
//	for(i=0;i<config->workers;i++)
	cdp_lock_release(tasks->empty);
}

/**
 * This is the main worker process.
 * Takes tasks from the queue in a loop and processes them by calling the registered callbacks.
 * @param id - id of the worker
 * @returns never, exits on shutdown.
 */
void worker_process(int id)
{
	task_t t;
	cdp_cb_t *cb;
	int r;
	LOG(L_INFO,"INFO:[%d] Worker process started...\n",id);	
	/* init the application level for this child */
	while(1){
		if (shutdownx&&(*shutdownx)) break;
		t = take_task();
		if (!t.msg) {
			if (shutdownx&&(*shutdownx)) break;
			LOG(L_INFO,"INFO:worker_process(): [%d] got empty task Q(%d/%d)\n",id,tasks->start,tasks->end);
			continue;
		}		
		LOG(L_DBG,"DBG:worker_process(): [%d] got task Q(%d/%d)\n",id,tasks->start,tasks->end);
		r = is_req(t.msg);
		for(cb = callbacks->head;cb;cb = cb->next)
			(*(cb->cb))(t.p,t.msg,*(cb->ptr));
		
		if (r){
			AAAFreeMessage(&(t.msg));
		}else{
			/* will be freed by the user in upper api */
			/*AAAFreeMessage(&(t.msg));*/
		}
	}
	worker_poison_queue();
	LOG(L_INFO,"INFO:[%d]... Worker process finished\n",id);	
#ifdef CDP_FOR_SER
#else
	#ifdef PKG_MALLOC
		LOG(memlog, "Worker[%d] Memory status (pkg):\n",id);
		//pkg_status();
		#ifdef pkg_sums
			pkg_sums();
		#endif 
	#endif
	dp_del_pid(getpid());	
#endif
	exit(0);
}

