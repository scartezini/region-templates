/*
 * Ciprian Docan (2006-2009) TASSL Rutgers University
 *
 * The redistribution of the source code is subject to the terms of version 
 * 2 of the GNU General Public License: http://www.gnu.org/licenses/gpl.html.
 *
 */

#ifndef __LIST_H_
#define __LIST_H_

#include <stddef.h>

/* New list implementation 10/31/2008. */
/* Ref: see the list.h implementation in the Linux kernel. */

struct list_head {
	struct list_head *prev, *next;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)}

#define LIST_HEAD(name) \
        struct list_head name = LIST_HEAD_INIT(name)

static inline void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next)
{
	new->next = next;
	new->prev = prev;
	
        next->prev = new;
	prev->next = new;
}

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/* Add element 'new' after element 'head'. */
static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

/* Add element 'new' before element 'head'. */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

/* Unlink element 'old' from the list it belongs. */
static inline void list_del(struct list_head *old)
{
	old->prev->next = old->next;
	old->next->prev = old->prev;
	old->next = NULL;
	old->prev = NULL;
}

static inline int list_empty(struct list_head *head)
{
	return head->next == head;
}

#define list_entry(ptr, type, member)                                   \
        (type *) ((char *) ptr - offsetof(type, member))

#define list_for_each(pos, head)                                        \
        for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head)                                \
        for (pos = (head)->next, n = pos->next; pos != (head);          \
             pos = n, n = pos->next)

#define list_for_each_entry(pos, head, member)                          \
        for (pos = list_entry((head)->next, typeof(*pos), member);      \
             &pos->member !=( head);                                    \
             pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member)                  \
        for (pos = list_entry((head)->next, typeof(*pos), member),      \
                n = list_entry(pos->member.next, typeof(*pos), member); \
             &pos->member != (head);                                 \
             pos = n, n = list_entry(pos->member.next, typeof(*pos), member))

#endif
