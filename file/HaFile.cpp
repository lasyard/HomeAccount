#include <sstream>

#include <wx/dir.h>
#include <wx/stdpaths.h>

#include "AnnuallyFileRW.h"
#include "CashFileRW.h"
#include "CatFileRW.h"
#include "DailyFileRW.h"
#include "HaFile.h"
#include "MonthIterator.h"
#include "MonthlyFileRW.h"
#include "SubFiles.h"

#include "../c/cal.h"

const char *const HaFile::IV = "Home Account IV";
const char *const HaFile::CAT_FILE_NAME = "cat";
const char *const HaFile::CASH_FILE_NAME = "cash";
const char *const HaFile::ANNUALLY_FILE_NAME = "annually";
const char *const HaFile::EXT = "ha";
const char *const HaFile::HEADER = "HAAD";

wxArrayString HaFile::getFileList(const wxString &dirName)
{
    wxArrayString fileNames;
    wxDir dir(dirName);
    if (!dir.IsOpened()) {
        if (!wxDir::Make(dirName)) {
            return fileNames;
        }
        dir.Open(dirName);
    }
    wxString fileName;
    if (dir.GetFirst(&fileName, wxString("*.") + EXT)) {
        fileNames.Add(fileName);
        while (dir.GetNext(&fileName)) {
            fileNames.Add(fileName);
        }
    }
    return fileNames;
}

HaFile *HaFile::getNewestFile(const wxString &dirName)
{
    wxArrayString fileNames = getFileList(dirName);
    wxString fileName;
    if (fileNames.IsEmpty()) {
        HaFile *file = new HaFile(wxFileName(dirName, newFileName()).GetFullPath().c_str(), false);
        file->clearFile();
        return file;
    }
    fileNames.Sort(true);
    return new HaFile(wxFileName(dirName, fileNames[0]).GetFullPath().c_str());
}

HaFile *HaFile::newCopy(const wxString &dirName)
{
    HaFile *haFile = new HaFile(wxFileName(dirName, newFileName()).GetFullPath().c_str(), false);
    haFile->copyFrom(this);
    return haFile;
}

void HaFile::save(FileRW *file)
{
    rawSave(file);
    file->afterSave();
    saveCatalog();
    flush();
}

FileRW *HaFile::import(const std::string &path)
{
    std::ifstream ifs(path);
    char line[FileRW::LINE_LEN];
    ifs.getline(line, FileRW::LINE_LEN);
    if (strncmp(line, HEADER, FileRW::TYPE_HEADER_LEN) == 0) {
        clearFile();
        FileRW *file;
        while (true) {
            ifs.getline(line, FileRW::LINE_LEN);
            file = importOne(line, ifs);
            if (file == nullptr) return nullptr;
            if (ifs.eof()) break;
            delete file;
        }
        return file;
    } else {
        return importOne(line, ifs);
    }
}

FileRW *HaFile::importOne(const char *header, std::istream &is)
{
    FileRW *file;
    if ((file = new CatFileRW())->import(header, is) || (file = new CashFileRW())->import(header, is) ||
        (file = new DailyFileRW())->import(header, is)) {
        file->setHaFile(this);
        file->save();
        return file;
    }
    return nullptr;
}

void HaFile::exportAll(const std::string &path)
{
    std::ofstream ofs(path);
    ofs << HEADER << ": exported by " << idString() << std::endl;
    for (MonthIterator i(minYear(), minMonth(minYear())); i < MonthIterator(maxYear(), maxMonth(maxYear())); ++i) {
        if (exist(DailyFileName(i.year(), i.month())())) {
            SubDailyFile t(this, i.year(), i.month());
            t()->saveAs(ofs);
            t()->writeSeparator(ofs);
        }
    }
    SubCashFile t(this);
    t()->saveAs(ofs);
    t()->writeSeparator(ofs);
    SubCatFile(this)()->saveAs(ofs);
}

void HaFile::calTotal(struct cat_root *cat, int sYear, int sMonth, int eYear, int eMonth)
{
    for (MonthIterator i(sYear, sMonth); i < MonthIterator(eYear, eMonth); ++i) {
        cal_data_total(SubDailyFile(this, i.year(), i.month())()->getData(), cat);
    }
}

money_t HaFile::calBalanceBefore(int year, int month)
{
    if (month == 1) {
        return SubAnnuallyFile(this)()->calBalanceToYear(year - 1);
    } else {
        SubMonthlyFile t(this, year);
        t()->setInitial(calBalanceBefore(year, 1));
        return t()->calBalanceToMonth(month - 1);
    }
}

money_t HaFile::calFinalBalance()
{
    return SubAnnuallyFile(this)()->calFinalBalance();
}

void HaFile::setInitial(money_t money)
{
    SubAnnuallyFile(this, true)()->setInitial(money);
}

money_t HaFile::getInitial()
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

void HaFile::updateMonthly(int year, int month, money_t income, money_t outlay)
{
    SubMonthlyFile(this, year, true)()->setIncomeOutlay(month, income, outlay);
}

void HaFile::updateAnnually(int year, money_t income, money_t outlay)
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

CatFileRW *HaFile::getCatFile()
{
    return getFile<CatFileRW>(CAT_FILE_NAME);
}

CashFileRW *HaFile::getCashFile()
{
    return getFile<CashFileRW>(CASH_FILE_NAME);
}

AnnuallyFileRW *HaFile::getAnnuallyFile()
{
    return getFile<AnnuallyFileRW>(ANNUALLY_FILE_NAME);
}

MonthlyFileRW *HaFile::getMonthlyFile(int year)
{
    return getFile<MonthlyFileRW>(MonthlyFileName(year)());
}

DailyFileRW *HaFile::getDailyFile(int year, int month)
{
    return getFile<DailyFileRW>(DailyFileName(year, month)());
}
