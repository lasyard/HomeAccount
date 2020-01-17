#ifndef _HA_APP_H_
#define _HA_APP_H_

#include <wx/app.h>

class HaApp : public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit();
    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

protected:
    static const wxString RESOURCE_IN_RES;

    // This must be long lived.
    wxLocale m_locale;
    wxString m_resDir;
    wxString m_dataDir;
    void (HaApp::*m_loadXmlResource)();

    void loadXrcsInRes();
    void loadXrsInResouceDir();
};

DECLARE_APP(HaApp)

#endif
