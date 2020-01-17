#ifndef _HA_FILE_H_
#define _HA_FILE_H_

#include <wx/arrstr.h>
#include <wx/datetime.h>
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

    static wxArrayString getFileList(const wxString &dirName);
    static HaFile *getNewestFile(const wxString &dirName);

    HaFile *newCopy(const wxString &dirName);
    void save(FileRW *file);
    void calTotal(struct cat_root *cat, int sYear, int sMonth, int eYear, int eMonth);
    money_t calBalanceBefore(int year, int month);
    money_t calFinalBalance();
    void setInitial(money_t money);
    money_t getInitial();
    int minYear();
    int maxYear();
    int minMonth(int year);
    int maxMonth(int year);
    void updateMonthly(int year, int month, money_t income, money_t outlay);
    void updateAnnually(int year, money_t income, money_t outlay);

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

    static wxString newFileName()
    {
        wxDateTime date = wxDateTime::Now();
        return date.Format("%Y%m%d_%H%M%S") + "_" + VER_STR + "." + EXT;
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
