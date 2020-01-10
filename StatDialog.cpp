#include "StatDialog.h"
#include "MonthValidator.h"

wxIMPLEMENT_DYNAMIC_CLASS(StatDialog, wxDialog);

BEGIN_EVENT_TABLE(StatDialog, wxDialog)
EVT_RADIOBOX(XRCID("radio"), StatDialog::onRadioBox)
END_EVENT_TABLE()

StatDialog::StatDialog()
{
    wxXmlResource::Get()->LoadObject(this, nullptr, "stat", "wxDialog");
    wxPanel *panel = XRCCTRL(*this, "classified", wxPanel);
    m_book = XRCCTRL(*this, "book", wxNotebook);
    m_from = new wxTextCtrl(
        panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, MonthValidator(&m_sYear, &m_sMonth));
    wxXmlResource::Get()->AttachUnknownControl("from", m_from);
    m_to = new wxTextCtrl(
        panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, MonthValidator(&m_eYear, &m_eMonth));
    wxXmlResource::Get()->AttachUnknownControl("to", m_to);
    // set radio button
    wxCommandEvent event(wxEVT_COMMAND_RADIOBOX_SELECTED, XRCID("radio"));
    event.SetInt(0);
    event.SetEventObject(this);
    wxPostEvent(this, event);
}

void StatDialog::onRadioBox(wxCommandEvent &event)
{
    int sel = event.GetInt();
    if (sel < 5) {
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

void StatDialog::setFile(HaFile *file)
{
    m_file = file;
    wxArrayString years;
    SubAnnuallyFile t(m_file);
    for (AnnuallyFileRW::pageIterator i = t()->pageBegin(); i != t()->pageEnd(); ++i) {
        years.Add(i->title.str);
    }
    years.Sort(true);
    wxString str;
    str.Printf(_("Annually statistics from %1$s to %2$s"), years.Last(), years[0]);
    XRCCTRL(*this, "min_max_year", wxStaticText)->SetLabel(str);
    wxPanel *panel = XRCCTRL(*this, "monthly", wxPanel);
    m_year = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, years);
    wxXmlResource::Get()->AttachUnknownControl("years", m_year);
}
