#include <wx/choice.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "HaDefs.h"
#include "MonthValidator.h"
#include "StatDialog.h"
#include "file/AnnuallyFileRW.h"
#include "file/HaFile.h"
#include "file/SubFiles.h"

StatDialog::StatDialog(wxWindow *parent, wxWindowID id, const wxString &title, HaFile *cryptoFile)
    : wxDialog(parent, id, title), m_file(cryptoFile)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    m_book = new wxBookCtrl(this, wxID_ANY);
    m_book->AddPage(createStatPanel(m_book), _("strCatStat"));
    m_book->AddPage(createAnnuallyPanel(m_book), _("strAnnuallyStat"));
    m_book->AddPage(createMonthlyPanel(m_book), _("strMonthlyStat"));
    sizer->Add(m_book, wxSizerFlags().Border().Expand());
    wxSizer *sizer_bt = CreateButtonSizer(wxOK | wxCANCEL);
    if (sizer_bt != NULL) sizer->Add(sizer_bt, wxSizerFlags().Border().Expand());
    SetSizer(sizer);
    sizer->SetSizeHints(this);
    // set radio button
    wxCommandEvent event(wxEVT_COMMAND_RADIOBOX_SELECTED, ID_RADIO);
    event.SetInt(0);
    event.SetEventObject(this);
    wxPostEvent(this, event);
}

BEGIN_EVENT_TABLE(StatDialog, wxDialog)
EVT_RADIOBOX(ID_RADIO, StatDialog::onRadioBox)
END_EVENT_TABLE()

void StatDialog::onRadioBox(wxCommandEvent &event)
{
    int sel = event.GetInt();
    if (sel < 4) {
        m_from->Disable();
        m_to->Disable();
    }
    int year = wxDateTime::Today().GetYear();
    int month = wxDateTime::Today().GetMonth() + 1;
    switch (sel) {
        case 0:
            m_sYear = m_eYear = year;
            m_sMonth = m_eMonth = month;
            break;
        case 1:
            if (--month == 0) {
                year--;
                month = 12;
            }
            m_sYear = m_eYear = year;
            m_sMonth = m_eMonth = month;
            break;
        case 2:
            m_sYear = m_eYear = year;
            m_sMonth = m_file->minMonth(year);
            m_eMonth = m_file->maxMonth(year);
            break;
        case 3:
            m_sYear = m_eYear = year - 1;
            m_sMonth = m_file->minMonth(year - 1);
            m_eMonth = m_file->maxMonth(year - 1);
            break;
        case 4:
            m_sYear = m_file->minYear();
            m_eYear = m_file->maxYear();
            m_sMonth = m_file->minMonth(m_sYear);
            m_eMonth = m_file->maxMonth(m_eYear);
            break;
        case 5:
            m_from->Enable();
            m_to->Enable();
            break;
        default:
            break;
    }
    m_book->GetPage(0)->TransferDataToWindow();
}

wxPanel *StatDialog::createStatPanel(wxWindow *parent)
{
    static wxString choices[] = {
        _("strCurMonth"),
        _("strPrevMonth"),
        _("strCurYear"),
        _("strPrevYear"),
        _("strAll"),
        _("strCustom"),
    };
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    // radio buttons
    m_radio = new wxRadioBox(
        panel, ID_RADIO, _("strChoice"), wxDefaultPosition, wxDefaultSize, sizeof(choices) / sizeof(wxString), choices);
    sizer->Add(m_radio, wxSizerFlags().Border().Expand());
    // top controls end
    wxBoxSizer *sizer_md = new wxBoxSizer(wxHORIZONTAL);
    // from label
    sizer_md->Add(new wxStaticText(panel, wxID_ANY, _("strFrom")), wxSizerFlags().Border().Expand());
    // from textctrl
    m_from = new wxTextCtrl(
        panel, ID_FROM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, MonthValidator(&m_sYear, &m_sMonth));
    sizer_md->Add(m_from, wxSizerFlags().Border().Expand());
    // to label
    sizer_md->Add(new wxStaticText(panel, wxID_ANY, _("strTo")), wxSizerFlags().Border().Expand());
    // to textctrl
    m_to = new wxTextCtrl(
        panel, ID_TO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, MonthValidator(&m_eYear, &m_eMonth));
    sizer_md->Add(m_to, wxSizerFlags().Border().Expand());
    // middle controls end
    sizer->Add(sizer_md, wxSizerFlags().Border().Expand());
    sizer->Add(new wxStaticText(panel, wxID_ANY, _("msgDateFormat")), wxSizerFlags().Border().Expand());
    panel->SetSizer(sizer);
    sizer->SetSizeHints(panel);
    return panel;
}

wxPanel *StatDialog::createAnnuallyPanel(wxWindow *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxString str;
    str.Printf(_("msgAnnuallyStat"), m_file->minYear(), m_file->maxYear());
    sizer->Add(new wxStaticText(panel, wxID_ANY, str), wxSizerFlags().Border().Expand());
    panel->SetSizer(sizer);
    sizer->SetSizeHints(panel);
    return panel;
}

wxPanel *StatDialog::createMonthlyPanel(wxWindow *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxArrayString years;
    SubAnnuallyFile t(m_file);
    for (AnnuallyFileRW::pageIterator i = t()->pageBegin(); i != t()->pageEnd(); ++i) {
        years.Add(i->title.str);
    }
    m_year = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, years);
    if (m_year->GetCount() > 0) {
        m_year->SetSelection(m_year->GetCount() - 1);
    }
    sizer->Add(m_year, wxSizerFlags().Border().Expand());
    panel->SetSizer(sizer);
    sizer->SetSizeHints(panel);
    return panel;
}
