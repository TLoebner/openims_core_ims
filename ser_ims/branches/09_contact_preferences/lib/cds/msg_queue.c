/* 
 * Copyright (C) 2005 iptelorg GmbH
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

#include <stdio.h>
#include <cds/msg_queue.h>
#include <cds/memory.h>
#include <cds/ref_cntr.h>
#include <cds/logger.h>

mq_message_t *create_message_ex(int data_len)
{
	mq_message_t *m;
	if (data_len < 0) data_len = 0;
	m = cds_malloc(data_len + sizeof(mq_message_t));
	if (!m) return NULL;
	m->data_len = data_len;
	m->data = (((char *)m) + sizeof(mq_message_t));
	m->next = NULL;
	m->allocation_style = message_allocated_with_data;
	m->destroy_function = NULL;
	return m;
}

mq_message_t *create_message(void *data, int data_len)
{
	mq_message_t *m;
	/* if (data_len < 0) data_len = 0; */
	m = cds_malloc(sizeof(mq_message_t));
	if (!m) return NULL;
	m->data_len = data_len;
	m->data = data;
	m->next = NULL;
	m->allocation_style = message_holding_data_ptr;
	m->destroy_function = cds_free_ptr;
	return m;
}

void init_message_ex(mq_message_t *m, void *data, int data_len, destroy_function_f func)
{
	/* if (data_len < 0) data_len = 0; */
	if (!m) return;
	
	m->data_len = data_len;
	m->data = data;
	m->next = NULL;
	m->destroy_function = func;
	m->allocation_style = message_holding_data_ptr;
}

void set_data_destroy_function(mq_message_t *msg, destroy_function_f func)
{
	if (msg) msg->destroy_function = func;
}

void free_message(mq_message_t *msg)
{
	if (msg->destroy_function && msg->data) 
		msg->destroy_function(msg->data);
	switch (msg->allocation_style) {
		case message_allocated_with_data: 
				break;
		case message_holding_data_ptr: 
				/* if (msg->data) cds_free(msg->data); */
				break;
	}
	cds_free(msg);
}

int push_message(msg_queue_t *q, mq_message_t *m)
{
	if ((!q) || (!m)) return -1;
	m->next = NULL;
	
	if (q->use_mutex) cds_mutex_lock(&q->q_mutex);
	if (q->last) q->last->next = m;
	else {
		q->first = m;
		q->last = m;
	}
	q->last = m;
	if (q->use_mutex) cds_mutex_unlock(&q->q_mutex);
	
	return 0;
}

int mq_add_to_top(msg_queue_t *q, mq_message_t *m)
{
	if ((!q) || (!m)) return -1;
	m->next = NULL;
	
	if (q->use_mutex) cds_mutex_lock(&q->q_mutex);
	m->next = q->first;
	q->first = m;
	if (!q->last) q->last = m;
	if (q->use_mutex) cds_mutex_unlock(&q->q_mutex);
	
	return 0;
}

mq_message_t *pop_message(msg_queue_t *q)
{
	mq_message_t *m;
	if (!q) return NULL;

	if (q->use_mutex) cds_mutex_lock(&q->q_mutex);
	m = q->first;
	if (m) {
		if (q->first == q->last) {
			q->first = NULL;
			q->last = NULL;
		}
		else q->first = m->next;
		m->next = NULL;
	}
	if (q->use_mutex) cds_mutex_unlock(&q->q_mutex);
		
	return m;
}

/** 1 ... empty, 0 ...  NOT empty !! */
int is_msg_queue_empty(msg_queue_t *q)
{
	int res = 1;
	if (q->use_mutex) cds_mutex_lock(&q->q_mutex);
	if (q->first) res = 0;
	if (q->use_mutex) cds_mutex_unlock(&q->q_mutex);
	return res;
}

int msg_queue_init(msg_queue_t *q)
{
	return msg_queue_init_ex(q, 1);
}

int msg_queue_init_ex(msg_queue_t *q, int synchronize)
{
	if (synchronize) cds_mutex_init(&q->q_mutex);
	init_reference_counter(&q->ref);
	q->use_mutex = synchronize;
	q->first = NULL;
	q->last = NULL;
	return 0;
}

void msg_queue_destroy(msg_queue_t *q)
{
	mq_message_t *m,*n;
	if (!q) return;
	
	if (q->use_mutex) cds_mutex_lock(&q->q_mutex);
	m = q->first;
	while (m) {
		n = m->next;
		free_message(m);
		m = n;
	}
	q->first = NULL;
	q->last = NULL;
	if (q->use_mutex) {
		cds_mutex_unlock(&q->q_mutex);
		cds_mutex_destroy(&q->q_mutex);
	}
}

void msg_queue_free(msg_queue_t *q)
{
	if (remove_reference(&q->ref)) {
		msg_queue_destroy(q);
		cds_free(q);
	}
}

