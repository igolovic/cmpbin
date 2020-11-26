#include <wx/frame.h>
#include <wx/filepicker.h>
#include <wx/listctrl.h>

class CmpbinFrame : public wxFrame
{
public:
    CmpbinFrame(const wxString &title);
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

	DECLARE_EVENT_TABLE()
};

enum
{
	wxID_BtnRunComparison = wxID_HIGHEST + 1,
	wxID_DirPicker1Id = wxID_HIGHEST + 2,
	wxID_DirPicker2Id = wxID_HIGHEST + 3,
	wxID_BtnSaveAsFile = wxID_HIGHEST + 4,
	wxID_MainPanel = 5,
	wxID_BtnAbout= wxID_HIGHEST + 6,
	wxID_BtnExit = wxID_HIGHEST + 7
};