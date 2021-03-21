#ifndef CMPBINLIBRARY_H_
#define CMPBINLIBRARY_H_

#include "../ListDataItem.h"
#include "../CmpbinFrame.h"

void Compare(
    wxString dirPath1,
    wxString dirPath2,
    CmpbinFrame* pParent,
    wxCommandEvent statusEvent,
    wxCommandEvent finishedEvent,
    void (*status)(CmpbinFrame*, wxCommandEvent, wxString),
    void (*finished)(CmpbinFrame*, wxCommandEvent, int, wxString, std::vector<ListDataItem>*),
    bool (*isCancelled)(CmpbinFrame*, wxCommandEvent)
    );

void FreeResources(std::vector<ListDataItem> *pListDataItems);

#endif
