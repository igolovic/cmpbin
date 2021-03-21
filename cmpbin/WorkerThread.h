#ifndef MYTHREAD_H_
#define MYTHREAD_H_

#include <wx/thread.h>
#include <wx/event.h>

#include "ListDataItem.h"
#include "CmpbinFrame.h"

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_STATUSEVENT, -1)
    DECLARE_EVENT_TYPE(wxEVT_FINISHEDEVENT, -1)
END_DECLARE_EVENT_TYPES()

class CmpbinFrame;

class WorkerThread : public wxThread
{
    public:
        WorkerThread(CmpbinFrame* pParent, wxString dirPath1, wxString dirPath2);
        ~WorkerThread();
    private:
        wxCommandEvent Evt;
        wxString DirPath1;
        wxString DirPath2;
        void* Entry();
    protected:
        CmpbinFrame* PParent;
};
#endif
