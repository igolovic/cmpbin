#include "CmpbinApp.h"
#include "CmpbinFrame.h"
#include "WorkerThread.h"

wxIMPLEMENT_APP(CmpbinApp);

bool CmpbinApp::OnInit()
{
	CmpbinFrame* frame = new CmpbinFrame("Cmpbin - compare binary content");
	frame->SetMinClientSize(wxSize(800, 600));
	frame->Show(true);
	frame->Maximize(true);
    return true;
}
