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

    DailyFileRW(const char *name, HaFile *file = nullptr) : DataFileRW(name, file)
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
        money_t i, o;
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

    virtual bool import(const char *header, std::istream &is);

    virtual const HaFile::FileType type()
    {
        return HaFile::DAILY;
    }

protected:
    int m_year;
    int m_month;
    mutable char m_title[DATE_LEN + 1];

    const char *makeTitle(int day) const
    {
        ymd_to_str(m_title, m_year, m_month, day, '.');
        return m_title;
    }

    virtual const char *typeHeader() const
    {
        return "HADL";
    }

    void addEmptyPage();
};

#endif
