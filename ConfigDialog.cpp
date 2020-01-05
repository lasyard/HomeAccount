#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valnum.h>

#include "ConfigDialog.h"
#include "HaDefs.h"

ConfigDialog::ConfigDialog(wxWindow *parent, wxWindowID id, const wxString &title, bool change)
    : wxDialog(parent, id, title)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    m_book = new wxBookCtrl(this, wxID_ANY);
    m_book->AddPage(createInitialPanel(m_book), _("strInitial"));
    m_book->AddPage(createPasswordPanel(m_book), _("strChangePasswd"));
    sizer->Add(m_book, wxSizerFlags().Border().Expand());
    wxSizer *sizer_bt = CreateButtonSizer(wxOK | wxCANCEL);
    if (sizer_bt != NULL) sizer->Add(sizer_bt, wxSizerFlags().Border().Expand());
    SetSizer(sizer);
    sizer->SetSizeHints(this);
}

BEGIN_EVENT_TABLE(ConfigDialog, wxDialog)
END_EVENT_TABLE()

wxPanel *ConfigDialog::createInitialPanel(wxWindow *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    m_initial = new wxTextCtrl(
        panel, ID_INITIAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxFloatingPointValidator<float>(2));
    sizer->Add(m_initial, wxSizerFlags().Border().Expand());
    panel->SetSizer(sizer);
    sizer->SetSizeHints(panel);
    return panel;
}

wxPanel *ConfigDialog::createPasswordPanel(wxWindow *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(new wxStaticText(panel, wxID_ANY, _("msgInputPasswd")), wxSizerFlags().Border().Expand());
    m_input = new wxTextCtrl(panel, ID_INPUT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizer->Add(m_input, wxSizerFlags().Border().Expand());
    sizer->Add(new wxStaticText(panel, wxID_ANY, _("msgInputPasswd1")), wxSizerFlags().Border().Expand());
    m_input1 = new wxTextCtrl(panel, ID_INPUT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizer->Add(m_input1, wxSizerFlags().Border().Expand());
    panel->SetSizer(sizer);
    sizer->SetSizeHints(panel);
    return panel;
}
