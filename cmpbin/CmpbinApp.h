#ifndef _CMPBINAPP_H_
#define _CMPBINAPP_H_

#include <wx/app.h>

#define    APP_VERSION_STR  wxT("4.0.1.0")

class CmpbinApp : public wxApp
{
public:
    virtual bool OnInit();
    virtual ~CmpbinApp(){};
};

#endif
