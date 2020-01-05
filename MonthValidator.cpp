#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/window.h>

#include "MonthValidator.h"

#include "c/utils.h"

bool MonthValidator::Validate(wxWindow *parent)
{
    if (tryTransfer()) return true;
    wxMessageBox(_("errInputDate"), _("appName"), wxOK | wxICON_ERROR);
    return false;
}

bool MonthValidator::TransferFromWindow()
{
    return tryTransfer();
}

bool MonthValidator::TransferToWindow()
{
    char buf[YEAR_LEN + MONTH_LEN + 1];
    char *p = buf;
    wxTextCtrl *text = getTextCtrl();
    if (text == NULL) return false;
    p += int_to_str_len(p, *m_year, YEAR_LEN);
    p += int_to_str_len(p, *m_month, MONTH_LEN);
    *p = '\0';
    text->SetValue(buf);
    return true;
}

wxTextCtrl *MonthValidator::getTextCtrl()
{
    wxWindow *win = GetWindow();
    if (win->IsKindOf(CLASSINFO(wxTextCtrl))) {
        return (wxTextCtrl *)win;
    } else {
        wxFAIL_MSG("MonthValidator can only be used with wxTextCtrl");
    }
    return NULL;
}

bool MonthValidator::tryTransfer()
{
    wxString text = getTextCtrl()->GetValue();
    if (text.Length() != YEAR_LEN + MONTH_LEN) return false;
    if (!str_to_ym(text.c_str(), m_year, m_month, '\0')) return false;
    return true;
}
