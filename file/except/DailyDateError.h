#ifndef _DAILY_DATE_ERROR_H_
#define _DAILY_DATE_ERROR_H_

#include <exception>
#include <string>

class DailyDateError : public std::exception
{
public:
    DailyDateError(const char *title) : m_title(title)
    {
    }

    virtual ~DailyDateError() _NOEXCEPT
    {
    }

    virtual const char *what() const _NOEXCEPT
    {
        return (std::string("Wrong date in daily file: ") + m_title + ".").c_str();
    }

    const std::string &title() const
    {
        return m_title;
    }

protected:
    std::string m_title;
};

#endif
