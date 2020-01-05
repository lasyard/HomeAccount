#ifndef _DAILY_FILE_EMPTY_H_
#define _DAILY_FILE_EMPTY_H_

#include <exception>

class DailyFileEmpty : public std::exception
{
    virtual const char *what() const _NOEXCEPT
    {
        return "Daily File is empty.";
    }
};

#endif
