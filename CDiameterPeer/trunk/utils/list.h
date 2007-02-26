#ifndef _MY_LIST_H_
#define _MY_LIST_H_


/* Linux list implementation plus a few addings */

/*
 *  this file is taken from the linux kernel sources.
 *  (linux-2.4.x/include/linux/list.h)
 * 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct my_list_head {
	struct my_list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct my_list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

#define ZERO_LIST_HEAD(ptr) do { \
	(ptr)->next = (struct my_list_head *)0; (ptr)->prev = (struct my_list_head *)0; \
} while (0)


/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static /*__inline__*/ void __my_list_add(struct my_list_head * new_entry,
	struct my_list_head * prev,
	struct my_list_head * next)
{
	next->prev = new_entry;
	new_entry->next = next;
	new_entry->prev = prev;
	prev->next = new_entry;
}

/**
 * my_list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static /*__inline__*/ void my_list_add(struct my_list_head *new_entry, struct my_list_head *head)
{
	__my_list_add(new_entry, head, head->next);
}

/**
 * my_list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static /*__inline__*/ void my_list_add_tail(struct my_list_head *new_entry, struct my_list_head *head)
{
	__my_list_add(new_entry, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static /*__inline__*/ void __my_list_del(struct my_list_head * prev,
				  struct my_list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * my_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: my_list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static /*__inline__*/ void my_list_del(struct my_list_head *entry)
{
	__my_list_del(entry->prev, entry->next);
}

/**
 * my_list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static /*__inline__*/ void my_list_del_init(struct my_list_head *entry)
{
	__my_list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry); 
}

static /*__inline__*/ void my_list_del_zero(struct my_list_head *entry)
{
	__my_list_del(entry->prev, entry->next);
	ZERO_LIST_HEAD(entry);
}


/**
 * my_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static /*__inline__*/ int my_list_empty(struct my_list_head *head)
{
	return head->next == head;
}

/**
 * my_list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static /*__inline__*/ void my_list_splice(struct my_list_head *list, struct my_list_head *head)
{
	struct my_list_head *first = list->next;

	if (first != list) {
		struct my_list_head *last = list->prev;
		struct my_list_head *at = head->next;

		first->prev = head;
		head->next = first;

		last->next = at;
		at->prev = last;
	}
}

/**
 * my_list_entry - get the struct for this entry
 * @ptr:	the &struct my_list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the my_list_struct within the struct.
 */
#define my_list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * my_list_for_each	-	iterate over a list
 * @pos:	the &struct my_list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define my_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); \
        	pos = pos->next)


#define my_list_for_each_backwards(pos, head) \
	for (pos = (head)->prev; pos != (head); \
        	pos = pos->prev)



/**
 * my_list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct my_list_head to use as a loop counter.
 * @n:		another &struct my_list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define my_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)



#define my_list_get_last( _head_ ) \
	(((_head_)->prev==(_head_))?0:((_head_)->prev))

#define my_list_get_first( _head_ ) \
	(((_head_)->next==(_head_))?0:((_head_)->next))

#if 0
static /*__inline__*/ struct my_list_head *my_list_get_last(struct my_list_head *head)
{
	if (head->prev == head)
		return (struct my_list_head *)0;
	else
		return head->prev;
}

static /*__inline__*/ struct my_list_head *my_list_get_first(struct my_list_head *head)
{
	if (head->next == head)
		return (struct my_list_head *)0;
	else
		return head->next;
}
#endif

#endif
