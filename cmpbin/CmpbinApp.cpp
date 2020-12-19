#include "CmpbinApp.h"
#include "CmpbinFrame.h"

wxIMPLEMENT_APP(CmpbinApp);

bool CmpbinApp::OnInit()
{
	CmpbinFrame* frame = new CmpbinFrame("Cmpbin - compare binary content");
	frame->SetMinClientSize(wxSize(800, 600));
	frame->Show(true);
	frame->Maximize(true);
    return true;
}

BEGIN_EVENT_TABLE(CmpbinFrame, wxFrame)
    EVT_BUTTON(wxID_BtnRunComparison, CmpbinFrame::BtnRunComparisonEvent)
    EVT_BUTTON(wxID_BtnSaveAsFile, CmpbinFrame::BtnCopyComparisonTextToClipboardEvent)
    EVT_BUTTON(wxID_BtnAbout, CmpbinFrame::BtnAboutEvent)
    EVT_BUTTON(wxID_BtnExit, CmpbinFrame::BtnExitEvent)
END_EVENT_TABLE()
