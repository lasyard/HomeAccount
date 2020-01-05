#ifndef _STAT_DIALOG_H_
#define _STAT_DIALOG_H_

#include <wx/bookctrl.h>
#include <wx/dialog.h>

#include "c/utils.h"

class HaFile;

class wxRadioBox;
class wxTextCtrl;
class wxChoice;

class StatDialog : public wxDialog
{
public:
    StatDialog(wxWindow *parent, wxWindowID id, const wxString &title, HaFile *cryptoFile);

    virtual ~StatDialog()
    {
    }

    wxBookCtrl *m_book;
    wxRadioBox *m_radio;
    wxTextCtrl *m_from;
    wxTextCtrl *m_to;
    wxChoice *m_year;
    int m_sYear, m_sMonth;
    int m_eYear, m_eMonth;

    bool TransferDataFromWindow()
    {
        if (getSelection() == 0) return m_book->GetPage(0)->TransferDataFromWindow();
        return true;
    }

    void onRadioBox(wxCommandEvent &event);

    int getSelection() const
    {
        return m_book->GetSelection();
    }

private:
    HaFile *m_file;

    wxPanel *createStatPanel(wxWindow *parent);
    wxPanel *createAnnuallyPanel(wxWindow *parent);
    wxPanel *createMonthlyPanel(wxWindow *parent);

    DECLARE_EVENT_TABLE()
};

#endif
