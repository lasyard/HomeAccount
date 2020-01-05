#ifndef _CONFIG_DIALOG_H_
#define _CONFIG_DIALOG_H_

#include <wx/bookctrl.h>
#include <wx/dialog.h>

class wxTextCtrl;

class ConfigDialog : public wxDialog
{
public:
    ConfigDialog(wxWindow *parent, wxWindowID id, const wxString &title, bool change = false);

    virtual ~ConfigDialog()
    {
    }

    wxBookCtrl *m_book;
    wxTextCtrl *m_initial;
    wxTextCtrl *m_input;
    wxTextCtrl *m_input1;

    int getSelection() const
    {
        return m_book->GetSelection();
    }

private:
    wxPanel *createInitialPanel(wxWindow *parent);
    wxPanel *createPasswordPanel(wxWindow *parent);

    DECLARE_EVENT_TABLE()
};

#endif
