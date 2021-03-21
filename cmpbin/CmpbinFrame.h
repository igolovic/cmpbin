#ifndef _CMPBINFRAME_H_
#define _CMPBINFRAME_H_

#include <wx/frame.h>
#include <wx/filepicker.h>
#include <wx/listctrl.h>
#include <wx/stattext.h>

#include "ListDataItem.h"
#include "Enums.h"
#include "WorkerThread.h"
class WorkerThread;

class CmpbinFrame : public wxFrame
{
public:
    CmpbinFrame(const wxString &title);
    virtual ~CmpbinFrame(){};
	void InitializeUI();
	void Enable();
	void Disable();
	wxStaticText *StDir1, *StDir2;
	wxPanel* MainPanel;
	wxDirPickerCtrl *DirPickerCtrl1, *DirPickerCtrl2;
	wxListView* ListViewCmp;
	wxButton *BtnRunComparison, *BtnCopyComparisonTextToClipboard, *BtnAbout, *BtnExit;
	wxString ComparisonText;
    std::vector<ListDataItem> *PPListDataItems;

	void BtnRunComparisonEvent(wxCommandEvent &event);
	void BtnCopyComparisonTextToClipboardEvent(wxCommandEvent &event);
	void BtnAboutEvent(wxCommandEvent &event);
	void BtnExitEvent(wxCommandEvent &event);
    void WorkerThreadStatusEvent(wxCommandEvent& event);
    void WorkerThreadFinishedEvent(wxCommandEvent& event);
    void CloseEvent(wxCloseEvent& event);

    WorkerThread *pWorkerThread;
    wxCriticalSection pWorkerThreadCS;    // protects the worker thread pointer

	DECLARE_EVENT_TABLE()
};

#endif
