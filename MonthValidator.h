#ifndef _MONTH_VALIDATOR_H_
#define _MONTH_VALIDATOR_H_

class MonthValidator : public wxValidator
{
public:
    MonthValidator(int *year, int *month) : m_year(year), m_month(month)
    {
    }

    MonthValidator(const MonthValidator &obj)
    {
        wxValidator::Copy(obj);
        m_year = obj.m_year;
        m_month = obj.m_month;
    }

    virtual ~MonthValidator()
    {
    }

    wxObject *Clone() const
    {
        return new MonthValidator(*this);
    }

    bool Validate(wxWindow *parent);
    bool TransferFromWindow();
    bool TransferToWindow();

private:
    int *m_year;
    int *m_month;

    wxTextCtrl *getTextCtrl();
    bool tryTransfer();
};

#endif
