#ifndef _ULIST_H_
#define _ULIST_H_

#include "core_defs.h"

struct ulist_item {
    struct ulist_item *next;
};

struct ulist_head {
    struct ulist_item *first;
    struct ulist_item *last;
};

#define ulist_entry(ptr, type, member) container_of(ptr, type, member)

#ifdef __cplusplus
extern "C" {
#endif

void ulist_item_init(struct ulist_item *item);
void ulist_head_init(struct ulist_head *head);

void ulist_add(struct ulist_head *head, struct ulist_item *item);
void ulist_add_head(struct ulist_head *head, struct ulist_item *item);

void ulist_del(struct ulist_head *head, struct ulist_item *item);
void ulist_ins(struct ulist_head *head, struct ulist_item *pos, struct ulist_item *item);

struct ulist_item *ulist_first(struct ulist_head *head);
struct ulist_item *ulist_last(struct ulist_head *head);

BOOL ulist_is_empty(const struct ulist_head *head);
BOOL ulist_is_first(const struct ulist_head *head, const struct ulist_item *item);
BOOL ulist_is_last(const struct ulist_head *head, const struct ulist_item *item);
BOOL ulist_is_single(const struct ulist_head *head, const struct ulist_item *item);

#ifdef __cplusplus
}
#endif

#endif
