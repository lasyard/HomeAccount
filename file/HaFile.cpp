#include <sstream>

#include <wx/datetime.h>
#include <wx/dir.h>
#include <wx/filename.h>

#include "AnnuallyFileRW.h"
#include "CashFileRW.h"
#include "CatFileRW.h"
#include "DailyFileRW.h"
#include "HaFile.h"
#include "MonthlyFileRW.h"
#include "SubFiles.h"

#include "../c/cal.h"

const char *const HaFile::IV = "Home Account IV";

const char *const HaFile::CatFileName = "cat";
const char *const HaFile::CashFileName = "cash";
const char *const HaFile::AnnuallyFileName = "annually";

const char *const HaFile::DIR = "HA";
const char *const HaFile::EXT = "ha";

wxString HaFile::newFileName()
{
    wxDateTime date = wxDateTime::Now();
    return date.Format("%Y%m%d_%H%M%S") + "." + EXT;
}

wxString HaFile::getPath(const wxString &fileName)
{
    return wxFileName(DIR, fileName).GetFullPath();
}

HaFile *HaFile::getNewestFile()
{
    wxDir dir(DIR);
    if (!dir.IsOpened()) {
        if (!wxDir::Make(DIR)) {
            return nullptr;
        }
        dir.Open(DIR);
    }
    wxArrayString fileNames;
    wxString fileName;
    if (dir.GetFirst(&fileName, wxString("*.") + EXT)) {
        fileNames.Add(fileName);
        while (dir.GetNext(&fileName)) {
            fileNames.Add(fileName);
        }
    }
    if (fileNames.IsEmpty()) {
        HaFile *file = new HaFile(getPath(newFileName()).c_str(), false);
        file->makeNewFile();
        return file;
    }
    fileNames.Sort(true);
    return new HaFile(getPath(fileNames[0]).c_str());
}

void HaFile::save(FileRW *file)
{
    rawSave(file);
    file->afterSave();
    saveCatalog();
    flush();
}

void HaFile::calTotal(struct cat_root *cat, int sYear, int sMonth, int eYear, int eMonth)
{
    int year, month;
    for (year = sYear, month = sMonth; year < eYear || (year == eYear && month <= eMonth);) {
        cal_data_total(SubDailyFile(this, year, month)()->getData(), cat);
        month++;
        if (month > 12) {
            year++;
            month = 1;
        }
    }
}

long HaFile::calBalanceBefore(int year, int month)
{
    if (month == 1) {
        return SubAnnuallyFile(this)()->calBalanceToYear(year - 1);
    } else {
        SubMonthlyFile t(this, year);
        t()->setInitial(calBalanceBefore(year, 1));
        return t()->calBalanceToMonth(month - 1);
    }
}

long HaFile::calFinalBalance()
{
    return SubAnnuallyFile(this)()->calFinalBalance();
}

void HaFile::setInitial(long money)
{
    SubAnnuallyFile(this, true)()->setInitial(money);
}

long HaFile::getInitial()
{
    return SubAnnuallyFile(this)()->getInitial();
}

int HaFile::minYear()
{
    return SubAnnuallyFile(this)()->minYear();
}

int HaFile::maxYear()
{
    return SubAnnuallyFile(this)()->maxYear();
}

int HaFile::minMonth(int year)
{
    return SubMonthlyFile(this, year)()->minMonth();
}

int HaFile::maxMonth(int year)
{
    return SubMonthlyFile(this, year)()->maxMonth();
}

void HaFile::updateMonthly(int year, int month, int income, int outlay)
{
    SubMonthlyFile(this, year, true)()->setIncomeOutlay(month, income, outlay);
}

void HaFile::updateAnnually(int year, int income, int outlay)
{
    SubAnnuallyFile(this, true)()->setIncomeOutlay(year, income, outlay);
}

void HaFile::loadFileRW(FileRW *fileRW)
{
    std::stringstream result;
    loadFile(fileRW->fileName(), result);
    fileRW->readStream(result);
}

void HaFile::rawSave(const FileRW *file)
{
    std::stringstream content;
    file->writeStream(content);
    saveFile(file->fileName(), content);
}

HaFile *HaFile::newCopy()
{
    HaFile *haFile = new HaFile(getPath(newFileName()).c_str(), false);
    haFile->copyFrom(this);
    return haFile;
}

CatFileRW *HaFile::getCatFile()
{
    return getFile<CatFileRW>(CatFileName);
}

CashFileRW *HaFile::getCashFile()
{
    return getFile<CashFileRW>(CashFileName);
}

AnnuallyFileRW *HaFile::getAnnuallyFile()
{
    return getFile<AnnuallyFileRW>(AnnuallyFileName);
}

MonthlyFileRW *HaFile::getMonthlyFile(int year)
{
    return getFile<MonthlyFileRW>(MonthlyFileName(year)());
}

DailyFileRW *HaFile::getDailyFile(int year, int month)
{
    return getFile<DailyFileRW>(DailyFileName(year, month)());
}
