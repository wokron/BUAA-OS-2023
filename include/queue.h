#ifndef _SYS_QUEUE_H_
#define _SYS_QUEUE_H_

/*
 * This file defines three types of data structures: lists, tail queues,
 * and circular queues.
 *
 * A list is headed by a single forward pointer(or an array of forward
 * pointers for a hash table header). The elements are doubly linked
 * so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before
 * or after an existing element or at the head of the list. A list
 * may only be traversed in the forward direction.
 *
 * A tail queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or
 * after an existing element, at the head of the list, or at the end of
 * the list. A tail queue may only be traversed in the forward direction.
 *
 * A circle queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or after
 * an existing element, at the head of the list, or at the end of the list.
 * A circle queue may be traversed in either direction, but has a more
 * complex end of list detection.
 *
 * For details on the use of these macros, see the queue(3) manual page.
 */

/*
 * List declarations.
 */

/*
 * A list is headed by a structure defined by the LIST_HEAD macro.  This structure con‐
 * tains a single pointer to the first element on the list.  The elements are doubly
 * linked so that an arbitrary element can be removed without traversing the list.  New
 * elements can be added to the list after an existing element or at the head of the list.
 * A LIST_HEAD structure is declared as follows:
 *
 *       LIST_HEAD(HEADNAME, TYPE) head;
 *
 * where HEADNAME is the name of the structure to be defined, and TYPE is the type of the
 * elements to be linked into the list.
 */
#define LIST_HEAD(name, type)                                                                      \
	struct name {                                                                              \
		struct type *lh_first; /* first element */                                         \
	}

/*
 * Set a list head variable to LIST_HEAD_INITIALIZER(head)
 * to reset it to the empty list.
 */
#define LIST_HEAD_INITIALIZER(head)                                                                \
	{ NULL }

/*
 * Use this inside a structure "LIST_ENTRY(type) field" to use
 * x as the list piece.
 *
 * The le_prev points at the pointer to the structure containing
 * this very LIST_ENTRY, so that if we want to remove this list entry,
 * we can do *le_prev = le_next to update the structure pointing at us.
 */
#define LIST_ENTRY(type)                                                                           \
	struct {                                                                                   \
		struct type *le_next;  /* next element */                                          \
		struct type **le_prev; /* address of previous next element */                      \
	}

/*
 * List functions.
 */

/*
 * Detect the list named "head" is empty.
 */
#define LIST_EMPTY(head) ((head)->lh_first == NULL)

/*
 * Return the first element in the list named "head".
 */
#define LIST_FIRST(head) ((head)->lh_first)

/*
 * Iterate over the elements in the list named "head".
 * During the loop, assign the list elements to the variable "var"
 * and use the LIST_ENTRY structure member "field" as the link field.
 */
#define LIST_FOREACH(var, head, field)                                                             \
	for ((var) = LIST_FIRST((head)); (var); (var) = LIST_NEXT((var), field))

/*
 * Reset the list named "head" to the empty list.
 */
#define LIST_INIT(head)                                                                            \
	do {                                                                                       \
		LIST_FIRST((head)) = NULL;                                                         \
	} while (0)

/*
 * Insert the element 'elm' *after* 'listelm' which is already in the list. The 'field'
 * name is the link element as above.
 *
 * Hint:
 * Step 1: assign 'elm.next' from 'listelm.next'.
 * Step 2: if 'listelm.next' is not NULL, then assign 'listelm.next.pre' from a proper value.
 * Step 3: assign 'listelm.next' from a proper value.
 * Step 4: assign 'elm.pre' from a proper value.
 */
#define LIST_INSERT_AFTER(listelm, elm, field)                                                     \
	/* Exercise 2.2: Your code here. */  \

/*
 * Insert the element "elm" *before* the element "listelm" which is
 * already in the list.  The "field" name is the link element
 * as above.
 */
#define LIST_INSERT_BEFORE(listelm, elm, field)                                                    \
	do {                                                                                       \
		(elm)->field.le_prev = (listelm)->field.le_prev;                                   \
		LIST_NEXT((elm), field) = (listelm);                                               \
		*(listelm)->field.le_prev = (elm);                                                 \
		(listelm)->field.le_prev = &LIST_NEXT((elm), field);                               \
	} while (0)

/*
 * Insert the element "elm" at the head of the list named "head".
 * The "field" name is the link element as above.
 */
#define LIST_INSERT_HEAD(head, elm, field)                                                         \
	do {                                                                                       \
		if ((LIST_NEXT((elm), field) = LIST_FIRST((head))) != NULL)                        \
			LIST_FIRST((head))->field.le_prev = &LIST_NEXT((elm), field);              \
		LIST_FIRST((head)) = (elm);                                                        \
		(elm)->field.le_prev = &LIST_FIRST((head));                                        \
	} while (0)

#define LIST_NEXT(elm, field) ((elm)->field.le_next)

/*
 * Remove the element "elm" from the list.
 * The "field" name is the link element as above.
 */
#define LIST_REMOVE(elm, field)                                                                    \
	do {                                                                                       \
		if (LIST_NEXT((elm), field) != NULL)                                               \
			LIST_NEXT((elm), field)->field.le_prev = (elm)->field.le_prev;             \
		*(elm)->field.le_prev = LIST_NEXT((elm), field);                                   \
	} while (0)

/*
 * Tail queue definitions.
 */
#define _TAILQ_HEAD(name, type, qual)                                                              \
	struct name {                                                                              \
		qual type *tqh_first;	   /* first element */                                     \
		qual type *qual *tqh_last; /* addr of last next element */                         \
	}
#define TAILQ_HEAD(name, type) _TAILQ_HEAD(name, struct type, )

#define TAILQ_HEAD_INITIALIZER(head)                                                               \
	{ NULL, &(head).tqh_first }

#define _TAILQ_ENTRY(type, qual)                                                                   \
	struct {                                                                                   \
		qual type *tqe_next;	   /* next element */                                      \
		qual type *qual *tqe_prev; /* address of previous next element */                  \
	}
#define TAILQ_ENTRY(type) _TAILQ_ENTRY(struct type, )

/*
 * Tail queue functions.
 */
#define TAILQ_INIT(head)                                                                           \
	do {                                                                                       \
		(head)->tqh_first = NULL;                                                          \
		(head)->tqh_last = &(head)->tqh_first;                                             \
	} while (/*CONSTCOND*/ 0)

#define TAILQ_INSERT_HEAD(head, elm, field)                                                        \
	do {                                                                                       \
		if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)                           \
			(head)->tqh_first->field.tqe_prev = &(elm)->field.tqe_next;                \
		else                                                                               \
			(head)->tqh_last = &(elm)->field.tqe_next;                                 \
		(head)->tqh_first = (elm);                                                         \
		(elm)->field.tqe_prev = &(head)->tqh_first;                                        \
	} while (/*CONSTCOND*/ 0)

#define TAILQ_INSERT_TAIL(head, elm, field)                                                        \
	do {                                                                                       \
		(elm)->field.tqe_next = NULL;                                                      \
		(elm)->field.tqe_prev = (head)->tqh_last;                                          \
		*(head)->tqh_last = (elm);                                                         \
		(head)->tqh_last = &(elm)->field.tqe_next;                                         \
	} while (/*CONSTCOND*/ 0)

#define TAILQ_INSERT_AFTER(head, listelm, elm, field)                                              \
	do {                                                                                       \
		if (((elm)->field.tqe_next = (listelm)->field.tqe_next) != NULL)                   \
			(elm)->field.tqe_next->field.tqe_prev = &(elm)->field.tqe_next;            \
		else                                                                               \
			(head)->tqh_last = &(elm)->field.tqe_next;                                 \
		(listelm)->field.tqe_next = (elm);                                                 \
		(elm)->field.tqe_prev = &(listelm)->field.tqe_next;                                \
	} while (/*CONSTCOND*/ 0)

#define TAILQ_INSERT_BEFORE(listelm, elm, field)                                                   \
	do {                                                                                       \
		(elm)->field.tqe_prev = (listelm)->field.tqe_prev;                                 \
		(elm)->field.tqe_next = (listelm);                                                 \
		*(listelm)->field.tqe_prev = (elm);                                                \
		(listelm)->field.tqe_prev = &(elm)->field.tqe_next;                                \
	} while (/*CONSTCOND*/ 0)

#define TAILQ_REMOVE(head, elm, field)                                                             \
	do {                                                                                       \
		if (((elm)->field.tqe_next) != NULL)                                               \
			(elm)->field.tqe_next->field.tqe_prev = (elm)->field.tqe_prev;             \
		else                                                                               \
			(head)->tqh_last = (elm)->field.tqe_prev;                                  \
		*(elm)->field.tqe_prev = (elm)->field.tqe_next;                                    \
	} while (/*CONSTCOND*/ 0)

#define TAILQ_FOREACH(var, head, field)                                                            \
	for ((var) = ((head)->tqh_first); (var); (var) = ((var)->field.tqe_next))

#define TAILQ_FOREACH_REVERSE(var, head, headname, field)                                          \
	for ((var) = (*(((struct headname *)((head)->tqh_last))->tqh_last)); (var);                \
	     (var) = (*(((struct headname *)((var)->field.tqe_prev))->tqh_last)))

#define TAILQ_CONCAT(head1, head2, field)                                                          \
	do {                                                                                       \
		if (!TAILQ_EMPTY(head2)) {                                                         \
			*(head1)->tqh_last = (head2)->tqh_first;                                   \
			(head2)->tqh_first->field.tqe_prev = (head1)->tqh_last;                    \
			(head1)->tqh_last = (head2)->tqh_last;                                     \
			TAILQ_INIT((head2));                                                       \
		}                                                                                  \
	} while (/*CONSTCOND*/ 0)

/*
 * Tail queue access methods.
 */
#define TAILQ_EMPTY(head) ((head)->tqh_first == NULL)
#define TAILQ_FIRST(head) ((head)->tqh_first)
#define TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)

#define TAILQ_LAST(head, headname) (*(((struct headname *)((head)->tqh_last))->tqh_last))
#define TAILQ_PREV(elm, headname, field) (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

#endif
