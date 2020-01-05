#ifndef _FILE_NAMES_H_
#define _FILE_NAMES_H_

#include "../c/utils.h"

class MonthlyFileName
{
public:
    MonthlyFileName(int year)
    {
        int_to_str_len(m_buf, year, YEAR_LEN);
        m_buf[YEAR_LEN] = '\0';
    }

    const char *operator()() const
    {
        return m_buf;
    }

protected:
    char m_buf[YEAR_LEN + 1];
};

class DailyFileName
{
public:
    DailyFileName(int year, int month)
    {
        ym_to_str(m_buf, year, month, '/');
        m_buf[YEAR_LEN + 1 + MONTH_LEN] = '\0';
    }

    const char *operator()() const
    {
        return m_buf;
    }

protected:
    char m_buf[YEAR_LEN + 1 + MONTH_LEN + 1];
};

#endif
