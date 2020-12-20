//#ifndef DEBUG
//#define DEBUG
//#endif

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/filepicker.h>
#include <wx/string.h>
#include <wx/msgdlg.h>
#include <wx/defs.h>
#include <wx/clipbrd.h>
#include <wx/utils.h>
#include <wx/aboutdlg.h>
#include <wx/icon.h>

#include "CmpbinFrame.h"
#include "CmpbinLibrary/Cmpbin.h"
#include "Pics/cmpbin.xpm"

CmpbinFrame::CmpbinFrame(const wxString &title)
    : wxFrame(NULL, wxID_CmpbinFrame, title)
{
	InitializeUI();
    this->SetIcon(cmpbin_xpm);
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

	CmpbinFrame::ListViewCmp = new wxListView(MainPanel, wxID_ANY);
	ListViewCmp->AppendColumn("File content hash");
	ListViewCmp->SetColumnWidth(0, ListViewCmp->GetSize().GetWidth()*0.2);
	ListViewCmp->AppendColumn("1st directory file");
	ListViewCmp->SetColumnWidth(1, ListViewCmp->GetSize().GetWidth()*0.4);
	ListViewCmp->AppendColumn("2nd directory file");
	ListViewCmp->SetColumnWidth(2, ListViewCmp->GetSize().GetWidth()*0.4);

	hboxListView->Add(ListViewCmp, 1, wxEXPAND, 5);

	// Row for buttons
	wxBoxSizer *hboxButtons = new wxBoxSizer(wxHORIZONTAL);

	CmpbinFrame::BtnRunComparison = new wxButton(MainPanel, wxID_BtnRunComparison, wxT("Run comparison"));
	hboxButtons->Add(CmpbinFrame::BtnRunComparison, 0);

	CmpbinFrame::BtnCopyComparisonTextToClipboard = new wxButton(MainPanel, wxID_BtnSaveAsFile, wxT("Copy comparison text to clipboard"));
	hboxButtons->Add(CmpbinFrame::BtnCopyComparisonTextToClipboard, 0, wxLEFT | wxBOTTOM, 5);

	CmpbinFrame::BtnAbout = new wxButton(MainPanel, wxID_BtnAbout, wxT("About"));
	hboxButtons->Add(CmpbinFrame::BtnAbout, 0, wxLEFT | wxBOTTOM, 5);

	CmpbinFrame::BtnExit = new wxButton(MainPanel, wxID_BtnExit, _("Exit"));
	hboxButtons->Add(CmpbinFrame::BtnExit, 0, wxLEFT | wxBOTTOM, 5);

	// Add rows to vertical sizer
	vbox->Add(gridSizer, 0, wxLEFT | wxLEFT | wxRIGHT | wxTOP, 10);
	vbox->Add(hboxListView, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
	vbox->Add(hboxButtons, 0, wxALIGN_RIGHT | wxRIGHT, 10);

	MainPanel->SetSizerAndFit(vbox);

	CmpbinFrame::BtnCopyComparisonTextToClipboard->Disable();
}


void CmpbinFrame::BtnRunComparisonEvent(wxCommandEvent &event)
{
	CmpbinFrame::ComparisonText = wxEmptyString;
	wxBeginBusyCursor();
	CmpbinFrame::MainPanel->Disable();
	CmpbinFrame::ListViewCmp->DeleteAllItems();

	const wxString dirPath1 = CmpbinFrame::DirPickerCtrl1->GetPath();
	const wxString dirPath2 = CmpbinFrame::DirPickerCtrl2->GetPath();

	try
	{
		if (wxDirExists(dirPath1) == false)
		{
			wxMessageBox(wxT("Directory 1 is not valid"));
			wxEndBusyCursor();
			CmpbinFrame::MainPanel->Enable();
			return;
		}
		else
			if (wxDirExists(dirPath2) == false)
			{
				wxMessageBox(wxT("Directory 2 is not valid"));
				wxEndBusyCursor();
				CmpbinFrame::MainPanel->Enable();
				return;
			}

		std::vector<ListDataItem> listDataItems = std::vector<ListDataItem>();
		int result = Compare(dirPath1, dirPath2, ComparisonText, listDataItems);
		if (result == 0)
		{
			CmpbinFrame::BtnCopyComparisonTextToClipboard->Enable();

			// Show comparison result in listview
			int rowIndex = 0;
			for (std::vector<ListDataItem>::iterator it = listDataItems.begin(); it != listDataItems.end(); ++it)
			{
				CmpbinFrame::ListViewCmp->InsertItem(rowIndex, it->Hash);

				if (it->FilesFromDirectory1.size() > 0)
				{
					auto files = it->FilesFromDirectory1;
					wxString filesStr;
					for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
						filesStr = filesStr + *it + wxT(", ");

					filesStr.RemoveLast(2);
					CmpbinFrame::ListViewCmp->SetItem(rowIndex, 1, filesStr);
				}

				if (it->FilesFromDirectory2.size() > 0)
				{
					auto files = it->FilesFromDirectory2;
					wxString filesStr;
					for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
						filesStr = filesStr + *it + wxT(", ");

					filesStr.RemoveLast(2);
					CmpbinFrame::ListViewCmp->SetItem(rowIndex, 2, filesStr);
				}

				rowIndex++;
			}
			CmpbinFrame::ListViewCmp->SetColumnWidth(0, ListViewCmp->GetSize().GetWidth()*0.2);
			CmpbinFrame::ListViewCmp->SetColumnWidth(1, ListViewCmp->GetSize().GetWidth()*0.4);
			CmpbinFrame::ListViewCmp->SetColumnWidth(2, ListViewCmp->GetSize().GetWidth()*0.4);
		}
		else
		{
			wxMessageBox(wxT("Comparison failed with code ") + wxString::Format("%d", result));
			CmpbinFrame::BtnCopyComparisonTextToClipboard->Disable();
		}

		wxEndBusyCursor();
		CmpbinFrame::MainPanel->Enable();
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
			// This data objects are held by the clipboard, so do not delete them in the app.
			wxTheClipboard->SetData(new wxTextDataObject(ComparisonText));
			wxTheClipboard->Close();
		}
	}
	catch (...)
	{
		wxMessageBox(wxT("Exception caught"));
	}
}

void CmpbinFrame::BtnAboutEvent(wxCommandEvent &event)
{
	try
	{
        wxAboutDialogInfo info;

        info.SetName(wxT("Cmpbin"));
        info.SetVersion(wxT("2.0.0.0"));
        info.SetWebSite(wxT("https://github.com/igolovic/cmpbin"), wxT("GIT repository"));
        info.SetDescription(
        wxT("Compare if binary content of each of the files from two folders has match in other folder. Usage:\n"
			"1) select directories with files to compare content\n"
			"2) click '" + CmpbinFrame::BtnRunComparison->GetLabelText() + "'\n"
			"3) view comparison result or copy it to clipboard using '" + CmpbinFrame::BtnCopyComparisonTextToClipboard->GetLabelText() + "'\n"
        ));
        info.SetCopyright(wxT("Copyright (c) 2020 Ivan Golovic - dedicated to my mom Vera :)"));

        wxAboutBox(info);
	}
	catch (...)
	{
		wxMessageBox(wxT("Exception caught"));
	}
}

void CmpbinFrame::BtnExitEvent(wxCommandEvent &event)
{
	try
	{
		Close(true);
	}
	catch (...)
	{
		wxMessageBox(wxT("Exception caught"));
	}
}
