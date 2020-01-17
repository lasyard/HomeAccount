#include "MonthlyFileRW.h"

money_t MonthlyFileRW::calBalanceToMonth(int month) const
{
    money_t balance = m_dt->initial;
    struct ulist_item *p;
    for (p = m_dt->pages.first; p != NULL; p = p->next) {
        struct page *o_pg = get_page(p);
        int o_month = str_to_int(o_pg->title.str + YEAR_LEN + 1, MONTH_LEN);
        if (o_month > month) break;
        balance = cal_page_balance(o_pg, balance);
    }
    return balance;
}

struct page *MonthlyFileRW::getPage(int month)
{
    struct string str;
    string_mock(&str, m_title, YEAR_LEN + 1 + MONTH_LEN);
    if (is_empty_data(m_dt)) {
        makeTitle(month);
        return add_page(m_dt, &str);
    }
    struct page *prev = NULL;
    struct ulist_item *p;
    for (p = m_dt->pages.first; p != NULL; prev = get_page(p), p = p->next) {
        struct page *o_pg = get_page(p);
        int o_month = str_to_int(o_pg->title.str + YEAR_LEN + 1, MONTH_LEN);
        if (month == o_month) {
            return o_pg;
        } else if (month < o_month) {
            makeTitle(month);
            if (prev == NULL) {
                return insert_page_head(m_dt, &str);
            } else {
                return insert_page(prev, &str);
            }
        }
    }
    makeTitle(month);
    return add_page(m_dt, &str);
}
