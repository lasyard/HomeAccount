#ifndef _FILE_ERROR_H_
#define _FILE_ERROR_H_

#include <exception>
#include <string>

class FileError : public std::exception
{
public:
    FileError()
    {
    }

    virtual ~FileError() _NOEXCEPT
    {
    }

    void setFileName(const std::string &fileName)
    {
        m_fileName = fileName;
    }

    const std::string &fileName() const
    {
        return m_fileName;
    }

    void setLineNo(int lineNo)
    {
        m_lineNo = lineNo;
    }

    int lineNo() const
    {
        return m_lineNo;
    }

protected:
    std::string m_fileName;
    int m_lineNo;
};

#endif
