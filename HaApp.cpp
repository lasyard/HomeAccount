#include <wx/fs_arc.h>
#include <wx/stdpaths.h>
#include <wx/xrc/xmlres.h>

#include "HaApp.h"
#include "MainFrame.h"

IMPLEMENT_APP(HaApp)

bool HaApp::OnInit()
{
    if (!wxApp::OnInit()) return false;
    m_locale.Init();
    if (!m_locale.AddCatalog("ha")) {
        wxLogError("Couldn't find/load the \"ha\" catalog");
    }
    m_locale.AddCatalog("wxstd");
    wxFileSystem::AddHandler(new wxArchiveFSHandler);
    wxXmlResource::Get()->InitAllHandlers();
    wxXmlResource::Get()->Load(wxFileName(wxStandardPaths::Get().GetResourcesDir(), "resources.xrs").GetFullPath());
    MainFrame *frame = new MainFrame;
    SetTopWindow(frame);
    frame->Show();
    frame->initView();
    return true;
}
