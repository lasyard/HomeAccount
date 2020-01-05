#include <fstream>
#include <string>

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
#include <wx/sizer.h>
#include <wx/textdlg.h>

#include "ConfigDialog.h"
#include "DailyGrid.h"
#include "HaDefs.h"
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

MainFrame::MainFrame() : wxFrame(NULL, ID_FRAME, _("appName")), m_file(nullptr)
{
    wxImage::AddHandler(new wxPNGHandler);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxFont font = GetFont();
    font.SetPixelSize(wxSize(16, 32));
    wxBoxSizer *sizer_up = new wxBoxSizer(wxHORIZONTAL);
    // date
    m_date = new wxDatePickerCtrl(
        this, ID_DATE, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    m_date->SetFont(font);
    m_date->SetMinSize(wxSize(210, 40));
    sizer_up->Add(m_date, wxSizerFlags().Border());
    // monthly button
    m_monthly = new wxButton(this, ID_MONTHLY, _("btnMonthly"));
    sizer_up->Add(m_monthly, wxSizerFlags().Border().Expand());
    // cash button
    m_cash = new wxButton(this, ID_CASH, _("btnCash"));
    sizer_up->Add(m_cash, wxSizerFlags().Border().Expand());
    // total button
    m_stat = new wxButton(this, ID_STAT, _("btnStat"));
    sizer_up->Add(m_stat, wxSizerFlags().Border().Expand());
    // export button
    m_export = new wxButton(this, ID_EXPORT, _("btnExport"));
    sizer_up->Add(m_export, wxSizerFlags().Border().Expand());
    // import button
    m_import = new wxButton(this, ID_IMPORT, _("btnImport"));
    sizer_up->Add(m_import, wxSizerFlags().Border().Expand());
    // config button
    m_config = new wxBitmapButton(this, ID_CONFIG, wxBITMAP_PNG(CONFIG));
    sizer_up->Add(m_config, wxSizerFlags().Border().Expand());
    // Add top sizer to sizer
    sizer->Add(sizer_up, wxSizerFlags().Border());
    // box
    m_box = new wxStaticBoxSizer(wxVERTICAL, this);
    m_box->SetMinSize(wxSize(960, 720));
    // grid
    m_grid = new DailyGrid(this, ID_GRID);
    m_box->Add(m_grid, wxSizerFlags(1).Border().Expand());
    // cash grid
    m_cashGrid = new DataGrid(this, ID_CASH_GRID);
    m_box->Add(m_cashGrid, wxSizerFlags(1).Border().Expand());
    // html
    m_html = new StatHtml(this, ID_HTML);
    m_box->Add(m_html, wxSizerFlags(1).Border().Expand());
    // controls end
    sizer->Add(m_box, wxSizerFlags(1).Border().Expand());
    SetSizer(sizer);
    sizer->SetSizeHints(this);
    showOne(NULL);
}

MainFrame::~MainFrame()
{
    if (m_file != nullptr) delete m_file;
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_DATE_CHANGED(ID_DATE, MainFrame::onDateChanged)
EVT_BUTTON(ID_MONTHLY, MainFrame::onMonthlyButton)
EVT_BUTTON(ID_CASH, MainFrame::onCashButton)
EVT_BUTTON(ID_STAT, MainFrame::onStatButton)
EVT_BUTTON(ID_EXPORT, MainFrame::onExportButton)
EVT_BUTTON(ID_IMPORT, MainFrame::onImportButton)
EVT_BUTTON(ID_CONFIG, MainFrame::onConfigButton)
EVT_CLOSE(MainFrame::onClose)
END_EVENT_TABLE()

void MainFrame::initView()
{
    m_file = HaFile::getNewestFile();
    int count;
    for (count = 0; count < 3; count++) {
        if (loadCatFile()) break;
        wxPasswordEntryDialog *dlg = new wxPasswordEntryDialog(this, _("msgPassword"), _("appName"));
        if (dlg->ShowModal() == wxID_OK) {
            m_file->setKey(std::string(dlg->GetValue()));
        } else {
            count = 3;
        }
        dlg->Destroy();
    }
    if (count >= 3) wxExit();
    m_year = wxDateTime::Today().GetYear();
    m_month = wxDateTime::Today().GetMonth() + 1;
    m_day = wxDateTime::Today().GetDay();
    loadView();
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
        loadView();
    } else {
        m_day = day;
        if (m_box->IsShown(m_grid)) m_grid->scrollToDay(m_day);
    }
}

void MainFrame::onMonthlyButton(wxCommandEvent &event)
{
    loadView();
}

void MainFrame::onCashButton(wxCommandEvent &event)
{
    loadView(true);
}

void MainFrame::onStatButton(wxCommandEvent &event)
{
    StatDialog *dlg = new StatDialog(this, wxID_ANY, _("appName"), m_file);
    if (dlg->ShowModal() == wxID_OK) {
        dailyQuerySave();
        int sel = dlg->getSelection();
        if (sel == 0) {
            catQuerySave();
            struct cat_root *cat = m_grid->catFileRW()->getCatRoot();
            if (mtree_is_leaf(&cat->root)) {
                wxMessageBox(_("errEmptyCat"), _("appName"), wxOK | wxICON_ERROR);
                dlg->Destroy();
                return;
            }
            int sYear, sMonth;
            int eYear, eMonth;
            sYear = dlg->m_sYear;
            sMonth = dlg->m_sMonth;
            eYear = dlg->m_eYear;
            eMonth = dlg->m_eMonth;
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
            str.Printf(_("msgAnnuallyStat"), t()->minYear(), t()->maxYear());
            m_html->showIO(t(), str);
        } else if (sel == 2) {
            int selection = dlg->m_year->GetSelection();
            wxString yearStr = dlg->m_year->GetString(selection);
            int year = str_to_int(yearStr, yearStr.Len());
            SubMonthlyFile t(m_file, year);
            wxString str;
            str.Printf(_("msgMonthlyStat"), year);
            m_html->showIO(t(), str);
        }
        showOne(m_html);
    }
    dlg->Destroy();
}

void MainFrame::onExportButton(wxCommandEvent &event)
{
    wxArrayString choices;
    choices.Add(_("strCurrentFile"));
    choices.Add(_("strCatFile"));
    wxSingleChoiceDialog *dlg = new wxSingleChoiceDialog(this, _("msgExportChoice"), _("appName"), choices);
    if (dlg->ShowModal() == wxID_OK) {
        int index = dlg->GetSelection();
        wxFileDialog *fileDlg = new wxFileDialog(this,
                                                 wxFileSelectorPromptStr,
                                                 wxEmptyString,
                                                 wxEmptyString,
                                                 wxFileSelectorDefaultWildcardStr,
                                                 wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (fileDlg->ShowModal() == wxID_OK) {
            wxString path = fileDlg->GetPath();
            if (index == 0) {
                if (m_box->IsShown(m_grid)) {
                    dailyQuerySave();
                    m_grid->dataFileRW()->saveAs(std::string(path));
                } else if (m_box->IsShown(m_cashGrid)) {
                    cashQuerySave();
                    m_cashGrid->dataFileRW()->saveAs(std::string(path));
                }
            } else if (index == 1) {
                catQuerySave();
                m_grid->catFileRW()->saveAs(std::string(path));
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
        choices.Add(_("strDataFile"));
        choices.Add(_("strCashFile"));
        choices.Add(_("strCatFile"));
        wxSingleChoiceDialog *dlg = new wxSingleChoiceDialog(this, _("msgExportChoice"), _("appName"), choices);
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
                    showDailyFileEmpty(e);
                    return;
                } catch (DailyDateError &e) {
                    showDailyDateError(e);
                    return;
                }
                file.setHaFile(m_file);
                file.save();
                // TODO
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
                loadView();
            } else if (index == 1) {
                SubCashFile(m_file, true)()->import(path);
                loadView(true);
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
    ConfigDialog *dlg = new ConfigDialog(this, wxID_ANY, _("appName"), true);
    char buf[MONEY_LEN + 1];
    money_to_str(buf, m_file->getInitial());
    dlg->m_initial->SetValue(buf);
    while (dlg->ShowModal() == wxID_OK) {
        int sel = dlg->getSelection();
        if (sel == 0) {
            m_file->setInitial(str_to_money(dlg->m_initial->GetValue()));
            if (m_grid->IsShownOnScreen()) {
                loadView();
            } else if (m_cashGrid->IsShownOnScreen()) {
                loadView(true);
            }
            break;
        } else if (sel == 1) {
            wxString passwd = dlg->m_input->GetValue();
            wxString passwd1 = dlg->m_input1->GetValue();
            if (passwd == passwd1) {
                m_file->changeKey(std::string(passwd));
                break;
            }
            wxMessageBox(_("msgPasswdMismatch"), _("appName"), wxOK | wxICON_ERROR);
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
    Destroy();
}

void MainFrame::loadView(bool showCash)
{
    dailyQuerySave();
    cashQuerySave();
    if (showCash) {
        loadCashFile();
    } else {
        loadDailyFile();
    }
}

bool MainFrame::loadCatFile()
{
    CatFileRW *cat;
    try {
        cat = m_file->getCatFile();
    } catch (CatFileError &e) {
        showCatFileError(e);
    } catch (CatDupItem &e) {
        showCatDupItem(e);
    } catch (FileCorrupt &) {
        wxMessageBox(_("errFileCorrupted"), _("appName"), wxOK | wxICON_ERROR);
        wxExit();
    } catch (BadPassword &) {
        return false;
    }
    m_grid->setCatFileRW(cat);
    return true;
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
    m_grid->scrollToDay(m_day);
    showOne(m_grid);
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
    showOne(m_cashGrid);
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
        HaFile *haFile = m_file->newCopy();
        delete m_file;
        m_file = haFile;
        if (m_grid->catFileRW() != nullptr) {
            m_grid->catFileRW()->setHaFile(m_file);
        }
        if (m_grid->dataFileRW() != nullptr) {
            m_grid->dataFileRW()->setHaFile(m_file);
        }
        if (m_cashGrid->dataFileRW() != nullptr) {
            m_cashGrid->dataFileRW()->setHaFile(m_file);
        }
    }
}

void MainFrame::showOne(wxWindow *w)
{
    wxSizerItemList::iterator iter;
    wxSizerItemList items = m_box->GetChildren();
    for (iter = items.begin(); iter != items.end(); ++iter) {
        wxSizerItem *item = *iter;
        if (item->IsWindow() && item->IsShown() && item->GetWindow() != w) {
            m_box->Hide(item->GetWindow());
        }
    }
    if (w != NULL && !m_box->IsShown(w)) m_box->Show(w);
    m_box->Layout();
    if (w == m_html) {
        m_export->Disable();
    } else {
        m_export->Enable();
    }
}

void MainFrame::showUnknowErrorAndExit()
{
    wxMessageBox(_("errUnknown"), _("appName"), wxOK | wxICON_ERROR);
    wxExit();
}

void MainFrame::showDataFileError(const DataFileError &e)
{
    wxString str;
    str.Printf(_("ErrParse"), e.fileName(), e.lineNo());
    wxMessageBox(str, _("appName"), wxOK | wxICON_ERROR);
}

void MainFrame::showDailyFileEmpty(const DailyFileEmpty &e)
{
    wxMessageBox(_("ErrFileEmpty"), _("AppName"), wxOK | wxICON_ERROR);
}

void MainFrame::showDailyDateError(const DailyDateError &e)
{
    wxString str;
    str.Printf(_("errInvalidDate"), e.title());
    wxMessageBox(str, _("appName"), wxOK | wxICON_ERROR);
}

void MainFrame::showCatFileError(const CatFileError &e)
{
    wxString str;
    str.Printf(_("errCatParse"), e.lineNo());
    wxMessageBox(str, _("appName"), wxOK | wxICON_ERROR);
}

void MainFrame::showCatDupItem(const CatDupItem &e)
{
    wxString str;
    str.Printf(_("errCatDupItem"), e.lineNo());
    wxMessageBox(str, _("appName"), wxOK | wxICON_ERROR);
}
