#ifndef _MAIN_FRAME_H_
#define _MAIN_FRAME_H_

#include <wx/frame.h>
#include <wx/notebook.h>

#include "DailyGrid.h"
#include "file/HaFile.h"

class wxDatePickerCtrl;
class wxDateEvent;

class wxNotebook;
class DataGrid;
class DailyGrid;
class StatHtml;

class DataFileError;

class MainFrame : public wxFrame
{
    wxDECLARE_DYNAMIC_CLASS(MainFrame);

public:
    MainFrame();

    virtual ~MainFrame()
    {
        safeDeleteFile();
    }

    void initView(const wxString &dir);
    void onDateChanged(wxDateEvent &event);
    void onMonthlyButton(wxCommandEvent &event);
    void onCashButton(wxCommandEvent &event);
    void onSaveButton(wxCommandEvent &event);
    void onStatButton(wxCommandEvent &event);
    void onExportButton(wxCommandEvent &event);
    void onImportButton(wxCommandEvent &event);
    void onConfigButton(wxCommandEvent &event);
    void onClose(wxCloseEvent &event);
    void onPageChanging(wxBookCtrlEvent &event);

private:
    static const int DAILY_PAGE = 0;
    static const int CASH_PAGE = 1;
    static const int STATISTICS_PAGE = 2;

    int m_year;
    int m_month;
    int m_day;
    wxString m_dir;
    HaFile *m_file;

    wxNotebook *m_book;
    DailyGrid *m_grid;
    DataGrid *m_cashGrid;
    StatHtml *m_html;
    wxDatePickerCtrl *m_date;

    void safeDeleteFile()
    {
        if (m_file != nullptr) delete m_file;
        m_file = nullptr;
    }

    void showDaily()
    {
        m_grid->scrollToDay(m_day);
        m_book->SetSelection(DAILY_PAGE);
    }

    void showCash()
    {
        m_book->SetSelection(CASH_PAGE);
    }

    void showStatistics()
    {
        m_book->SetSelection(STATISTICS_PAGE);
    }

    void loadCatFile();
    void loadDailyFile();
    void loadCashFile();
    void catQuerySave();
    void dailyQuerySave();
    void cashQuerySave();
    void copyFile();
    void removeOldFiles(const wxString &dirName);

    void showUnknowErrorAndExit();
    void showDataFileError(const DataFileError &e);

    DECLARE_EVENT_TABLE()
};

#endif
