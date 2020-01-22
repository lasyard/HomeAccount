#ifndef _FILE_TYPE_H_
#define _FILE_TYPE_H_

class FileType
{
public:
    enum Type { DAILY, CASH, CAT, WHOLE, INVALID };

    FileType()
    {
    }

    virtual ~FileType()
    {
    }

    virtual const Type type() const = 0;
};

class HaFileStub : public FileType
{
public:
    virtual const Type type() const
    {
        return WHOLE;
    }
};

#endif
