#include <wx/frame.h>
#include <wx/filepicker.h>

class CmpbinFrame : public wxFrame
{
public:
    CmpbinFrame(const wxString &title);
	void InitializeUI();
	wxPanel* MainPanel;
	wxDirPickerCtrl *DirPickerCtrl1, *DirPickerCtrl2;
	wxButton *BtnRunComparison, *BtnBtnCopyComparisonTextToClipboard;
	wxString ComparisonText;

	void BtnRunComparisonEvent(wxCommandEvent &event);
	void BtnCopyComparisonTextToClipboardEvent(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

enum
{
	wxID_BtnRunComparison = wxID_HIGHEST + 1,
	wxID_DirPicker1Id = wxID_HIGHEST + 2,
	wxID_DirPicker2Id = wxID_HIGHEST + 3,
	wxID_BtnSaveAsFile = wxID_HIGHEST + 4,
	wxID_MainPanel = 5
};