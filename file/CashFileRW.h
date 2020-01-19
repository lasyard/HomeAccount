#ifndef _CASH_FILE_R_W_H_
#define _CASH_FILE_R_W_H_

#include "DataFileRW.h"

class CashFileRW : public DataFileRW
{
public:
    CashFileRW() : CashFileRW(HaFile::CASH_FILE_NAME)
    {
    }

    CashFileRW(const char *fileName, HaFile *file = nullptr) : DataFileRW(fileName, file)
    {
    }

    virtual void afterLoad()
    {
        DataFileRW::afterLoad();
        setInitial(m_file->calFinalBalance());
    }

    virtual void afterSave() const
    {
    }

    virtual const HaFile::FileType type()
    {
        return HaFile::CASH;
    }

protected:
    virtual const char *typeHeader() const
    {
        return "HACS";
    }
};

#endif
