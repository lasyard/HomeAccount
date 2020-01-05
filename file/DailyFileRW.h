#ifndef _DAILY_FILE_R_W_H_
#define _DAILY_FILE_R_W_H_

#include <string>

#include "DataFileRW.h"

class DailyFileRW : public DataFileRW
{
public:
    DailyFileRW() : DataFileRW("")
    {
    }

    DailyFileRW(const char *name, HaFile *file = NULL) : DataFileRW(name, file)
    {
        str_to_ym(name, &m_year, &m_month, '/');
    }

    virtual void afterLoad()
    {
        addEmptyPage();
        DataFileRW::afterLoad();
        setInitial(m_file->calBalanceBefore(m_year, m_month));
    }

    virtual void afterSave() const
    {
        long i, o;
        cal_data_income_outlay(m_dt, &i, &o);
        m_file->updateMonthly(m_year, m_month, i, o);
    }

    int year() const
    {
        return m_year;
    }

    int month() const
    {
        return m_month;
    }

    void setDateFromData();

protected:
    int m_year;
    int m_month;
    mutable char m_title[DATE_LEN + 1];

    virtual void writeInitial(std::ostream &os, long initial) const
    {
    }

    const char *makeTitle(int day) const
    {
        ymd_to_str(m_title, m_year, m_month, day, '.');
        return m_title;
    }

    void addEmptyPage();
};

#endif
