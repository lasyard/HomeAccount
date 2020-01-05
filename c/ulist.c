#include "ulist.h"

void ulist_item_init(struct ulist_item *item)
{
    item->next = NULL;
}

void ulist_head_init(struct ulist_head *head)
{
    head->first = head->last = NULL;
}

void ulist_add(struct ulist_head *head, struct ulist_item *item)
{
    if (head->first == NULL) {
        head->first = head->last = item;
    } else {
        head->last->next = item;
        head->last = item;
    }
}

void ulist_add_head(struct ulist_head *head, struct ulist_item *item)
{
    item->next = head->first;
    head->first = item;
    if (head->last == NULL) head->last = item;
}

void ulist_del(struct ulist_head *head, struct ulist_item *item)
{
    struct ulist_item *p;
    if (head->first == item) {
        head->first = item->next;
        if (head->last == item) head->last = head->first;
        return;
    }
    for (p = head->first; p->next != NULL; p = p->next) {
        if (p->next == item) {
            p->next = item->next;
            if (head->last == item) head->last = p;
            break;
        }
    }
}

void ulist_ins(struct ulist_head *head, struct ulist_item *pos, struct ulist_item *item)
{
    item->next = pos->next;
    pos->next = item;
    if (head->last == pos) head->last = item;
}

struct ulist_item *ulist_first(struct ulist_head *head)
{
    return head->first;
}

struct ulist_item *ulist_last(struct ulist_head *head)
{
    return head->last;
}

BOOL ulist_is_empty(const struct ulist_head *head)
{
    return head->first == NULL;
}

BOOL ulist_is_first(const struct ulist_head *head, const struct ulist_item *item)
{
    return head->first == item;
}

BOOL ulist_is_last(const struct ulist_head *head, const struct ulist_item *item)
{
    return head->last == item;
}

BOOL ulist_is_single(const struct ulist_head *head, const struct ulist_item *item)
{
    return head->first == item && head->last == item;
}
