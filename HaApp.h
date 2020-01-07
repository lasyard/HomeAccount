#ifndef _HA_APP_H_
#define _HA_APP_H_

#include <wx/app.h>

class HaApp : public wxApp
{
public:
    virtual bool OnInit();

protected:
    wxLocale m_locale;
};

DECLARE_APP(HaApp)

#endif
