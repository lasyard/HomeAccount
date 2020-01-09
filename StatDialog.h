#ifndef _STAT_DIALOG_H_
#define _STAT_DIALOG_H_

#include <wx/arrstr.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

#include "file/SubFiles.h"

#include "c/utils.h"

class HaFile;

class StatDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(StatDialog);

public:
    StatDialog();

    virtual ~StatDialog()
    {
    }

    int getSelection() const
    {
        return m_book->GetSelection();
    }

    void getData(int &sYear, int &sMonth, int &eYear, int &eMonth)
    {
        sYear = m_sYear;
        sMonth = m_sMonth;
        eYear = m_eYear;
        eMonth = m_eMonth;
    }

    wxString getSelectedYear()
    {
        return m_year->GetString(m_year->GetSelection());
    }

    void setFile(HaFile *file);
    void onRadioBox(wxCommandEvent &event);

private:
    HaFile *m_file;
    wxBookCtrl *m_book;
    wxTextCtrl *m_from;
    wxTextCtrl *m_to;
    wxChoice *m_year;
    int m_sYear, m_sMonth;
    int m_eYear, m_eMonth;

    DECLARE_EVENT_TABLE()
};

#endif
