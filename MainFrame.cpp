#include <fstream>

#include <wx/app.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/choicdlg.h>
#include <wx/choice.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/textdlg.h>
#include <wx/xrc/xmlres.h>

#include "ConfigDialog.h"
#include "MainFrame.h"
#include "StatDialog.h"
#include "StatHtml.h"
#include "file/AnnuallyFileRW.h"
#include "file/CashFileRW.h"
#include "file/DailyFileRW.h"
#include "file/HaFile.h"
#include "file/MonthlyFileRW.h"
#include "file/SubFiles.h"
#include "file/except/BadPassword.h"
#include "file/except/CatDupItem.h"
#include "file/except/CatFileError.h"
#include "file/except/DailyDateError.h"
#include "file/except/DailyFileEmpty.h"
#include "file/except/DataFileError.h"
#include "file/except/FileCorrupt.h"

wxIMPLEMENT_DYNAMIC_CLASS(MainFrame, wxFrame);

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_DATE_CHANGED(XRCID("date"), MainFrame::onDateChanged)
EVT_BUTTON(XRCID("statistics"), MainFrame::onStatButton)
EVT_BUTTON(XRCID("export"), MainFrame::onExportButton)
EVT_BUTTON(XRCID("import"), MainFrame::onImportButton)
EVT_BUTTON(XRCID("config"), MainFrame::onConfigButton)
EVT_NOTEBOOK_PAGE_CHANGING(XRCID("book"), MainFrame::onPageChanging)
EVT_CLOSE(MainFrame::onClose)
END_EVENT_TABLE()

MainFrame::MainFrame() : m_dir(), m_file(nullptr)
{
    wxXmlResource::Get()->LoadObject(this, nullptr, "main", "wxFrame");
    m_book = XRCCTRL(*this, "book", wxNotebook);
    m_grid = XRCCTRL(*this, "daily", DailyGrid);
    m_cashGrid = XRCCTRL(*this, "cash", DataGrid);
    m_date = XRCCTRL(*this, "date", wxDatePickerCtrl);
    m_html = XRCCTRL(*this, "html", StatHtml);
}

void MainFrame::initView(const wxString &dir)
{
    m_dir = dir;
    safeDeleteFile();
    m_file = HaFile::getNewestFile(m_dir);
    m_grid->initGrid();
    m_cashGrid->initGrid();
    int count = 0;
    while (true) {
        try {
            loadCatFile();
            break;
        } catch (FileCorrupt &) {
            wxMessageBox(_("File corrupted"), _("App name"), wxOK | wxICON_ERROR);
            wxExit();
        } catch (BadPassword &) {
            if (count == 3) break;
            count++;
            wxPasswordEntryDialog *dlg = new wxPasswordEntryDialog(this, _("Input password"), _("App name"));
            if (dlg->ShowModal() == wxID_OK) {
                m_file->setKey(dlg->GetValue().ToStdString());
            } else {
                wxExit();
            }
            dlg->Destroy();
        }
    }
    if (count >= 3) wxExit();
    m_year = wxDateTime::Today().GetYear();
    m_month = wxDateTime::Today().GetMonth() + 1;
    m_day = wxDateTime::Today().GetDay();
    loadDailyFile();
    loadCashFile();
    showDaily();
}

void MainFrame::onDateChanged(wxDateEvent &event)
{
    int year, month, day;
    year = event.GetDate().GetYear();
    month = event.GetDate().GetMonth() + 1;
    day = event.GetDate().GetDay();
    if (month != m_month || year != m_year) {
        m_year = year;
        m_month = month;
        m_day = day;
        dailyQuerySave();
        loadDailyFile();
    } else {
        m_day = day;
    }
    showDaily();
}

void MainFrame::onPageChanging(wxBookCtrlEvent &event)
{
    int sel = event.GetOldSelection();
    switch (sel) {
        case DAILY_PAGE:
            m_grid->SaveEditControlValue();
            dailyQuerySave();
            loadCashFile();
            m_cashGrid->updateData();
            break;
        case CASH_PAGE:
            m_cashGrid->SaveEditControlValue();
            cashQuerySave();
            break;
        default:
            break;
    }
}

void MainFrame::onStatButton(wxCommandEvent &event)
{
    StatDialog *dlg = new StatDialog();
    dlg->setFile(m_file);
    if (dlg->ShowModal() == wxID_OK) {
        dailyQuerySave();
        int sel = dlg->getSelection();
        if (sel == 0) {
            catQuerySave();
            struct cat_root *cat = m_grid->catFileRW()->getCatRoot();
            if (mtree_is_leaf(&cat->root)) {
                wxMessageBox(_("Category config is empty"), _("App name"), wxOK | wxICON_ERROR);
                dlg->Destroy();
                return;
            }
            int sYear, sMonth, eYear, eMonth;
            dlg->getData(sYear, sMonth, eYear, eMonth);
            clear_total(cat);
            m_file->calTotal(cat, sYear, sMonth, eYear, eMonth);
            sum_total(mtree_first_child(&cat->root));
            get_cat_node(mtree_first_child(&cat->root))->total += cat->no_cat_in_sum;
            sum_total(mtree_last_child(&cat->root));
            get_cat_node(mtree_last_child(&cat->root))->total += cat->no_cat_out_sum;
            m_html->showTotal(cat, sYear, sMonth, eYear, eMonth);
        } else if (sel == 1) {
            SubAnnuallyFile t(m_file);
            wxString str;
            str.Printf(_("Annually statistics"), t()->minYear(), t()->maxYear());
            m_html->showIO(t(), str);
        } else if (sel == 2) {
            wxString yearStr = dlg->getSelectedYear();
            int year = str_to_int(yearStr, yearStr.Len());
            SubMonthlyFile t(m_file, year);
            wxString str;
            str.Printf(_("Monthly statistics"), year);
            m_html->showIO(t(), str);
        }
        showStatistics();
    }
    dlg->Destroy();
}

void MainFrame::onExportButton(wxCommandEvent &event)
{
    wxArrayString choices;
    choices.Add(_("Current datum"));
    choices.Add(_("Categories config"));
    wxSingleChoiceDialog *dlg = new wxSingleChoiceDialog(this, _("Choose one to export"), _("App name"), choices);
    if (dlg->ShowModal() == wxID_OK) {
        int index = dlg->GetSelection();
        wxString wildCardString;
        if (index == 0 && m_book->GetSelection() == STATISTICS_PAGE) {
            wildCardString = "HTML files (*.html)|*.html";
        } else {
            wildCardString = "Text files (*.txt)|*.txt";
        }
        wxFileDialog *fileDlg = new wxFileDialog(this,
                                                 wxFileSelectorPromptStr,
                                                 wxEmptyString,
                                                 wxEmptyString,
                                                 wildCardString,
                                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (fileDlg->ShowModal() == wxID_OK) {
            std::string path = fileDlg->GetPath().ToStdString();
            if (index == 0) {
                if (m_book->GetSelection() == DAILY_PAGE) {
                    dailyQuerySave();
                    m_grid->dataFileRW()->saveAs(path);
                } else if (m_book->GetSelection() == CASH_PAGE) {
                    cashQuerySave();
                    m_cashGrid->dataFileRW()->saveAs(path);
                } else if (m_book->GetSelection() == STATISTICS_PAGE) {
                    m_html->saveAs(path);
                }
            } else if (index == 1) {
                catQuerySave();
                m_grid->catFileRW()->saveAs(path);
            }
        }
        fileDlg->Destroy();
    }
    dlg->Destroy();
}

void MainFrame::onImportButton(wxCommandEvent &event)
{
    wxFileDialog *fileDlg = new wxFileDialog(
        this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN);
    if (fileDlg->ShowModal() == wxID_OK) {
        std::string path = fileDlg->GetPath().ToStdString();
        wxArrayString choices;
        choices.Add(_("Monthly datum"));
        choices.Add(_("Cash datum"));
        choices.Add(_("Categories config"));
        wxSingleChoiceDialog *dlg =
            new wxSingleChoiceDialog(this, _("Choose one to import into"), _("App name"), choices);
        if (dlg->ShowModal() == wxID_OK) {
            int index = dlg->GetSelection();
            if (index == 0) {
                DailyFileRW file;
                try {
                    file.import(path);
                    file.setDateFromData();
                } catch (DataFileError &e) {
                    showDataFileError(e);
                    return;
                } catch (DailyFileEmpty &e) {
                    wxMessageBox(_("File is empty"), _("App name"), wxOK | wxICON_ERROR);
                    return;
                } catch (DailyDateError &e) {
                    wxString str;
                    str.Printf(_("Invalid date \"%1$s\""), e.title());
                    wxMessageBox(str, _("App name"), wxOK | wxICON_ERROR);
                    return;
                }
                file.setHaFile(m_file);
                file.save();
                m_year = file.year();
                m_month = file.month();
                m_day = 1;
                wxDateTime::Tm tm;
                tm.year = m_year;
                tm.mon = wxDateTime::Month(m_month - 1);
                tm.mday = m_day;
                tm.hour = 0;
                tm.min = 0;
                tm.sec = 0;
                tm.msec = 0;
                m_date->SetValue(tm);
                loadDailyFile();
                loadCashFile();
            } else if (index == 1) {
                SubCashFile(m_file, true)()->import(path);
                loadCashFile();
            } else if (index == 2) {
                CatFileRW *catFile = m_file->getCatFile();
                catFile->import(path);
                catFile->setModified();
                m_grid->setCatFileRW(catFile);
                m_grid->updateCat();
            }
        }
        dlg->Destroy();
    }
    fileDlg->Destroy();
}

void MainFrame::onConfigButton(wxCommandEvent &event)
{
    ConfigDialog *dlg = new ConfigDialog();
    char buf[MONEY_LEN + 1];
    money_to_str(buf, m_file->getInitial());
    dlg->setInitial(buf);
    while (dlg->ShowModal() == wxID_OK) {
        int sel = dlg->getSelection();
        if (sel == 0) {
            m_file->setInitial(str_to_money(dlg->getInitial()));
            dailyQuerySave();
            loadDailyFile();
            cashQuerySave();
            loadCashFile();
            break;
        } else if (sel == 1) {
            wxString passwd1 = dlg->getPass1();
            wxString passwd2 = dlg->getPass2();
            if (passwd1 == passwd2) {
                m_file->changeKey(passwd1.ToStdString());
                break;
            }
            wxMessageBox(_("Input passwords are not the same"), _("App name"), wxOK | wxICON_ERROR);
        } else {
            break;
        }
    }
    dlg->Destroy();
}

void MainFrame::onClose(wxCloseEvent &event)
{
    dailyQuerySave();
    cashQuerySave();
    catQuerySave();
#ifndef DEBUG
    removeOldFiles(m_dir);
#ifdef __WXMAC__
    if (!m_file->isOld()) {
        wxString path = wxStandardPaths::Get().GetUserConfigDir();
        path = path.BeforeLast('/') + "/Mobile Documents/com~apple~CloudDocs/HA";
        if (!wxDirExists(path)) {
            wxMkdir(path);
        }
        m_file->copyTo(path);
        removeOldFiles(path);
    }
#endif
#endif
    Destroy();
}

void MainFrame::removeOldFiles(const wxString &dirName)
{
    wxArrayString fileNames = HaFile::getFileList(dirName);
    if (fileNames.GetCount() > 10) {
        fileNames.Sort(false);
        for (int i = 0; i < fileNames.GetCount() - 10; i++) {
            wxRemoveFile(wxFileName(dirName, fileNames[i]).GetFullPath());
        }
    }
}

void MainFrame::loadCatFile()
{
    CatFileRW *cat;
    try {
        cat = m_file->getCatFile();
    } catch (CatFileError &e) {
        wxString str;
        str.Printf(_("Error in category config file at line %1$d"), e.lineNo());
        wxMessageBox(str, _("App name"), wxOK | wxICON_ERROR);
    } catch (CatDupItem &e) {
        wxString str;
        str.Printf(_("Duplicate words in categories config at line %1$d"), e.lineNo());
        wxMessageBox(str, _("App name"), wxOK | wxICON_ERROR);
    }
    m_grid->setCatFileRW(cat);
}

void MainFrame::loadDailyFile()
{
    DailyFileRW *file;
    try {
        file = m_file->getDailyFile(m_year, m_month);
    } catch (DataFileError &e) {
        showDataFileError(e);
    }
    file->afterLoad();
    m_grid->setDataFileRW(file);
}

void MainFrame::loadCashFile()
{
    CashFileRW *file;
    try {
        file = m_file->getCashFile();
    } catch (DataFileError &e) {
        showDataFileError(e);
    }
    file->afterLoad();
    m_cashGrid->setDataFileRW(file);
}

void MainFrame::catQuerySave()
{
    if (m_grid->catModified()) {
        copyFile();
        m_grid->catFileRW()->save();
    }
}

void MainFrame::dailyQuerySave()
{
    if (m_grid->dataModified()) {
        copyFile();
        m_grid->dataFileRW()->save();
    }
}

void MainFrame::cashQuerySave()
{
    if (m_cashGrid->dataModified()) {
        copyFile();
        m_cashGrid->dataFileRW()->save();
    }
}

void MainFrame::copyFile()
{
    if (m_file->isOld()) {
        HaFile *haFile = m_file->newCopy(m_dir);
        delete m_file;
        m_file = haFile;
        m_grid->setHaFile(m_file);
        m_cashGrid->setHaFile(m_file);
    }
}

void MainFrame::showUnknowErrorAndExit()
{
    wxMessageBox(_("Unknown error"), _("App name"), wxOK | wxICON_ERROR);
    wxExit();
}

void MainFrame::showDataFileError(const DataFileError &e)
{
    wxString str;
    str.Printf(_("Error in file \"%1$s\" at line %2$d"), e.fileName(), e.lineNo());
    wxMessageBox(str, _("App name"), wxOK | wxICON_ERROR);
}
