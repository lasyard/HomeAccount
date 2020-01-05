#include "AnnuallyFileRW.h"

long AnnuallyFileRW::calBalanceToYear(int year) const
{
    int balance = m_dt->initial;
    struct ulist_item *p;
    for (p = m_dt->pages.first; p != NULL; p = p->next) {
        struct page *o_pg = get_page(p);
        int o_year = str_to_int(o_pg->title.str, YEAR_LEN);
        if (o_year > year) break;
        balance = cal_page_balance(o_pg, balance);
    }
    return balance;
}

struct page *AnnuallyFileRW::getPage(int year)
{
    struct string str;
    string_mock(&str, m_title, YEAR_LEN);
    if (is_empty_data(m_dt)) {
        makeTitle(year);
        return add_page(m_dt, &str);
    }
    struct page *prev = NULL;
    struct ulist_item *p;
    for (p = m_dt->pages.first; p != NULL; prev = get_page(p), p = p->next) {
        struct page *o_pg = get_page(p);
        int o_year = str_to_int(o_pg->title.str, YEAR_LEN);
        if (year == o_year) {
            return o_pg;
        } else if (year < o_year) {
            makeTitle(year);
            if (prev == NULL) {
                return insert_page_head(m_dt, &str);
            } else {
                return insert_page(prev, &str);
            }
        }
    }
    makeTitle(year);
    return add_page(m_dt, &str);
}
