#ifndef _CMPBINFRAME_H_
#define _CMPBINFRAME_H_

#include <wx/frame.h>
#include <wx/filepicker.h>
#include <wx/listctrl.h>
#include <wx/stattext.h>

#include "Enums.h"

class CmpbinFrame : public wxFrame
{
public:
    CmpbinFrame(const wxString &title);
    virtual ~CmpbinFrame(){};
	void InitializeUI();
	wxPanel* MainPanel;
	wxDirPickerCtrl *DirPickerCtrl1, *DirPickerCtrl2;
	wxListView* ListViewCmp;
	wxButton *BtnRunComparison, *BtnCopyComparisonTextToClipboard, *BtnAbout, *BtnExit;
	wxString ComparisonText;

	void BtnRunComparisonEvent(wxCommandEvent &event);
	void BtnCopyComparisonTextToClipboardEvent(wxCommandEvent &event);
	void BtnAboutEvent(wxCommandEvent &event);
	void BtnExitEvent(wxCommandEvent &event);
    void WorkerThreadStatusEvent(wxCommandEvent& event);
    void WorkerThreadFinishedEvent(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif
