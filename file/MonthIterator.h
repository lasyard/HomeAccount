#ifndef _MONTH_ITERATOR_H_
#define _MONTH_ITERATOR_H_

class MonthIterator
{
public:
    MonthIterator(int year, int month) : m_year(year), m_month(month)
    {
    }

    MonthIterator &operator++()
    {
        m_month++;
        if (m_month > 12) {
            m_year++;
            m_month = 1;
        }
        return *this;
    }

    MonthIterator &operator++(int)
    {
        ++(*this);
        return *this;
    }

    bool operator==(const MonthIterator &obj)
    {
        return m_year == obj.m_year && m_month == obj.m_month;
    }

    bool operator!=(const MonthIterator &obj)
    {
        return !(*this == obj);
    }

    bool operator<(const MonthIterator &obj)
    {
        return m_year < obj.m_year || (m_year == obj.m_year && m_month < obj.m_month);
    }

    bool operator<=(const MonthIterator &obj)
    {
        return m_year < obj.m_year || (m_year == obj.m_year && m_month <= obj.m_month);
    }

    bool operator>(const MonthIterator &obj)
    {
        return m_year > obj.m_year || (m_year == obj.m_year && m_month > obj.m_month);
    }
    bool operator>=(const MonthIterator &obj)
    {
        return m_year > obj.m_year || (m_year == obj.m_year && m_month >= obj.m_month);
    }

    int year()
    {
        return m_year;
    }

    int month()
    {
        return m_month;
    }

protected:
    int m_year;
    int m_month;
};

#endif
