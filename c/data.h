#ifndef _DATA_H_
#define _DATA_H_

#include <time.h>

#include "err.h"
#include "str.h"
#include "ulist.h"

struct data {
    struct ulist_head pages;
    int pages_num;
    int items_num;
    money_t initial;
};

struct page {
    struct ulist_item ulist;
    struct data *owner;
    struct ulist_head items;
    int items_num;
    struct string title;
};

struct item {
    struct ulist_item ulist;
    struct page *owner;
    time_t time;
    money_t money;
    struct string desc;
    struct string comment;
    struct word *word;
};

#define get_item(ptr) ulist_entry(ptr, struct item, ulist)
#define get_page(ptr) ulist_entry(ptr, struct page, ulist)

#ifdef __cplusplus
extern "C" {
#endif

struct item *add_dummy_item(struct page *pg);
struct item *add_item(struct page *pg, time_t time, money_t money, struct string *desc, struct string *comment);
struct item *add_simple_item(struct page *pg, money_t money);

struct item *insert_dummy_item(struct item *pos);
struct item *insert_dummy_item_head(struct page *pg);

struct item *item_set(struct item *it, time_t time, money_t money, struct string *desc, struct string *comment);
struct item *item_set_time(struct item *it, time_t time);
struct item *item_set_money(struct item *it, money_t money);
struct item *item_set_desc(struct item *it, struct string *desc);
struct item *item_set_comment(struct item *it, struct string *comment);
void clear_item(struct item *it);
void delete_item(struct item *it);
const char *item_cat_name(const struct item *it);

BOOL is_dummy_item(const struct item *it);
BOOL is_single_item(const struct item *it);
BOOL is_last_item(const struct item *it);

void clear_page(struct page *pg);
struct page *add_page(struct data *dt, struct string *title);
struct page *insert_page(struct page *pos, struct string *title);
struct page *insert_page_head(struct data *dt, struct string *title);
BOOL is_empty_page(const struct page *pg);

void init_data(struct data *dt);
void clear_data(struct data *dt);
BOOL is_empty_data(const struct data *dt);

struct data *add_dummy_item_to_empty_page(struct data *dt);

money_t cal_item_balance(const struct item *it, money_t initial);
money_t cal_page_balance(const struct page *pg, money_t initial);
money_t cal_data_balance(const struct data *dt, money_t initial);

void cal_page_income_outlay(const struct page *pg, money_t *income, money_t *outlay);
void cal_data_income_outlay(const struct data *dt, money_t *income, money_t *outlay);

err_code parse_data(struct data *dt, const char *line);

#ifdef __cplusplus
}
#endif

#endif
