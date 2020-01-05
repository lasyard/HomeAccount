#ifndef _CASH_FILE_R_W_H_
#define _CASH_FILE_R_W_H_

#include "DataFileRW.h"

class CashFileRW : public DataFileRW
{
public:
    CashFileRW(const char *fileName, HaFile *file = NULL) : DataFileRW(fileName, file)
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

protected:
};

#endif
