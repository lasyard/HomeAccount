#ifndef _MONTHLY_FILE_R_W_H_
#define _MONTHLY_FILE_R_W_H_

#include "DataFileRW.h"

class MonthlyFileRW : public DataFileRW
{
public:
    MonthlyFileRW(const char *name, HaFile *file = nullptr) : DataFileRW(name, file)
    {
        m_year = str_to_int(name, YEAR_LEN);
        m_title[YEAR_LEN + 1 + MONTH_LEN] = '\0';
    }

    virtual void afterLoad()
    {
        DataFileRW::afterLoad();
        setInitial(m_file->calBalanceBefore(m_year, 1));
    }

    virtual void afterSave() const
    {
        long i, o;
        cal_data_income_outlay(m_dt, &i, &o);
        m_file->updateAnnually(m_year, i, o);
    }

    int year() const
    {
        return m_year;
    }

    int minMonth() const
    {
        if (is_empty_data(m_dt)) return 1;
        const struct string &title = get_page(m_dt->pages.first)->title;
        return str_to_int(title.str + YEAR_LEN + 1, MONTH_LEN);
    }

    int maxMonth() const
    {
        if (is_empty_data(m_dt)) return 12;
        const struct string &title = get_page(m_dt->pages.last)->title;
        return str_to_int(title.str + YEAR_LEN + 1, MONTH_LEN);
    }

    long calBalanceToMonth(int month) const;

    void setIncomeOutlay(int month, int income, int outlay)
    {
        struct page *pg = getPage(month);
        clear_page(pg);
        add_simple_item(pg, -income);
        add_simple_item(pg, outlay);
    }

protected:
    int m_year;
    mutable char m_title[YEAR_LEN + 1 + MONTH_LEN + 1];

    virtual void writeInitial(std::ostream &os, long initial) const
    {
    }

    const char *makeTitle(int month) const
    {
        ym_to_str(m_title, m_year, month, '.');
        return m_title;
    }

    struct page *getPage(int month);
};

#endif
