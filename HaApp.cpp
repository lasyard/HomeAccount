#include <wx/cmdline.h>
#include <wx/fs_arc.h>
#include <wx/stdpaths.h>
#include <wx/xrc/xmlres.h>

#include "ArtProvider.h"
#include "HaApp.h"
#include "MainFrame.h"

IMPLEMENT_APP(HaApp)

const wxString HaApp::RESOURCE_IN_RES = "r";

bool HaApp::OnInit()
{
    if (!wxApp::OnInit()) return false;
    m_locale.Init();
    if (!m_locale.AddCatalog("ha")) {
        wxLogError("Couldn't find/load the \"ha\" catalog");
    }
    m_locale.AddCatalog("wxstd");
    wxArtProvider::Push(new ArtProvider(m_resDir));
    wxImage::AddHandler(new wxPNGHandler);
    wxFileSystem::AddHandler(new wxArchiveFSHandler);
    wxXmlResource::Get()->InitAllHandlers();
    (this->*m_loadXmlResource)();
    MainFrame* frame = new MainFrame();
    SetTopWindow(frame);
    frame->Show();
#ifdef DEBUG
    m_dataDir = "HA";
#endif
    frame->initView(m_dataDir);
    return true;
}

int HaApp::OnExit()
{
    return wxApp::OnExit();
}

void HaApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.AddSwitch(RESOURCE_IN_RES);
    wxAppConsole::OnInitCmdLine(parser);
}

bool HaApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    if (!wxApp::OnCmdLineParsed(parser)) return false;
    if (parser.Found(RESOURCE_IN_RES)) {
        m_dataDir = "HA";
        m_resDir = "res";
        m_loadXmlResource = &HaApp::loadXrcsInRes;
    } else {
        m_dataDir = wxStandardPaths::Get().GetDocumentsDir() + "/HA";
        m_resDir = wxStandardPaths::Get().GetResourcesDir();
        m_loadXmlResource = &HaApp::loadXrsInResouceDir;
    }
    return true;
}

void HaApp::loadXrsInResouceDir()
{
    wxXmlResource::Get()->Load(wxFileName(m_resDir, "resources.xrs").GetFullPath());
}

void HaApp::loadXrcsInRes()
{
    wxXmlResource::Get()->LoadAllFiles(m_resDir);
}
