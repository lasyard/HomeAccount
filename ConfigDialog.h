#ifndef _CONFIG_DIALOG_H_
#define _CONFIG_DIALOG_H_

#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

class ConfigDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(ConfigDialog);

public:
    ConfigDialog()
    {
        wxXmlResource::Get()->LoadObject(this, NULL, "config", "wxDialog");
        m_book = XRCCTRL(*this, "book", wxNotebook);
        m_initial = XRCCTRL(*this, "initial", wxTextCtrl);
        m_pass1 = XRCCTRL(*this, "pass1", wxTextCtrl);
        m_pass2 = XRCCTRL(*this, "pass2", wxTextCtrl);
    }

    virtual ~ConfigDialog()
    {
    }

    int getSelection() const
    {
        return m_book->GetSelection();
    }

    wxString getInitial() const
    {
        return m_initial->GetValue();
    }

    void setInitial(const wxString &v)
    {
        m_initial->SetValue(v);
    }

    wxString getPass1() const
    {
        return m_pass1->GetValue();
    }

    wxString getPass2() const
    {
        return m_pass2->GetValue();
    }

private:
    wxNotebook *m_book;
    wxTextCtrl *m_initial;
    wxTextCtrl *m_pass1;
    wxTextCtrl *m_pass2;

    DECLARE_EVENT_TABLE()
};

#endif
