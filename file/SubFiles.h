#ifndef _SUB_FILES_H_
#define _SUB_FILES_H_

#include "AnnuallyFileRW.h"
#include "CashFileRW.h"
#include "CatFileRW.h"
#include "DailyFileRW.h"
#include "HaFile.h"
#include "MonthlyFileRW.h"

template <typename FileRWType>
class SubFile
{
public:
    SubFile(HaFile *ha, const char *name, bool save = false) : m_ha(ha), m_save(save)
    {
        m_file = ha->getFile<FileRWType>(name);
    }

    virtual ~SubFile()
    {
        if (m_save) m_ha->save(m_file);
        delete m_file;
    }

    FileRWType *operator()() const
    {
        return m_file;
    }

protected:
    HaFile *m_ha;
    FileRWType *m_file;
    bool m_save;
};

class SubAnnuallyFile : public SubFile<AnnuallyFileRW>
{
public:
    SubAnnuallyFile(HaFile *ha, bool save = false) : SubFile(ha, HaFile::ANNUALLY_FILE_NAME, save)
    {
    }
};

class SubMonthlyFile : public SubFile<MonthlyFileRW>
{
public:
    SubMonthlyFile(HaFile *ha, int year, bool save = false) : SubFile(ha, MonthlyFileName(year)(), save)
    {
    }
};

class SubDailyFile : public SubFile<DailyFileRW>
{
public:
    SubDailyFile(HaFile *ha, int year, int month, bool save = false) : SubFile(ha, DailyFileName(year, month)(), save)
    {
    }
};

class SubCashFile : public SubFile<CashFileRW>
{
public:
    SubCashFile(HaFile *ha, bool save = false) : SubFile(ha, HaFile::CASH_FILE_NAME, save)
    {
    }
};

class SubCatFile : public SubFile<CatFileRW>
{
public:
    SubCatFile(HaFile *ha, bool save = false) : SubFile(ha, HaFile::CAT_FILE_NAME, save)
    {
    }
};

#endif
