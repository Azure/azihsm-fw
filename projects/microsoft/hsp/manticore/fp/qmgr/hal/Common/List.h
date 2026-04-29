// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once

typedef struct ListHead_t
{
    struct ListHead_t* pPrev;
    struct ListHead_t* pNext;
} ListHead_t, *PListHead_t;

typedef struct CountedListHead_t
{
    struct ListHead_t* pPrev;
    struct ListHead_t* pNext;
    uint32_t node_count;
} CountedListHead_t, *PCountedListHead_t;

#define CONTAINER_OF(ptr, type, member) \
    (type*)((char*)(ptr) - (char*)&((type*)0)->member)

// #define TYPE_OFFSET(type, member)  ((uint32_t)(&((type*)0)->member))
#define TYPE_OFFSET(type, member) offsetof(type, member)
/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */
static inline void init_list_head(PListHead_t list)
{
    list->pNext = list;
    list->pPrev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(PListHead_t one,
                              PListHead_t prev,
                              PListHead_t next)
{
    next->pPrev = one;
    one->pNext = next;
    one->pPrev = prev;
    prev->pNext = one;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(PListHead_t one, PListHead_t head)
{
    __list_add(one, head, head->pNext);
}


/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(PListHead_t one, PListHead_t head)
{
    __list_add(one, head->pPrev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(PListHead_t  prev, PListHead_t  next)
{
    next->pPrev = prev;
    prev->pNext = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void __list_del_entry(PListHead_t entry)
{
    __list_del(entry->pPrev, entry->pNext);
}

static inline void list_del(PListHead_t entry)
{
    __list_del(entry->pPrev, entry->pNext);
    entry->pNext = NULL;
    entry->pPrev = NULL;
}

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void list_replace(PListHead_t old,
                                PListHead_t one)
{
    one->pNext = old->pNext;
    one->pNext->pPrev = one;
    one->pPrev = old->pPrev;
    one->pPrev->pNext = one;
}

static inline void list_replace_init(PListHead_t old,
                                     PListHead_t one)
{
    list_replace(old, one);
    init_list_head(old);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void list_del_init(PListHead_t entry)
{
    __list_del_entry(entry);
    init_list_head(entry);
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void list_move(PListHead_t list, PListHead_t head)
{
    __list_del_entry(list);
    list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(PListHead_t list,
                                  PListHead_t head)
{
    __list_del_entry(list);
    list_add_tail(list, head);
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int list_is_last(const PListHead_t list,
                               const PListHead_t head)
{
    return list->pNext == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const PListHead_t head)
{
    return head->pNext == head;
}

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
static inline int list_empty_careful(const PListHead_t head)
{
    PListHead_t next = head->pNext;
    return (next == head) && (next == head->pPrev);
}

/**
 * list_rotate_left - rotate the list to the left
 * @head: the head of the list
 */
static inline void list_rotate_left(PListHead_t head)
{
    PListHead_t first;

    if (!list_empty(head))
    {
        first = head->pNext;
        list_move_tail(first, head);
    }
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int list_is_singular(const PListHead_t head)
{
    return !list_empty(head) && (head->pNext == head->pPrev);
}

static inline void __list_cut_position(PListHead_t list,
                                       PListHead_t head,
                                       PListHead_t entry)
{
    PListHead_t new_first = entry->pNext;
    list->pNext = head->pNext;
    list->pNext->pPrev = list;
    list->pPrev = entry;
    entry->pNext = list;
    head->pNext = new_first;
    new_first->pPrev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *    and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void list_cut_position(PListHead_t list,
                                     PListHead_t head,
                                     PListHead_t entry)
{
    if (list_empty(head))
    {
        return;
    }

    if (list_is_singular(head) &&
        ((head->pNext != entry) && (head != entry)))
    {
        return;
    }

    if (entry == head)
    {
        init_list_head(list);
    }
    else
    {
        __list_cut_position(list, head, entry);
    }
}

static inline void __list_splice(const PListHead_t list,
                                 PListHead_t prev,
                                 PListHead_t next)
{
    PListHead_t first = list->pNext;
    PListHead_t last = list->pPrev;

    first->pPrev = prev;
    prev->pNext = first;

    last->pNext = next;
    next->pPrev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice(const PListHead_t list,
                               PListHead_t head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head, head->pNext);
    }
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice_tail(PListHead_t list,
                                    PListHead_t head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head->pPrev, head);
    }
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void list_splice_init(PListHead_t list,
                                    PListHead_t head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head, head->pNext);
        init_list_head(list);
    }
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void list_splice_tail_init(PListHead_t list,
                                         PListHead_t head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head->pPrev, head);
        init_list_head(list);
    }
}
/**
 * list_get_first - get the struct for this entry
 * @ptr:    the &struct ListHead_t pointer.
 */
static inline PListHead_t list_get_first(PListHead_t head)
{
    ListHead_t* one = NULL;

    if (!list_empty(head))
    {
        one = (head)->pNext;
        list_del(one);
    }

    return (one);
}

/**
 * LIST_ENTRY - get the struct for this entry
 * @ptr:    the &struct ListHead_t pointer.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the ListHead_t within the struct.
 */
#define LIST_ENTRY(ptr, type, member) \
    CONTAINER_OF(ptr, type, member)

/**
 * LIST_FIRST_ENTRY - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the ListHead_t within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define LIST_FIRST_ENTRY(ptr, type, member) \
    LIST_ENTRY((ptr)->pNext, type, member)

/**
 * LIST_LAST_ENTRY - get the last element from a list
 * @ptr:    the list head to take the element from.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the ListHead_t within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define LIST_LAST_ENTRY(ptr, type, member) \
    LIST_ENTRY((ptr)->pPrev, type, member)

/**
 * LIST_FIRST_ENTRY_OR_NULL - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the ListHead_t within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define LIST_FIRST_ENTRY_OR_NULL(ptr, type, member) \
    (!list_empty(ptr) ? LIST_FIRST_ENTRY(ptr, type, member) : NULL)

/**
 * LIST_NEXT_ENTRY - get the next element in list
 * @pos:    the type * to cursor
 * @member:    the name of the ListHead_t within the struct.
 */
#define LIST_NEXT_ENTRY(pos, member) \
    LIST_ENTRY((pos)->member.pNext, typeof(*(pos)), member)

/**
 * LIST_PREV_ENTRY - get the prev element in list
 * @pos:    the type * to cursor
 * @member:    the name of the ListHead_t within the struct.
 */
#define LIST_PREV_ENTRY(pos, member) \
    LIST_ENTRY((pos)->member.pPrev, typeof(*(pos)), member)

/**
 * LIST_FOR_EACH    -    iterate over a list
 * @pos:    the &struct ListHead_t to use as a loop cursor.
 * @head:    the head for your list.
 */
#define LIST_FOR_EACH(pos, head) \
    for (pos = (head)->pNext; pos != (head); pos = pos->pNext)

/**
 * LIST_FOR_EACH_ENTRY_TYPE    -    iterate over list of given type
 * @pos:    the type * to use as a loop counter.
 * @head:    the head for your list.
 * @member:    the name of the list_struct within the struct.
 * @type:    the type of the struct this is embedded in.
 */
#define LIST_FOR_EACH_ENTRY_TYPE(pos, head, type, member) \
    for (pos = LIST_ENTRY((head)->pNext, type, member);   \
         &pos->member != (ListHead_t*)(head);             \
         pos = LIST_ENTRY(pos->member.pNext, type, member))

/**
 * LIST_FOR_EACH_PREV    -    iterate over a list backwards
 * @pos:    the &struct ListHead_t to use as a loop cursor.
 * @head:    the head for your list.
 */
#define LIST_FOR_EACH_PREV(pos, head) \
    for (pos = (head)->pPrev; pos != (head); pos = pos->pPrev)

/**
 * LIST_FOR_EACH_SAFE - iterate over a list safe against removal of list entry
 * @pos:    the &struct ListHead_t to use as a loop cursor.
 * @n:        another &struct ListHead_t to use as temporary storage
 * @head:    the head for your list.
 */
#define LIST_FOR_EACH_SAFE(pos, n, head)                     \
    for (pos = (head)->pNext, n = pos->pNext; pos != (head); \
         pos = n, n = pos->pNext)

/**
 * LIST_FOR_EACH_PREV_SAFE - iterate over a list backwards safe against removal of list entry
 * @pos:    the &struct ListHead_t to use as a loop cursor.
 * @n:        another &struct ListHead_t to use as temporary storage
 * @head:    the head for your list.
 */
#define LIST_FOR_EACH_PREV_SAFE(pos, n, head) \
    for (pos = (head)->pPrev, n = pos->pPrev; \
         pos != (head);                       \
         pos = n, n = pos->pPrev)

/**
 * LIST_FOR_EACH_ENTRY    -    iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the ListHead_t within the struct.
 */
#define LIST_FOR_EACH_ENTRY(pos, head, member)               \
    for (pos = LIST_FIRST_ENTRY(head, typeof(*pos), member); \
         &pos->member != (head);                             \
         pos = LIST_NEXT_ENTRY(pos, member))

/**
 * LIST_FOR_EACH_ENTRY_REVERSE - iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the ListHead_t within the struct.
 */
#define LIST_FOR_EACH_ENTRY_REVERSE(pos, head, member)      \
    for (pos = LIST_LAST_ENTRY(head, typeof(*pos), member); \
         &pos->member != (head);                            \
         pos = LIST_PREV_ENTRY(pos, member))

/**
 * LIST_PREPARE_ENTRY - prepare a pos entry for use in LIST_FOR_EACH_ENTRY_CONTINUE()
 * @pos:    the type * to use as a start point
 * @head:    the head of the list
 * @member:    the name of the ListHead_t within the struct.
 *
 * Prepares a pos entry for use as a start point in LIST_FOR_EACH_ENTRY_CONTINUE().
 */
#define LIST_PREPARE_ENTRY(pos, head, member) \
    ((pos) ? : LIST_ENTRY(head, typeof(*pos), member))

/**
 * LIST_FOR_EACH_ENTRY_CONTINUE - continue iteration over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the ListHead_t within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define LIST_FOR_EACH_ENTRY_CONTINUE(pos, head, member) \
    for (pos = LIST_NEXT_ENTRY(pos, member);            \
         &pos->member != (head);                        \
         pos = LIST_NEXT_ENTRY(pos, member))

/**
 * LIST_FOR_EACH_ENTRY_CONTINUE_REVERSE - iterate backwards from the given point
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the ListHead_t within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define LIST_FOR_EACH_ENTRY_CONTINUE_REVERSE(pos, head, member) \
    for (pos = LIST_PREV_ENTRY(pos, member);                    \
         &pos->member != (head);                                \
         pos = LIST_PREV_ENTRY(pos, member))

/**
 * LIST_FOR_EACH_ENTRY_FROM - iterate over list of given type from the current point
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the ListHead_t within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define LIST_FOR_EACH_ENTRY_FROM(pos, head, member) \
    for (; &pos->member != (head);                  \
         pos = LIST_NEXT_ENTRY(pos, member))

/**
 * LIST_FOR_EACH_ENTRY_SAFE - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the ListHead_t within the struct.
 */
#define LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member)       \
    for (pos = LIST_FIRST_ENTRY(head, typeof(*pos), member), \
         n = LIST_NEXT_ENTRY(pos, member);                   \
         &pos->member != (head);                             \
         pos = n, n = LIST_NEXT_ENTRY(n, member))

/**
 * LIST_FOR_EACH_ENTRY_SAFE_CONTINUE - continue list iteration safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the ListHead_t within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define LIST_FOR_EACH_ENTRY_SAFE_CONTINUE(pos, n, head, member) \
    for (pos = LIST_NEXT_ENTRY(pos, member),                    \
         n = LIST_NEXT_ENTRY(pos, member);                      \
         &pos->member != (head);                                \
         pos = n, n = LIST_NEXT_ENTRY(n, member))

/**
 * LIST_FOR_EACH_ENTRY_SAFE_FROM - iterate over list from current point safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the ListHead_t within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define LIST_FOR_EACH_ENTRY_SAFE_FROM(pos, n, head, member) \
    for (n = LIST_NEXT_ENTRY(pos, member);                  \
         &pos->member != (head);                            \
         pos = n, n = LIST_NEXT_ENTRY(n, member))

/**
 * LIST_FOR_EACH_ENTRY_SAFE_REVERSE - iterate backwards over list safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the ListHead_t within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define LIST_FOR_EACH_ENTRY_SAFE_REVERSE(pos, n, head, member) \
    for (pos = LIST_LAST_ENTRY(head, typeof(*pos), member),    \
         n = LIST_PREV_ENTRY(pos, member);                     \
         &pos->member != (head);                               \
         pos = n, n = LIST_PREV_ENTRY(n, member))

/**
 * LIST_SAFE_RESET_NEXT - reset a stale list_for_each_entry_safe loop
 * @pos:    the loop cursor used in the list_for_each_entry_safe loop
 * @n:        temporary storage used in list_for_each_entry_safe
 * @member:    the name of the ListHead_t within the struct.
 *
 * LIST_SAFE_RESET_NEXT is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and LIST_SAFE_RESET_NEXT is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define LIST_SAFE_RESET_NEXT(pos, n, member) \
    n = LIST_NEXT_ENTRY(pos, member)
