#include <fstream>

#include <wx/app.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/choicdlg.h>
#include <wx/choice.h>
#include <wx/dataview.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/stdpaths.h>
#include <wx/textdlg.h>
#include <wx/xrc/xmlres.h>

#include "CatModel.h"
#include "ConfigDialog.h"
#include "DailyTable.h"
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
EVT_NOTEBOOK_PAGE_CHANGED(XRCID("book"), MainFrame::onPageChanged)
EVT_CLOSE(MainFrame::onClose)
END_EVENT_TABLE()

MainFrame::MainFrame() : m_dir(), m_file(nullptr), m_daily(nullptr), m_cash(nullptr)
{
    wxXmlResource::Get()->LoadObject(this, nullptr, "main", "wxFrame");
    m_book = XRCCTRL(*this, "book", wxNotebook);
    m_grid = XRCCTRL(*this, "daily", DailyGrid);
    m_cashGrid = XRCCTRL(*this, "cash", DataGrid);
    m_datePicker = XRCCTRL(*this, "date", wxDatePickerCtrl);
    m_html = XRCCTRL(*this, "html", StatHtml);
    wxPanel *panel = XRCCTRL(*this, "categories_panel", wxPanel);
    m_catView = new wxDataViewCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES);
    wxXmlResource::Get()->AttachUnknownControl("categories", m_catView);
    m_catView->AppendTextColumn(_("Category"), CatModel::NAME_COLUMN, wxDATAVIEW_CELL_INERT, 200);
    m_catView->AppendTextColumn(_("Count"), CatModel::COUNT_COLUMN, wxDATAVIEW_CELL_INERT, 80);
    m_catView->AppendTextColumn(_("Total"), CatModel::TOTAL_COLUMN, wxDATAVIEW_CELL_INERT, 200);
    m_catView->AppendBitmapColumn(_("In use"), CatModel::INUSE_COLUMN, wxDATAVIEW_CELL_INERT, 80);
    SetIcon(wxICON(APP_ICON));
}

void MainFrame::initView(const wxString &dir)
{
    safeDeleteFile();
    m_dir = dir;
    m_file = HaFile::getNewestFile(m_dir);
    // Must do SetTable, for the grid is painted when showing the dialog.
    m_daily = new DailyTable();
    m_grid->SetTable(m_daily, true);
    m_cash = new DataTable();
    m_cashGrid->SetTable(m_cash, true);
    m_cashGrid->setGrid();
    int count = 0;
    while (true) {
        try {
            m_file->tryLoadFile();
            break;
        } catch (FileCorrupt &) {
            wxMessageBox(_("File corrupted"), _("App name"), wxOK | wxICON_ERROR);
            wxExit();
        } catch (BadPassword &) {
            if (count == 3)
                break;
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
    if (count >= 3) {
        wxExit();
    }
    m_date.SetToCurrent();
    m_datePicker->SetValue(m_date);
    loadDailyFile();
    loadCashFile();
    showDaily();
}

void MainFrame::onDateChanged(wxDateEvent &event)
{
    wxDateTime tm = event.GetDate();
    bool loadNew = false;
    if (tm.GetMonth() != m_date.GetMonth() || tm.GetYear() != m_date.GetYear()) {
        loadNew = true;
    }
    m_date = tm;
    if (loadNew) {
        dailyQuerySave();
        loadDailyFile();
    }
    showDaily();
}

// On Windows, GetOldSelection() == GetSelection(), so `PageChanged` must be handled.
void MainFrame::onPageChanging(wxBookCtrlEvent &event)
{
    int sel = event.GetOldSelection();
    switch (sel) {
    case DAILY_PAGE:
        dailyQuerySave();
        loadCashFile();
        break;
    case CASH_PAGE:
        cashQuerySave();
        break;
    default:
        break;
    }
}

void MainFrame::onPageChanged(wxBookCtrlEvent &event)
{
    if (event.GetSelection() == STATISTICS_PAGE) {
        if (m_html->GetOpenedPageTitle().IsEmpty()) {
            struct cat_root *cat = checkGetCat();
            if (cat != nullptr) {
                int sYear = m_file->minYear();
                int eYear = m_file->maxYear();
                int sMonth = m_file->minMonth(sYear);
                int eMonth = m_file->maxMonth(eYear);
                m_file->calTotal(cat, sYear, sMonth, eYear, eMonth);
                m_html->showTotal(cat, sYear, sMonth, eYear, eMonth);
            }
        }
    } else if (event.GetSelection() == CAT_PAGE) {
        struct cat_root *cat = checkGetCat();
        if (cat == nullptr) {
            return;
        }
        int sYear = m_file->minYear();
        int sMonth = m_file->minMonth(sYear);
        int eYear = m_file->maxYear();
        int eMonth = m_file->maxMonth(eYear);
        m_file->calTotal(cat, sYear, sMonth, eYear, eMonth);
        CatModel *model = new CatModel(cat);
        m_catView->AssociateModel(model);
        expandAllCat(wxDataViewItem(model->income()));
        expandAllCat(wxDataViewItem(model->outlay()));
        model->DecRef();
    }
}

void MainFrame::expandAllCat(const wxDataViewItem &item)
{
    m_catView->Expand(item);
    const CatItem *it = static_cast<const CatItem *>(item.GetID());
    std::vector<CatItem *> children = it->getChildren();
    for (auto i = children.cbegin(); i != children.cend(); ++i) {
        if ((*i)->hasChild()) {
            expandAllCat(wxDataViewItem(*i));
        }
    }
}

struct cat_root *MainFrame::checkGetCat()
{
    struct cat_root *cat = m_daily->getCatRoot();
    if (mtree_is_leaf(&cat->root)) {
        wxMessageBox(_("Category config is empty"), _("App name"), wxOK | wxICON_ERROR);
        return nullptr;
    }
    return cat;
}

void MainFrame::onStatButton(wxCommandEvent &event)
{
    dailyQuerySave();
    StatDialog *dlg = new StatDialog();
    dlg->setFile(m_file);
    if (dlg->ShowModal() == wxID_OK) {
        dailyQuerySave();
        int sel = dlg->getSelection();
        if (sel == 0) {
            struct cat_root *cat = checkGetCat();
            if (cat == nullptr) {
                dlg->Destroy();
                return;
            }
            int sYear, sMonth, eYear, eMonth;
            dlg->getData(sYear, sMonth, eYear, eMonth);
            m_file->calTotal(cat, sYear, sMonth, eYear, eMonth);
            m_html->showTotal(cat, sYear, sMonth, eYear, eMonth);
        } else if (sel == 1) {
            SubAnnuallyFile t(m_file);
            wxString str;
            str.Printf(_("Annually statistics"), t()->minYear(), t()->maxYear());
            m_html->showIO(t(), str);
            m_html->setFileName("annually_statistics");
        } else if (sel == 2) {
            wxString yearStr = dlg->getSelectedYear();
            int year = str_to_int(yearStr, yearStr.Len());
            SubMonthlyFile t(m_file, year);
            wxString str;
            str.Printf(_("Monthly statistics"), year);
            m_html->showIO(t(), str);
            m_html->setFileName(wxString::Format("monthly_statistics_of_year_%1$d", year));
        }
        showStatistics();
    }
    dlg->Destroy();
}

void MainFrame::onExportButton(wxCommandEvent &event)
{
    wxArrayString choices;
    choices.Add(_("Current datum"));
    // choices.Add(_("Categories config"));
    choices.Add(_("All datum"));
    wxSingleChoiceDialog *dlg = new wxSingleChoiceDialog(this, _("Choose one to export"), _("App name"), choices);
    if (dlg->ShowModal() == wxID_OK) {
        int index = dlg->GetSelection();
        wxString wildCardString;
        wxString defaultFileName;
        wildCardString = "Text files (*.txt)|*.txt";
        if (index == 0) {
            if (m_book->GetSelection() == STATISTICS_PAGE) {
                if (m_html->fileName().IsEmpty()) {
                    wxMessageBox(_("Nothing to export"), _("App name"), wxOK | wxICON_INFORMATION);
                    return;
                }
                wildCardString = "HTML files (*.html)|*.html";
                defaultFileName = m_html->fileName();
            } else if (m_book->GetSelection() == DAILY_PAGE) {
                defaultFileName = wxString::Format("daily_%s", m_daily->dataFileName());
            } else if (m_book->GetSelection() == CASH_PAGE) {
                defaultFileName = m_cash->dataFileName();
            } else if (m_book->GetSelection() == CAT_PAGE) {
                defaultFileName = "category";
            }
        } else if (index == 1) {
            defaultFileName = "ha_all_data";
        }
        defaultFileName.Replace("/", "_");
        wxFileDialog *fileDlg = new wxFileDialog(this,
                                                 wxFileSelectorPromptStr,
                                                 wxEmptyString,
                                                 defaultFileName,
                                                 wildCardString,
                                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (fileDlg->ShowModal() == wxID_OK) {
            dailyQuerySave();
            std::string path = fileDlg->GetPath().ToStdString();
            if (index == 0) {
                if (m_book->GetSelection() == DAILY_PAGE) {
                    m_daily->saveAs(path);
                } else if (m_book->GetSelection() == CASH_PAGE) {
                    m_cash->saveAs(path);
                } else if (m_book->GetSelection() == STATISTICS_PAGE) {
                    m_html->saveAs(path);
                } else if (m_book->GetSelection() == CAT_PAGE) {
                    m_daily->saveCatAs(path);
                }
            } else if (index == 1) {
                m_file->exportAll(path);
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
        dailyQuerySave();
        cashQuerySave();
        std::string path = fileDlg->GetPath().ToStdString();
        FileType *file;
        copyFile();
        try {
            file = m_file->import(path);
        } catch (DataFileError &e) {
            showFileError(e);
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
        if (file == nullptr) {
            wxMessageBox(_("Not a valid file."), _("App name"), wxOK | wxICON_ERROR);
            return;
        }
        switch (file->type()) {
        case FileType::DAILY: {
            DailyFileRW *daily = static_cast<DailyFileRW *>(file);
            m_date = wxDateTime(1, wxDateTime::Month(daily->month() - 1), daily->year());
            m_datePicker->SetValue(m_date);
            loadDailyFile();
            showDaily();
        } break;
        case FileType::CASH:
            loadCashFile();
            showCash();
            break;
        case FileType::CAT:
            loadDailyFile();
            break;
        case FileType::WHOLE:
            loadDailyFile();
            loadCashFile();
            break;
        case FileType::INVALID:
            break;
        }
        delete file;
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
        for (size_t i = 0; i < fileNames.GetCount() - 10; i++) {
            wxRemoveFile(wxFileName(dirName, fileNames[i]).GetFullPath());
        }
    }
}

void MainFrame::loadDailyFile()
{
    CatFileRW *cat = nullptr;
    DailyFileRW *file = nullptr;
    try {
        file = m_file->getDailyFile(m_date.GetYear(), m_date.GetMonth() + 1);
        file->afterLoad();
        cat = m_file->getCatFile();
    } catch (DataFileError &e) {
        showFileError(e);
    } catch (CatFileError &e) {
        wxString str;
        str.Printf(_("Error in category config file at line %1$d"), e.lineNo());
        wxMessageBox(str, _("App name"), wxOK | wxICON_ERROR);
    } catch (CatDupItem &e) {
        wxString str;
        str.Printf(_("Duplicate words in categories config at line %1$d"), e.lineNo());
        wxMessageBox(str, _("App name"), wxOK | wxICON_ERROR);
    }
    // The number of cols and rows are only updated in SetTable, so make new table each time.
    m_daily = new DailyTable(file, cat);
    m_grid->SetTable(m_daily, true);
    // Row labels are not updated even by SetTable, so do this.
    m_grid->ForceRefresh();
    m_grid->setGrid();
}

void MainFrame::loadCashFile()
{
    CashFileRW *file = nullptr;
    try {
        file = m_file->getCashFile();
        file->afterLoad();
    } catch (DataFileError &e) {
        showFileError(e);
    }
    m_cash = new DataTable(file);
    m_cashGrid->SetTable(m_cash, true);
    m_cashGrid->ForceRefresh();
    m_cashGrid->setGrid();
}

void MainFrame::dailyQuerySave()
{
    m_grid->SaveEditControlValue();
    if (m_daily->catModified()) {
        copyFile();
        m_daily->saveCat();
    }
    if (m_daily->dataModified()) {
        copyFile();
        m_daily->save();
    }
}

void MainFrame::cashQuerySave()
{
    m_cashGrid->SaveEditControlValue();
    if (m_cash->dataModified()) {
        copyFile();
        m_cash->save();
    }
}

void MainFrame::copyFile()
{
    if (m_file->isOld()) {
        HaFile *haFile = m_file->newCopy(m_dir);
        delete m_file;
        m_file = haFile;
        m_daily->setHaFile(m_file);
        m_cash->setHaFile(m_file);
    }
}

void MainFrame::showUnknowErrorAndExit()
{
    wxMessageBox(_("Unknown error"), _("App name"), wxOK | wxICON_ERROR);
    wxExit();
}

void MainFrame::showFileError(const FileError &e)
{
    wxString str;
    str.Printf(_("Error in file \"%1$s\" at line %2$d"), e.fileName(), e.lineNo());
    wxMessageBox(str, _("App name"), wxOK | wxICON_ERROR);
}
