//#ifndef DEBUG
//#define DEBUG
//#endif
//
//#ifndef LINUX
//#define LINUX
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
#include "CmpbinApp.h"
#include "WorkerThread.h"
#include "Pics/cmpbin.xpm"

BEGIN_EVENT_TABLE(CmpbinFrame, wxFrame)
    EVT_BUTTON(wxID_BtnRunComparison, CmpbinFrame::BtnRunComparisonEvent)
    EVT_BUTTON(wxID_BtnSaveAsFile, CmpbinFrame::BtnCopyComparisonTextToClipboardEvent)
    EVT_BUTTON(wxID_BtnAbout, CmpbinFrame::BtnAboutEvent)
    EVT_BUTTON(wxID_BtnExit, CmpbinFrame::BtnExitEvent)
    EVT_COMMAND(wxID_ANY, wxEVT_STATUSEVENT, CmpbinFrame::WorkerThreadStatusEvent)
    EVT_COMMAND(wxID_ANY, wxEVT_FINISHEDEVENT, CmpbinFrame::WorkerThreadFinishedEvent)
    EVT_CLOSE(CmpbinFrame::CloseEvent)
END_EVENT_TABLE()

CmpbinFrame::CmpbinFrame(const wxString &title)
    : wxFrame(NULL, wxID_CmpbinFrame, title)
{
	InitializeUI();
    this->SetIcon(cmpbin_xpm);
}

void CmpbinFrame::InitializeUI()
{
    this->CreateStatusBar();
    this->SetStatusText("Select directories and run comparison");

    CmpbinFrame::PPListDataItems = nullptr;

	// Create a top-level panel to hold all the contents of the frame
	CmpbinFrame::MainPanel = new wxPanel(this, wxID_MainPanel);
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

	// Row for file dialog controls
	wxFlexGridSizer *gridSizer1 = new wxFlexGridSizer(2, 4, 3, 3);
	wxFlexGridSizer *gridSizer2 = new wxFlexGridSizer(2, 4, 3, 3);

	CmpbinFrame::StDir1 = new wxStaticText(MainPanel, wxID_ANY, wxT("First directory:"));
	CmpbinFrame::StDir2 = new wxStaticText(MainPanel, wxID_ANY, wxT("Second directory:"));

	CmpbinFrame::DirPickerCtrl1 = new wxDirPickerCtrl(MainPanel, wxID_DirPicker1Id,
		wxEmptyString, wxDirSelectorPromptStr,
		wxDefaultPosition, wxSize(350, wxDefaultCoord));

	CmpbinFrame::DirPickerCtrl2 = new wxDirPickerCtrl(MainPanel, wxID_DirPicker2Id,
		wxEmptyString, wxDirSelectorPromptStr,
		wxDefaultPosition, wxSize(350, wxDefaultCoord));

#ifdef DEBUG

#ifdef LINUX
	CmpbinFrame::DirPickerCtrl1->SetPath("D:\\cmp-folder-1");
	CmpbinFrame::DirPickerCtrl2->SetPath("D:\\cmp-folder-2");
#else
	CmpbinFrame::DirPickerCtrl1->SetPath("/home/user/Documents/cmp1");
	CmpbinFrame::DirPickerCtrl2->SetPath("/home/user/Documents/cmp2");
#endif

#endif

	gridSizer1->Add(CmpbinFrame::StDir1, 0, wxALIGN_CENTRE_VERTICAL | wxRIGHT, 8);
	gridSizer1->Add(DirPickerCtrl1, 1, wxRIGHT, 8);

	gridSizer1->Add(CmpbinFrame::StDir2, 0, wxALIGN_CENTRE_VERTICAL | wxRIGHT, 8);
	gridSizer1->Add(DirPickerCtrl2, 1, wxRIGHT, 8);

    CmpbinFrame::BtnRunComparison = new wxButton(MainPanel, wxID_BtnRunComparison, wxT("Run comparison"));
	gridSizer2->Add(CmpbinFrame::BtnRunComparison, 0);

	CmpbinFrame::BtnCopyComparisonTextToClipboard = new wxButton(MainPanel, wxID_BtnSaveAsFile, wxT("Copy comparison as CSV to clipboard"));
	gridSizer2->Add(CmpbinFrame::BtnCopyComparisonTextToClipboard, 0, wxLEFT | wxBOTTOM, 5);

	CmpbinFrame::BtnAbout = new wxButton(MainPanel, wxID_BtnAbout, wxT("About"));
	gridSizer2->Add(CmpbinFrame::BtnAbout, 0, wxLEFT | wxBOTTOM, 5);

    CmpbinFrame::BtnExit = new wxButton(MainPanel, wxID_BtnExit, wxT("Exit"));
	gridSizer2->Add(CmpbinFrame::BtnExit, 0, wxLEFT | wxBOTTOM, 5);

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

	// Add rows to vertical sizer
	vbox->Add(gridSizer1, 0, wxLEFT | wxLEFT | wxRIGHT | wxTOP, 10);
    vbox->Add(gridSizer2, 0, wxLEFT | wxLEFT | wxRIGHT | wxTOP, 10);
	vbox->Add(hboxListView, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

	MainPanel->SetSizerAndFit(vbox);

	CmpbinFrame::BtnCopyComparisonTextToClipboard->Disable();
}


void CmpbinFrame::BtnRunComparisonEvent(wxCommandEvent &event)
{
	CmpbinFrame::ComparisonText = wxEmptyString;
	Disable();
	CmpbinFrame::ListViewCmp->DeleteAllItems();

	if(CmpbinFrame::PPListDataItems != nullptr)
        delete CmpbinFrame::PPListDataItems;

	const wxString dirPath1 = CmpbinFrame::DirPickerCtrl1->GetPath();
	const wxString dirPath2 = CmpbinFrame::DirPickerCtrl2->GetPath();

	try
	{
		if (wxDirExists(dirPath1) == false)
		{
			wxMessageBox(wxT("Directory 1 is not valid"));
			Enable();
			return;
		}
		else
			if (wxDirExists(dirPath2) == false)
			{
				wxMessageBox(wxT("Directory 2 is not valid"));
				wxEndBusyCursor();
				Enable();
				return;
			}

        pWorkerThread = new WorkerThread(this, dirPath1, dirPath2);
        pWorkerThread->Create();
        if (pWorkerThread->Run() != wxTHREAD_NO_ERROR)
        {
            wxLogError("Can't create the thread!");
            delete pWorkerThread;
            pWorkerThread = NULL;
        }
	}
	catch(...)
	{
		wxMessageBox(wxT("Exception caught"));
		Enable();
		wxEndBusyCursor();
	}
}

void CmpbinFrame::WorkerThreadStatusEvent(wxCommandEvent& event)
{
    wxString message = event.GetString();
    this->SetStatusText(message);
}

void CmpbinFrame::WorkerThreadFinishedEvent(wxCommandEvent& event)
{
    int status = event.GetInt();
    if (status == 0)
    {
        wxString textOutput = event.GetString();
        ComparisonText = textOutput;

        CmpbinFrame::BtnCopyComparisonTextToClipboard->Enable();

        void* pData = event.GetClientData();
        std::vector<ListDataItem> *pListDataItems = reinterpret_cast<std::vector<ListDataItem>*>(pData);

        CmpbinFrame::PPListDataItems = pListDataItems;

        // Show comparison result in listview
        int rowIndex = 0;
        for (std::vector<ListDataItem>::iterator it = pListDataItems->begin(); it != pListDataItems->end(); ++it)
        {
            CmpbinFrame::ListViewCmp->InsertItem(rowIndex, it->FileHash);

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
        wxMessageBox(wxT("Comparison failed with code ") + wxString::Format("%d", status));
        CmpbinFrame::BtnCopyComparisonTextToClipboard->Disable();
    }

    Enable();
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
        info.SetVersion(APP_VERSION_STR);
        info.SetWebSite(wxT("https://github.com/igolovic/cmpbin"), wxT("GIT repository"));
        info.SetDescription(wxT("Ivan Golovic - dedicated to my mom Vera :)"));
        info.SetCopyright(wxT("Copyright (c) 2020 Ivan Golovic"));
        info.SetIcon(cmpbin_xpm);

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


void CmpbinFrame::CloseEvent(wxCloseEvent&)
{
    {
        // Critical section protects multi-threaded access to worker-thread pointer
        wxCriticalSectionLocker enter(pWorkerThreadCS);
        if (pWorkerThread)
        {
            // Set flag that worker-thread should be destroyed
            if (pWorkerThread->Delete() != wxTHREAD_NO_ERROR )
                wxLogError("Can't delete the thread!");
        }
    }

    while (1)
    {
        {
            // Critical section protects multi-threaded access to worker-thread pointer
            wxCriticalSectionLocker enter(pWorkerThreadCS);
            // If destructor of worker-thread ran successfully break from this loop and close window
            if (!pWorkerThread)
                break;
        }

        wxThread::This()->Sleep(1);
    }

    Destroy();
}

void CmpbinFrame::Enable()
{
    CmpbinFrame::StDir1->Enable();
    CmpbinFrame::DirPickerCtrl1->Enable();
    CmpbinFrame::StDir2->Enable();
    CmpbinFrame::DirPickerCtrl2->Enable();
    CmpbinFrame::BtnRunComparison->Enable();
    CmpbinFrame::BtnCopyComparisonTextToClipboard->Enable();
    CmpbinFrame::BtnAbout->Enable();
    CmpbinFrame::ListViewCmp->Enable();
}

void CmpbinFrame::Disable()
{
    CmpbinFrame::StDir1->Disable();
    CmpbinFrame::DirPickerCtrl1->Disable();
    CmpbinFrame::StDir2->Disable();
    CmpbinFrame::DirPickerCtrl2->Disable();
    CmpbinFrame::BtnRunComparison->Disable();
    CmpbinFrame::BtnCopyComparisonTextToClipboard->Disable();
    CmpbinFrame::BtnAbout->Disable();
    CmpbinFrame::ListViewCmp->Disable();
}

CmpbinFrame::~CmpbinFrame()
{
    delete PPListDataItems;
    PPListDataItems = nullptr;
}
