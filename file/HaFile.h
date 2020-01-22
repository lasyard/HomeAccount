#ifndef _HA_FILE_H_
#define _HA_FILE_H_

#include <wx/arrstr.h>
#include <wx/datetime.h>
#include <wx/string.h>

#include "CryptoFile.h"
#include "FileNames.h"

class FileType;
class FileRW;
class CatFileRW;
class CashFileRW;
class AnnuallyFileRW;
class MonthlyFileRW;
class DailyFileRW;

class HaFile : public CryptoFile
{
public:
    static const char *const CAT_FILE_NAME;
    static const char *const CASH_FILE_NAME;
    static const char *const ANNUALLY_FILE_NAME;
    static const char *const EXT;
    static const char *const HEADER;

    HaFile(const char *fileName, bool isOld = true) : CryptoFile(fileName, IV), m_isOld(isOld)
    {
    }

    virtual ~HaFile()
    {
    }

    static wxString idString()
    {
        wxDateTime date = wxDateTime::Now();
        return date.Format("%Y%m%d_%H%M%S") + "_" + VER_STR;
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

    FileType *import(const std::string &path);
    void exportAll(const std::string &path);

protected:
    static const char *const IV;

    static wxString newFileName()
    {
        return idString() + "." + EXT;
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
    FileRW *importOne(const char *header, std::istream &is);

    template <typename>
    friend class SubFile;
};

#endif
