#ifndef _DAILY_DATE_ERROR_H_
#define _DAILY_DATE_ERROR_H_

#include <exception>
#include <string>

class DailyDateError : public std::exception
{
public:
    DailyDateError(const char *title) : m_title(title)
    {
        m_msg = std::string("Wrong date in daily file: ") + m_title + ".";
    }

    virtual ~DailyDateError() wxNOEXCEPT
    {
    }

    virtual const char *what() const wxNOEXCEPT
    {
        return m_msg.c_str();
    }

    const std::string &title() const
    {
        return m_title;
    }

protected:
    std::string m_title;
    std::string m_msg;
};

#endif
