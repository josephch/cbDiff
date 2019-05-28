#include "cbDiffMenu.h"

#include <wx/filedlg.h>

#include "cbDiffUtils.h"
#include "cbDiffEditor.h"

#include <projectmanager.h>
#include <editormanager.h>
#include <cbproject.h>
#include <cbeditor.h>

using namespace cbDiffUtils;

namespace
{
    const long ID_SELECT_FIRST = wxNewId();
    const long ID_SELECT_SECOND = wxNewId();
    const long ID_SELECT_LOCAL = wxNewId();
}


BEGIN_EVENT_TABLE(cbDiffMenu, wxMenu)
END_EVENT_TABLE()

cbDiffMenu::cbDiffMenu(wxEvtHandler *parent, wxString basefile, bool &prevSelectionValid, wxString &prevFileName, std::vector<long> &ids):
    wxMenu(),
    basefile_(basefile),
    ids_(ids),
    prevValid_(prevSelectionValid),
    prevFileName_(prevFileName)
{
    if(prevValid_ == false)
    {
        Append(ID_SELECT_FIRST, _("Compare to"));

        parent->Connect(ID_SELECT_FIRST, wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&cbDiffMenu::OnSelectFirst, NULL, this);
    }
    else
    {
        Append(ID_SELECT_SECOND, _("Compare"));
        Append(ID_SELECT_FIRST, _("Reselect first"));

        parent->Connect(ID_SELECT_SECOND, wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&cbDiffMenu::OnSelectSecond, NULL, this);
        parent->Connect(ID_SELECT_FIRST, wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&cbDiffMenu::OnSelectFirst, NULL, this);
    }

    // the project or NULL
    ProjectFile *file = IsFileInActiveProject(basefile);
    projectFileNames_ = GetActiveProjectFilesAbsolute(file);
    openFileNames_ = cbDiffUtils::GetOpenFilesLong(basefile);

    const unsigned int projectFiles = projectFileNames_.GetCount();
    const unsigned int openFiles = openFileNames_.GetCount();

    wxArrayString shortnames = GetActiveProjectFilesRelative(file);

    while (ids_.size() < projectFiles + openFiles)
            ids_.push_back(wxNewId());

    // project open?
    if(projectFiles)
    {
        wxMenu *projmenu = new wxMenu();
        bool hasEntries = false;
        for(unsigned int i = 0; i < shortnames.GetCount(); ++i)
        {
            if( !shortnames[i].IsEmpty() )
            {
                projmenu->Append(ids_[i], shortnames[i]);
                parent->Connect(ids_[i],wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&cbDiffMenu::OnSelectProject, NULL, this);
                hasEntries = true;
            }
        }
        if ( hasEntries )
            AppendSubMenu(projmenu, _("Project files"));
    }


    shortnames = cbDiffUtils::GetOpenFilesShort(basefile);
    // files open?
    if(openFiles)
    {
        wxMenu *openmenu = new wxMenu();
        bool hasEntries = false;
        for(unsigned int i = 0; i < openFiles; i++)
        {
            if( !shortnames[i].IsEmpty() )
            {
                openmenu->Append(ids_[i+projectFiles], shortnames[i]);
                parent->Connect(ids_[i+projectFiles],wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&cbDiffMenu::OnSelectOpen, NULL, this);
                hasEntries = true;
            }
        }
        if ( hasEntries )
            AppendSubMenu(openmenu, _("Open files"));
    }

    Append(ID_SELECT_LOCAL, _("Local file..."));
    parent->Connect(ID_SELECT_LOCAL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&cbDiffMenu::OnSelectLocal, NULL, this);
}

void cbDiffMenu::OnSelectOpen(wxCommandEvent &event)
{
    if(!wxFileExists(basefile_))
        return;

    unsigned int idx;
    for( idx = 0 ; idx < ids_.size() ; ++idx )
    {
        if (ids_[idx] == event.GetId())
            break;
    }
    if ( idx == ids_.size() ) return;

    idx -= projectFileNames_.GetCount();

    if(idx < openFileNames_.GetCount() && wxFileExists(openFileNames_[idx]))
        new cbDiffEditor(basefile_, openFileNames_[idx]);
}

void cbDiffMenu::OnSelectProject(wxCommandEvent &event)
{
    if(!wxFileExists(basefile_))
        return;

    unsigned int idx;
    for( idx = 0 ; idx < ids_.size() ; ++idx )
    {
        if (ids_[idx] == event.GetId())
            break;
    }
    if ( idx == ids_.size() ) return;

    if(idx < projectFileNames_.GetCount() && wxFileExists(projectFileNames_[idx]))
        new cbDiffEditor(basefile_, projectFileNames_[idx]);
}

void cbDiffMenu::OnSelectLocal(wxCommandEvent &event)
{
    if(!wxFileExists(basefile_))
        return;

    wxFileDialog selfile(Manager::Get()->GetAppWindow(), _("Select file"),
                         wxEmptyString, wxEmptyString,
                         wxFileSelectorDefaultWildcardStr,
                         wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_PREVIEW);

    if(selfile.ShowModal() == wxID_OK)
        new cbDiffEditor(basefile_, selfile.GetPath());
}

void cbDiffMenu::OnSelectFirst(wxCommandEvent &event)
{
    prevFileName_ = basefile_;
    prevValid_ = true;

}

void cbDiffMenu::OnSelectSecond(wxCommandEvent &event)
{
    if ( prevValid_ ) new cbDiffEditor(prevFileName_, basefile_);

    prevValid_ = false;
}
