#include "CmpbinApp.h"
#include "CmpbinFrame.h"
#include "WorkerThread.h"

wxIMPLEMENT_APP(CmpbinApp);

bool CmpbinApp::OnInit()
{
	CmpbinFrame* frame = new CmpbinFrame(wxString::Format("Cmpbin %s - compare files in two folders by their binary content and find matches and unique files", APP_VERSION_STR));
	frame->SetMinClientSize(wxSize(800, 600));
	frame->Show(true);
	frame->Maximize(true);
    return true;
}
