#ifndef _INTERNAL_ERROR_H_
#define _INTERNAL_ERROR_H_

#include <cstdarg>
#include <exception>
#include <string>

class InternalError : public std::exception
{
public:
    static InternalError *create(const char *format, ...)
    {
        InternalError *m = new InternalError();
        va_list args;
        va_start(args, format);
        vsprintf(m->m_msg, format, args);
        va_end(args);
        return m;
    }

    virtual const char *what() const _NOEXCEPT
    {
        return m_msg;
    }

protected:
    char m_msg[1024];

    InternalError()
    {
    }

    ~InternalError()
    {
    }
};

#endif
