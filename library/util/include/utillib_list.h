
#ifndef _UTIL_LIST_H_
#define _UTIL_LIST_H_


/* Linux style double-linked list implementation */

typedef struct list_ {
    struct list_ *next;
    struct list_ *prev;
} list_t;


/* the smallest data structure we can use */
#define LIST_MIN_ITEM_SIZE sizeof(list_t)

/* for-each operation on a list, given a temporary variable */
#define LIST_FOREACH(root, tmp) LIST_FOREACH_TYPED(root, list_t, tmp)

#define LIST_FOREACH_TYPED(root, type, tmp) \
    for((tmp) = (type*) (root)->next; (tmp) != (type *) (root); (tmp) = (tmp)->next)

/* the list API */
extern void list_init(list_t *list);
extern void list_cleanup(list_t *list);

extern void list_add(list_t *list, void *item);
extern void list_remove(list_t *list, void *item);
extern void * list_pop(list_t *list);

extern BOOL list_find(list_t *list, void *item);
extern BOOL list_is_empty(list_t *list);
extern int list_size_get(list_t *list);

#endif /* _UTIL_LIST_ */
