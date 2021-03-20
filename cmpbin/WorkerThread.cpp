#include <ctime>

#include "WorkerThread.h"
#include "CmpbinLibrary/Cmpbin.h"

DEFINE_EVENT_TYPE(wxEVT_STATUSEVENT)
DEFINE_EVENT_TYPE(wxEVT_FINISHEDEVENT)

WorkerThread::WorkerThread(wxEvtHandler* pParent, wxString dirPath1, wxString dirPath2)
    : wxThread(wxTHREAD_DETACHED), PParent(pParent)
{
    PParent = pParent;
    DirPath1 = dirPath1;
    DirPath2 = dirPath2;
}

void Status(wxEvtHandler* pParent, wxCommandEvent evt, wxString message)
{
    evt.SetString(message);
    wxPostEvent(pParent, evt);
}

void Finished(wxEvtHandler* pParent, wxCommandEvent evt, int, wxString textOutput, std::vector<ListDataItem> *pLlistDataItems)
{
    evt.SetString(textOutput);
    evt.SetClientData(reinterpret_cast<void*>(pLlistDataItems));
    wxPostEvent(pParent, evt);
}

void* WorkerThread::Entry()
{
    wxCommandEvent StatusEvent(wxEVT_STATUSEVENT, GetId());
    wxCommandEvent FinishedEvent(wxEVT_FINISHEDEVENT, GetId());

    Compare(DirPath1, DirPath2, PParent, StatusEvent, FinishedEvent, &Status, &Finished);

    return 0;
}

//enum EventId
//{
//    StatusEvent = 0,
//    FinishedEvent = 1
//}
