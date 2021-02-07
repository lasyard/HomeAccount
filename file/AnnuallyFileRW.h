#ifndef _ANNUALLY_FILE_R_W_H_
#define _ANNUALLY_FILE_R_W_H_

#include "DataFileRW.h"

class AnnuallyFileRW : public DataFileRW
{
public:
    AnnuallyFileRW(const char *name, HaFile *file = nullptr) : DataFileRW(name, file)
    {
        m_title[YEAR_LEN] = '\0';
    }

    virtual void afterSave() const
    {
    }

    int minYear() const
    {
        if (is_empty_data(m_dt)) return 2000;
        const struct string &title = get_page(m_dt->pages.first)->title;
        return str_to_int(title.str, YEAR_LEN);
    }

    int maxYear() const
    {
        if (is_empty_data(m_dt)) return 2000;
        const struct string &title = get_page(m_dt->pages.last)->title;
        return str_to_int(title.str, YEAR_LEN);
    }

    money_t calBalanceToYear(int year) const;

    money_t calFinalBalance() const
    {
        return cal_data_balance(m_dt, m_dt->initial);
    }

    void setIncomeOutlay(int year, money_t income, money_t outlay)
    {
        struct page *pg = getPage(year);
        clear_page(pg);
        add_simple_item(pg, -income);
        add_simple_item(pg, outlay);
    }

protected:
    mutable char m_title[YEAR_LEN + 1];

    const char *makeTitle(int year) const
    {
        int_to_str_len(m_title, year, YEAR_LEN);
        return m_title;
    }

    virtual void writeInitial(std::ostream &os, money_t initial) const
    {
        char buf[MONEY_LEN];
        money_to_str(buf, -initial);
        os << buf << std::endl;
    }

    struct page *getPage(int year);
};

#endif
