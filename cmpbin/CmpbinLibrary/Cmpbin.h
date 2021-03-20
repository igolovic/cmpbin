#include <wx/event.h>

#include "../ListDataItem.h"

void Compare(
    wxString dirPath1,
    wxString dirPath2,
    wxEvtHandler* pParent,
    wxCommandEvent statusEvent,
    wxCommandEvent finishedEvent,
    void (*status)(wxEvtHandler*, wxCommandEvent, wxString),
    void (*finished)(wxEvtHandler*, wxCommandEvent, int, wxString, std::vector<ListDataItem>*)
    );
//, wxString &textOutput, std::vector<ListDataItem> &listDataItems
