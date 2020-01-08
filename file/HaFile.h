#ifndef _HA_FILE_H_
#define _HA_FILE_H_

#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/string.h>

#include "CryptoFile.h"
#include "FileNames.h"

class FileRW;
class CatFileRW;
class CashFileRW;
class AnnuallyFileRW;
class MonthlyFileRW;
class DailyFileRW;

class HaFile : public CryptoFile
{
public:
    static const char *const CatFileName;
    static const char *const CashFileName;
    static const char *const AnnuallyFileName;
    static const char *const DIR;
    static const char *const EXT;

    HaFile(const char *fileName, bool isOld = true) : CryptoFile(fileName, IV), m_isOld(isOld)
    {
    }

    virtual ~HaFile()
    {
    }

    static HaFile *getNewestFile();

    void save(FileRW *file);
    HaFile *newCopy();
    void calTotal(struct cat_root *cat, int sYear, int sMonth, int eYear, int eMonth);
    long calBalanceBefore(int year, int month);
    long calFinalBalance();
    void setInitial(long money);
    long getInitial();
    int minYear();
    int maxYear();
    int minMonth(int year);
    int maxMonth(int year);
    void updateMonthly(int year, int month, int income, int outlay);
    void updateAnnually(int year, int income, int outlay);

    CatFileRW *getCatFile();
    CashFileRW *getCashFile();
    AnnuallyFileRW *getAnnuallyFile();
    MonthlyFileRW *getMonthlyFile(int year);
    DailyFileRW *getDailyFile(int year, int month);

    bool isOld() const
    {
        return m_isOld;
    }

protected:
    static const char *const IV;
    static wxString m_dir;

    static wxString getPath(const wxString &fileName)
    {
        return wxFileName(m_dir, fileName).GetFullPath();
    }

    static wxString newFileName()
    {
        wxDateTime date = wxDateTime::Now();
        return date.Format("%Y%m%d_%H%M%S") + "_home_account." + EXT;
    }

    static void setDir(wxString dir)
    {
        m_dir = dir;
    }

    bool m_isOld;

    template <typename FileRWType>
    FileRWType *getFile(const char *name)
    {
        tryLoadFile();
        FileRWType *file = new FileRWType(name, this);
        try {
            loadFileRW(file);
        } catch (std::exception &e) {
            delete file;
            throw e;
        }
        return file;
    }

    void loadFileRW(FileRW *fileRW);
    void rawSave(const FileRW *file);

    template <typename>
    friend class SubFile;
};

#endif
