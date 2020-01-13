#ifndef _DATA_H_
#define _DATA_H_

#include "str.h"
#include "ulist.h"

struct data {
    struct ulist_head pages;
    int pages_num;
    int items_num;
    long initial;
};

struct page {
    struct ulist_item ulist;
    struct data *owner;
    struct ulist_head items;
    int items_num;
    struct string title;
};

#define ITEM_NORMAL 0x00000000
#define ITEM_TO_BE_DELETED 0x00000001

struct item {
    struct ulist_item ulist;
    struct page *owner;
    long money;
    struct string desc;
    struct string comment;
    struct cat_node *cat;
    int flag;
};

#define get_item(ptr) ulist_entry(ptr, struct item, ulist)
#define get_page(ptr) ulist_entry(ptr, struct page, ulist)

#ifdef __cplusplus
extern "C" {
#endif

struct item *add_dummy_item(struct page *pg);
struct item *add_item(struct page *pg, long money, struct string *desc, struct string *comment);
struct item *add_simple_item(struct page *pg, long money);

struct item *insert_dummy_item(struct item *pos);
struct item *insert_dummy_item_head(struct page *pg);

struct item *item_set(struct item *it, long money, struct string *desc, struct string *comment);
struct item *item_set_money(struct item *it, long money);
struct item *item_set_desc(struct item *it, struct string *desc);
struct item *item_set_comment(struct item *it, struct string *comment);

void delete_item(struct item *it);
void set_item_delete(struct item *it);

BOOL is_dummy_item(const struct item *it);

void clear_page(struct page *pg);
struct page *add_page(struct data *dt, struct string *title);
struct page *insert_page(struct page *pos, struct string *title);
struct page *insert_page_head(struct data *dt, struct string *title);
BOOL is_empty_page(const struct page *pg);

void init_data(struct data *dt);
void clear_data(struct data *dt);
BOOL is_empty_data(const struct data *dt);

struct item *nth_item_of_data(struct data *dt, int n);

struct data *add_dummy_item_to_empty_page(struct data *dt);
void delete_items_from_data(struct data *dt);

long cal_item_balance(const struct item *it, long initial);
long cal_page_balance(const struct page *pg, long initial);
long cal_data_balance(const struct data *dt, long initial);

void cal_page_income_outlay(const struct page *pg, long *income, long *outlay);
void cal_data_income_outlay(const struct data *dt, long *income, long *outlay);

#ifdef __cplusplus
}
#endif

#endif
