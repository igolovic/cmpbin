#ifndef DEBUG
#define DEBUG
#endif

#include <wx/panel.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/filepicker.h>
#include <wx/string.h>
#include <wx/msgdlg.h>
#include <wx/defs.h>
#include <wx/clipbrd.h>
#include <wx/utils.h>

#include "CmpbinFrame.h"
#include "CmpbinLibrary/Cmpbin.h"

CmpbinFrame::CmpbinFrame(const wxString &title)
    : wxFrame(NULL, wxID_ANY, title)
{
	InitializeUI();
}

void CmpbinFrame::InitializeUI()
{
	// Create a top-level panel to hold all the contents of the frame
	CmpbinFrame::MainPanel = new wxPanel(this, wxID_MainPanel);
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

	// Row for file dialog controls
	wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2, 2, 3, 3);

	wxStaticText *stDir1 = new wxStaticText(MainPanel, wxID_ANY, wxT("First directory:"));
	wxStaticText *stDir2 = new wxStaticText(MainPanel, wxID_ANY, wxT("Second directory:"));

	CmpbinFrame::DirPickerCtrl1 = new wxDirPickerCtrl(MainPanel, wxID_DirPicker1Id,
		wxEmptyString, wxDirSelectorPromptStr,
		wxDefaultPosition, wxSize(350, wxDefaultCoord));

	CmpbinFrame::DirPickerCtrl2 = new wxDirPickerCtrl(MainPanel, wxID_DirPicker2Id,
		wxEmptyString, wxDirSelectorPromptStr,
		wxDefaultPosition, wxSize(350, wxDefaultCoord));

#ifdef DEBUG
	CmpbinFrame::DirPickerCtrl1->SetPath("C:\\cmp1");
	CmpbinFrame::DirPickerCtrl2->SetPath("C:\\cmp2");
#endif

	gridSizer->Add(stDir1, 0, wxRIGHT, 8);
	gridSizer->Add(DirPickerCtrl1, 1, wxRIGHT, 8);

	gridSizer->Add(stDir2, 0, wxRIGHT, 8);
	gridSizer->Add(DirPickerCtrl2, 1, wxRIGHT, 8);

	// Row for listview
	wxBoxSizer *hboxListView = new wxBoxSizer(wxHORIZONTAL);

	wxListView* listView = new wxListView(MainPanel, wxID_ANY);
	listView->AppendColumn("1st directory file");
	listView->SetColumnWidth(0, listView->GetSize().GetWidth()*0.5);
	listView->AppendColumn("2nd directory file");
	listView->SetColumnWidth(1, listView->GetSize().GetWidth()*0.5);

	hboxListView->Add(listView, 1, wxEXPAND, 5);

	// Row for buttons
	wxBoxSizer *hboxButtons = new wxBoxSizer(wxHORIZONTAL);

	CmpbinFrame::BtnRunComparison = new wxButton(MainPanel, wxID_BtnRunComparison, wxT("Run comparison"));
	hboxButtons->Add(CmpbinFrame::BtnRunComparison, 0);
	BtnBtnCopyComparisonTextToClipboard = new wxButton(MainPanel, wxID_BtnSaveAsFile, wxT("Copy comparison text to clipboard"));
	hboxButtons->Add(BtnBtnCopyComparisonTextToClipboard, 0, wxLEFT | wxBOTTOM, 5);

	// Add rows to vertical sizer
	vbox->Add(gridSizer, 0, wxLEFT | wxLEFT | wxRIGHT | wxTOP, 10);
	vbox->Add(hboxListView, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	vbox->Add(hboxButtons, 0, wxALIGN_RIGHT | wxRIGHT, 10);

	MainPanel->SetSizerAndFit(vbox);

	BtnBtnCopyComparisonTextToClipboard->Disable();
}


void CmpbinFrame::BtnRunComparisonEvent(wxCommandEvent &event)
{
	CmpbinFrame::ComparisonText = wxEmptyString;
	wxBeginBusyCursor();
	CmpbinFrame::MainPanel->Disable();

	const wxString dirPath1 = CmpbinFrame::DirPickerCtrl1->GetPath();
	const wxString dirPath2 = CmpbinFrame::DirPickerCtrl2->GetPath();

	try
	{
		if (wxDirExists(dirPath1) == false)
		{
			wxMessageBox(wxT("Directory 1 is not valid"));
			CmpbinFrame::MainPanel->Enable();
			return;
		}
		else
			if (wxDirExists(dirPath2) == false)
			{
				wxMessageBox(wxT("Directory 2 is not valid"));
				CmpbinFrame::MainPanel->Enable();
				return;
			}

		std::vector<ListDataItem> listDataItems = std::vector<ListDataItem>();
		int result = Compare(dirPath1, dirPath2, ComparisonText, listDataItems);
		if (result == 0)
		{
			BtnBtnCopyComparisonTextToClipboard->Enable();
		}
		else
		{
			wxMessageBox(wxT("Comparison failed with code ") + wxString::Format("%d", result));
			BtnBtnCopyComparisonTextToClipboard->Disable();
		}

		CmpbinFrame::MainPanel->Enable();
		wxEndBusyCursor();
	}
	catch(...)
	{
		wxMessageBox(wxT("Exception caught"));
		CmpbinFrame::MainPanel->Enable();
		wxEndBusyCursor();
	}
}

void CmpbinFrame::BtnCopyComparisonTextToClipboardEvent(wxCommandEvent &event)
{
	try
	{
		// Write some text to the clipboard
		if (wxTheClipboard->Open())
		{
			// This data objects are held by the clipboard,
			// so do not delete them in the app.
			wxTheClipboard->SetData(new wxTextDataObject(ComparisonText));
			wxTheClipboard->Close();
		}
	}
	catch (...)
	{
		wxMessageBox(wxT("Exception caught"));
	}
}