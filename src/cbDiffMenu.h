#ifndef CBDIFFMENU_H
#define CBDIFFMENU_H

#include <wx/menu.h>
#include <vector>

class cbDiffMenu : public wxMenu
{
public:
    cbDiffMenu(wxEvtHandler *parent, wxString basefile, bool &prevSelected, wxString &prevFileName, std::vector<long> &ids);
    virtual ~cbDiffMenu(){}
private:
    wxString basefile_;
    wxArrayString projectFileNames_;
    wxArrayString openFileNames_;

    std::vector<long> &ids_;

    bool &prevValid_;
    wxString &prevFileName_;

    void OnSelectProject(wxCommandEvent &event);
    void OnSelectOpen(wxCommandEvent &event);
    void OnSelectLocal(wxCommandEvent &event);
    void OnSelectFirst(wxCommandEvent &event);
    void OnSelectSecond(wxCommandEvent &event);
    DECLARE_EVENT_TABLE();
};

#endif
