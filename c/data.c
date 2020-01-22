#include <stdlib.h>

#include "cat.h"
#include "data.h"
#include "utils.h"

static void __init_item(struct item *it)
{
    ulist_item_init(&it->ulist);
    it->owner = NULL;
    it->money = 0;
    string_init(&it->desc);
    string_init(&it->comment);
    it->word = NULL;
}

static void __release_item(struct item *it)
{
    string_release(&it->desc);
    string_release(&it->comment);
}

static struct item *__new_item()
{
    struct item *p;
    if ((p = (struct item *)malloc(sizeof(struct item))) == NULL) return NULL;
    __init_item(p);
    return p;
}

static void __inc_items_num(struct page *pg)
{
    pg->items_num++;
    if (pg->owner != NULL) pg->owner->items_num++;
}

static void __dec_items_num(struct page *pg)
{
    pg->items_num--;
    if (pg->owner != NULL) pg->owner->items_num--;
}

static void __add_item_to_page(struct page *pg, struct item *it)
{
    it->owner = pg;
    ulist_add(&pg->items, &it->ulist);
    __inc_items_num(pg);
}

static void __add_item_to_page_head(struct page *pg, struct item *it)
{
    it->owner = pg;
    ulist_add_head(&pg->items, &it->ulist);
    __inc_items_num(pg);
}

static void __insert_item(struct item *pos, struct item *it)
{
    it->owner = pos->owner;
    ulist_ins(&pos->owner->items, &pos->ulist, &it->ulist);
    __inc_items_num(pos->owner);
}

struct item *add_dummy_item(struct page *pg)
{
    struct item *p;
    if ((p = __new_item()) == NULL) return NULL;
    __add_item_to_page(pg, p);
    return p;
}

struct item *add_item(struct page *pg, money_t money, struct string *desc, struct string *comment)
{
    struct item *p;
    if ((p = add_dummy_item(pg)) == NULL) return NULL;
    if (item_set(p, money, desc, comment) == NULL) {
        delete_item(p);
        return NULL;
    }
    return p;
}

struct item *add_simple_item(struct page *pg, money_t money)
{
    struct item *p;
    if ((p = add_dummy_item(pg)) == NULL) return NULL;
    p->money = money;
    return p;
}

struct item *insert_dummy_item(struct item *pos)
{
    struct item *p;
    if ((p = __new_item()) == NULL) return NULL;
    __insert_item(pos, p);
    return p;
}

struct item *insert_dummy_item_head(struct page *pg)
{
    struct item *p;
    if ((p = __new_item()) == NULL) return NULL;
    __add_item_to_page_head(pg, p);
    return p;
}

struct item *item_set(struct item *it, money_t money, struct string *desc, struct string *comment)
{
    if (item_set_desc(it, desc) == NULL) return NULL;
    if (item_set_comment(it, comment) == NULL) return NULL;
    return item_set_money(it, money);
}

struct item *item_set_money(struct item *it, money_t money)
{
    it->money = money;
    return it;
}

struct item *item_set_desc(struct item *it, struct string *desc)
{
    if (string_copy(&it->desc, desc) != NULL) {
        return it;
    }
    __release_item(it);
    return NULL;
}

struct item *item_set_comment(struct item *it, struct string *comment)
{
    if (string_copy(&it->comment, comment) != NULL) {
        return it;
    }
    __release_item(it);
    return NULL;
}

void clear_item(struct item *it)
{
    it->money = 0;
    string_init(&it->desc);
    string_init(&it->comment);
    it->word = NULL;
}

void delete_item(struct item *it)
{
    ulist_del(&it->owner->items, &it->ulist);
    __dec_items_num(it->owner);
    __release_item(it);
    free(it);
}

BOOL is_dummy_item(const struct item *it)
{
    return it->money == 0 && string_is_empty(&it->desc) && string_is_empty(&it->comment);
}

BOOL is_single_item(const struct item *it)
{
    return ulist_is_single(&it->owner->items, &it->ulist);
}

BOOL is_last_item(const struct item *it)
{
    return ulist_is_last(&it->owner->items, &it->ulist);
}

const char *item_cat_name(const struct item *it)
{
    if (it->word == NULL) return NULL;
    return it->word->owner->name.str;
}

static void __init_page(struct page *pg)
{
    ulist_item_init(&pg->ulist);
    ulist_head_init(&pg->items);
    pg->owner = NULL;
    pg->items_num = 0;
    string_init(&pg->title);
}

static struct page *__new_page()
{
    struct page *p;
    if ((p = (struct page *)malloc(sizeof(struct page))) == NULL) return NULL;
    __init_page(p);
    return p;
}

static void __release_items_of_page(struct page *pg)
{
    struct ulist_item *p, *q;
    for (p = pg->items.first; p != NULL; p = q) {
        struct item *it = get_item(p);
        q = p->next;
        __release_item(it);
        free(it);
    }
}

static void __release_page(struct page *pg)
{
    string_release(&pg->title);
    __release_items_of_page(pg);
}

void clear_page(struct page *pg)
{
    __release_items_of_page(pg);
    ulist_head_init(&pg->items);
    pg->items_num = 0;
}

static struct page *__new_page_with_title(struct string *title)
{
    struct page *p;
    if ((p = __new_page()) == NULL) return NULL;
    if (string_copy(&p->title, title) == NULL) {
        __release_page(p);
        free(p);
        return NULL;
    }
    return p;
}

static void __inc_pages_and_items_num(struct data *dt, const struct page *pg)
{
    dt->pages_num++;
    dt->items_num += pg->items_num;
}

static void __add_page_to_data(struct data *dt, struct page *pg)
{
    pg->owner = dt;
    ulist_add(&dt->pages, &pg->ulist);
    __inc_pages_and_items_num(dt, pg);
}

static struct page *__insert_page(struct page *pos, struct page *pg)
{
    pg->owner = pos->owner;
    ulist_ins(&pos->owner->pages, &pos->ulist, &pg->ulist);
    __inc_pages_and_items_num(pg->owner, pg);
    return pg;
}

static struct page *__insert_page_head(struct data *dt, struct page *pg)
{
    pg->owner = dt;
    ulist_add_head(&dt->pages, &pg->ulist);
    __inc_pages_and_items_num(pg->owner, pg);
    return pg;
}

struct page *add_page(struct data *dt, struct string *title)
{
    struct page *p;
    p = __new_page_with_title(title);
    if (p == NULL) return NULL;
    __add_page_to_data(dt, p);
    return p;
}

struct page *insert_page(struct page *pos, struct string *title)
{
    struct page *p;
    if ((p = __new_page_with_title(title)) == NULL) return NULL;
    __insert_page(pos, p);
    return p;
}

struct page *insert_page_head(struct data *dt, struct string *title)
{
    struct page *p;
    if ((p = __new_page_with_title(title)) == NULL) return NULL;
    __insert_page_head(dt, p);
    return p;
}

BOOL is_empty_page(const struct page *pg)
{
    return ulist_is_empty(&pg->items);
}

void init_data(struct data *dt)
{
    dt->initial = 0;
    dt->items_num = 0;
    dt->pages_num = 0;
    ulist_head_init(&dt->pages);
}

void clear_data(struct data *dt)
{
    struct ulist_item *p, *q;
    for (p = dt->pages.first; p != NULL; p = q) {
        struct page *pg = get_page(p);
        q = p->next;
        __release_page(pg);
        free(pg);
    }
    init_data(dt);
}

BOOL is_empty_data(const struct data *dt)
{
    return ulist_is_empty(&dt->pages);
}

struct data *add_dummy_item_to_empty_page(struct data *dt)
{
    struct ulist_item *p;
    for (p = dt->pages.first; p != NULL; p = p->next) {
        if (is_empty_page(get_page(p))) {
            if (add_dummy_item(get_page(p)) == NULL) return NULL;
        }
    }
    return dt;
}

money_t cal_item_balance(const struct item *it, money_t initial)
{
    return initial - it->money;
}

money_t cal_page_balance(const struct page *pg, money_t initial)
{
    money_t balance = initial;
    struct ulist_item *p;
    for (p = pg->items.first; p != NULL; p = p->next) {
        balance = cal_item_balance(get_item(p), balance);
    }
    return balance;
}

money_t cal_data_balance(const struct data *dt, money_t initial)
{
    money_t balance = initial;
    struct ulist_item *p;
    for (p = dt->pages.first; p != NULL; p = p->next) {
        balance = cal_page_balance(get_page(p), balance);
    }
    return balance;
}

void cal_page_income_outlay(const struct page *pg, money_t *income, money_t *outlay)
{
    struct ulist_item *p;
    money_t i = 0, o = 0;
    for (p = pg->items.first; p != NULL; p = p->next) {
        struct item *it = get_item(p);
        if (it->money < 0) {
            i -= it->money;
        } else {
            o += it->money;
        }
    }
    *income = i;
    *outlay = o;
}

void cal_data_income_outlay(const struct data *dt, money_t *income, money_t *outlay)
{
    struct ulist_item *p;
    money_t i = 0, o = 0;
    for (p = dt->pages.first; p != NULL; p = p->next) {
        money_t ii, oo;
        cal_page_income_outlay(get_page(p), &ii, &oo);
        i += ii;
        o += oo;
    }
    *income = i;
    *outlay = o;
}
