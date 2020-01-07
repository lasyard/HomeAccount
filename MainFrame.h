#ifndef _MAIN_FRAME_H_
#define _MAIN_FRAME_H_

#include <wx/frame.h>

class wxDatePickerCtrl;
class wxDateEvent;
class wxFrame;
class wxStaticBoxSizer;

class DataGrid;
class DailyGrid;
class StatHtml;
class HaDir;
class HaFile;

class DataFileError;
class DailyFileEmpty;
class DailyDateError;
class CatFileError;
class CatDupItem;

class MainFrame : public wxFrame
{
public:
    MainFrame();
    virtual ~MainFrame();

    wxDatePickerCtrl *m_date;
    wxStaticBoxSizer *m_box;
    DailyGrid *m_grid;
    DataGrid *m_cashGrid;
    StatHtml *m_html;
    wxButton *m_monthly;
    wxButton *m_cash;
    wxButton *m_save;
    wxButton *m_stat;
    wxButton *m_export;
    wxButton *m_import;
    wxButton *m_config;

    void initView();
    void onDateChanged(wxDateEvent &event);
    void onMonthlyButton(wxCommandEvent &event);
    void onCashButton(wxCommandEvent &event);
    void onSaveButton(wxCommandEvent &event);
    void onStatButton(wxCommandEvent &event);
    void onExportButton(wxCommandEvent &event);
    void onImportButton(wxCommandEvent &event);
    void onConfigButton(wxCommandEvent &event);
    void onClose(wxCloseEvent &event);

private:
    int m_year;
    int m_month;
    int m_day;
    HaFile *m_file;

    void loadView(bool showCash = false);
    bool loadCatFile();
    void loadDailyFile();
    void loadCashFile();
    void catQuerySave();
    void dailyQuerySave();
    void cashQuerySave();
    void copyFile();
    void showOne(wxWindow *w);

    void showUnknowErrorAndExit();
    void showDataFileError(const DataFileError &e);

    DECLARE_EVENT_TABLE()
};

#endif
