#include "cal.h"
#include "cat.h"
#include "data.h"

static void __cal_item_total(struct item *it, struct cat_root *cat)
{
    update_item_cat(it, cat);
    if (it->word == NULL) {
        if (it->money < 0) {
            cat->no_cat_in_sum -= it->money;
        } else {
            cat->no_cat_out_sum += it->money;
        }
    } else {
        if (it->money < 0) {
            it->word->total -= it->money;
            it->word->count++;
        } else {
            it->word->total += it->money;
            it->word->count++;
        }
    }
}

static void __cal_page_total(struct page *pg, struct cat_root *cat)
{
    struct ulist_item *p;
    for (p = pg->items.first; p != NULL; p = p->next) {
        __cal_item_total(get_item(p), cat);
    }
}

void cal_data_total(struct data *dt, struct cat_root *cat)
{
    struct ulist_item *p;
    for (p = dt->pages.first; p != NULL; p = p->next) {
        __cal_page_total(get_page(p), cat);
    }
}

void update_item_cat(struct item *it, struct cat_root *cat)
{
    if (mtree_is_leaf(&cat->root)) {
        it->word = NULL;
    } else if (it->money < 0) {
        it->word = get_word_by_name(mtree_first_child(&cat->root), &it->desc);
    } else {
        it->word = get_word_by_name(mtree_last_child(&cat->root), &it->desc);
    }
}
