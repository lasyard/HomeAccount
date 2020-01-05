#include "HaApp.h"
#include "MainFrame.h"

IMPLEMENT_APP(HaApp)

bool HaApp::OnInit()
{
    MainFrame *frame = new MainFrame;
    SetTopWindow(frame);
    frame->Show();
    frame->initView();
    return true;
}
