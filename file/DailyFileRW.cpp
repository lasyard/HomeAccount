#include "DailyFileRW.h"
#include "except/DailyDateError.h"
#include "except/DailyFileEmpty.h"

void DailyFileRW::setDateFromData()
{
    if (is_empty_data(m_dt)) throw DailyFileEmpty();
    struct ulist_item *p = m_dt->pages.first;
    const struct string *title = &(get_page(p)->title);
    int day;
    if (!str_to_ymd(title->str, &m_year, &m_month, &day, '.')) throw DailyDateError(title->str);
    int year, month;
    for (p = p->next; p != NULL; p = p->next) {
        const struct string *title = &(get_page(p)->title);
        if (!str_to_ymd(title->str, &year, &month, &day, '.') || year != m_year || month != m_month) {
            throw DailyDateError(title->str);
        }
    }
}

void DailyFileRW::addEmptyPage()
{
    struct string str;
    string_mock(&str, m_title, DATE_LEN);
    int last_day = last_day_of_month(m_year, m_month);
    int d;
    if (is_empty_data(m_dt)) {
        for (d = 1; d <= last_day; d++) {
            makeTitle(d);
            add_page(m_dt, &str);
        }
        return;
    }
    struct page *prev = NULL;
    struct ulist_item *p;
    for (d = 1, p = m_dt->pages.first; p != NULL; prev = get_page(p), p = p->next) {
        struct page *pg = get_page(p);
        int year, month, day;
        if (str_to_ymd(pg->title.str, &year, &month, &day, '.')) {
            while (d < day) {
                makeTitle(d);
                if (prev == NULL) {
                    prev = insert_page_head(m_dt, &str);
                } else {
                    prev = insert_page(prev, &str);
                }
                d++;
            }
            d++;
        }
    }
    while (d <= last_day) {
        makeTitle(d);
        add_page(m_dt, &str);
        d++;
    }
}
