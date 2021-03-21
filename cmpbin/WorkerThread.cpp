#include <ctime>

#include "WorkerThread.h"
#include "CmpbinLibrary/Cmpbin.h"
#include "CmpbinFrame.h"

DEFINE_EVENT_TYPE(wxEVT_STATUSEVENT)
DEFINE_EVENT_TYPE(wxEVT_FINISHEDEVENT)

WorkerThread::WorkerThread(CmpbinFrame* pParent, wxString dirPath1, wxString dirPath2)
    : wxThread(wxTHREAD_DETACHED), PParent(pParent)
{
    PParent = pParent;
    DirPath1 = dirPath1;
    DirPath2 = dirPath2;
}

void Status(CmpbinFrame* pParent, wxCommandEvent evt, wxString message)
{
    evt.SetString(message);
    wxPostEvent(pParent, evt);
}

void Finished(CmpbinFrame* pParent, wxCommandEvent evt, int, wxString textOutput, std::vector<ListDataItem> *pLlistDataItems)
{
    evt.SetString(textOutput);
    evt.SetClientData(reinterpret_cast<void*>(pLlistDataItems));
    wxPostEvent(pParent, evt);
}

bool IsCancelled(CmpbinFrame* pParent, wxCommandEvent evt)
{
    if(pParent->pWorkerThread->TestDestroy())
        return true;

    return false;
}

void* WorkerThread::Entry()
{
    wxCommandEvent StatusEvent(wxEVT_STATUSEVENT, GetId());
    wxCommandEvent FinishedEvent(wxEVT_FINISHEDEVENT, GetId());

    Compare(DirPath1, DirPath2, PParent, StatusEvent, FinishedEvent, &Status, &Finished, &IsCancelled);

    return 0;
}

WorkerThread::~WorkerThread()
{
    wxCriticalSectionLocker enter(PParent->pWorkerThreadCS);

    // Prevention of "dangling pointer"
    PParent->pWorkerThread = NULL;
}
