#include "cbDiffEditor.h"

#include <manager.h>
#include <editormanager.h>
#include <configmanager.h>
#include <logmanager.h>
#include <cbeditor.h>

#include "wxDiff.h"

#include "cbDiffToolbar.h"

/// diffctrls
#include "cbTableCtrl.h"
#include "cbUnifiedCtrl.h"
#include "cbSideBySideCtrl.h"



//! static Editor set
cbDiffEditor::EditorsSet cbDiffEditor::m_AllEditors;


namespace {

    /// IDs
    const long int ID_BBRELOAD          = wxNewId();
    const long int ID_BBSWAP            = wxNewId();
    const long int ID_BUTTON_TABLE      = wxNewId();
    const long int ID_BUTTON_UNIFIED    = wxNewId();
    const long int ID_BUTTON_SIDEBYSIDE = wxNewId();

};

BEGIN_EVENT_TABLE(cbDiffEditor,EditorBase)
    EVT_CONTEXT_MENU(cbDiffEditor::OnContextMenu)
END_EVENT_TABLE()

cbDiffEditor::cbDiffEditor(const wxString &firstfile, const wxString &secondfile, int viewmode, bool leftReadOnly, bool rightReadOnly):
    EditorBase((wxWindow*)Manager::Get()->GetEditorManager()->GetNotebook(), firstfile + secondfile),
    m_diffctrl(0)
{
    m_fromfile = firstfile;
    m_tofile = secondfile;

    m_colorset.m_addedlines = wxColour(0,255,0,50);
    m_colorset.m_removedlines = wxColour(255,0,0,50);
    m_colorset.m_caretlinetype = 0;
    m_colorset.m_caretline = wxColor(122,122,0);

    ConfigManager *cfg = Manager::Get()->GetConfigManager(_T("cbdiffsettings"));
    if (cfg)
    {
        wxColour add = cfg->ReadColour(_T("addedlines"), wxColour(0,255,0,50));
        int addalpha = cfg->ReadInt(_T("addedlinesalpha"), 50);
        wxColour rem = cfg->ReadColour(_T("removedlines"), wxColour(255,0,0,50));
        int remalpha = cfg->ReadInt(_T("removedlinesalpha"), 50);
        m_colorset.m_caretlinetype = cfg->ReadInt(_T("caretlinetype"));
        wxColour car = cfg->ReadColour(_T("caretline"), wxColor(122,122,0));
        int caralpha = cfg->ReadInt(_T("caretlinealpha"), 50);
        m_colorset.m_addedlines = wxColour(add.Red(), add.Green(), add.Blue(), addalpha);
        m_colorset.m_removedlines = wxColour(rem.Red(), rem.Green(), rem.Blue(), remalpha);
        m_colorset.m_caretline = wxColour(car.Red(), car.Green(), car.Blue(), caralpha);

        if(viewmode == DEFAULT)
            viewmode = cfg->ReadInt(_T("viewmode"), 0) + TABLE;
    }
    HighlightLanguage hl = Manager::Get()->GetEditorManager()->GetColourSet()->GetLanguageForFilename(firstfile);
    if (hl != HL_NONE)
        m_colorset.m_hlang = Manager::Get()->GetEditorManager()->GetColourSet()->GetLanguageName(hl);

    cbDiffToolbar* difftoolbar = new cbDiffToolbar(this, viewmode);

    wxBoxSizer* BoxSizer = new wxBoxSizer(wxVERTICAL);
    BoxSizer->Add(difftoolbar, 0, wxALL|wxEXPAND, 0);
    SetSizer(BoxSizer);
    InitDiffCtrl(viewmode, leftReadOnly, rightReadOnly);

    m_AllEditors.insert(this);

    ShowDiff();

    m_diffctrl->Layout();
    BoxSizer->Layout();
    Layout();
}

cbDiffEditor::~cbDiffEditor()
{
    m_AllEditors.erase(this);
}

void cbDiffEditor::ShowDiff()
{
    /* Diff creation */
    wxDiff diff(m_fromfile, m_tofile);
    updateTitle();

    wxString different = diff.IsDifferent();
    if(different != wxEmptyString)
        cbMessageBox(different, _("cbDiff"));

    m_diff = diff.GetDiff();

    m_diffctrl->ShowDiff(diff);
}

bool cbDiffEditor::SaveAsUnifiedDiff()
{
    ConfigManager* mgr = Manager::Get()->GetConfigManager(_T("app"));
    wxString Path = wxGetCwd();
    wxString Filter;
    if(mgr && Path.IsEmpty())
        Path = mgr->Read(_T("/file_dialogs/save_file_as/directory"), Path);

    wxFileDialog dlg(Manager::Get()->GetAppWindow(), _("Save file"), Path, wxEmptyString, _("Diff files (*.diff)|*.diff"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    PlaceWindow(&dlg);
    if (dlg.ShowModal() != wxID_OK)  // cancelled out
        return false;

    wxString Filename = dlg.GetPath();

    // store the last used directory
    if(mgr)
    {
        wxString Test = dlg.GetDirectory();
        mgr->Write(_T("/file_dialogs/save_file_as/directory"), dlg.GetDirectory());
    }

    if(!cbSaveToFile(Filename, m_diff))
    {
        wxString msg;
        msg.Printf(_("File %s could not be saved..."), GetFilename().c_str());
        cbMessageBox(msg, _("Error saving file"), wxICON_ERROR);
        return false;
    }
    return true;
}

void cbDiffEditor::Swap()
{
    wxString temp = m_fromfile;
    m_fromfile = m_tofile;
    m_tofile = temp;
    Reload();
}

void cbDiffEditor::Reload()
{
    if(!m_fromfile.IsEmpty() && !m_tofile.IsEmpty())
        ShowDiff();
}

int cbDiffEditor::GetMode()
{
    return m_viewingmode;
}

void cbDiffEditor::InitDiffCtrl(int mode, bool leftReadOnly, bool rightReadOnly)
{
    leftReadOnly_ = leftReadOnly;
    rightReadOnly_ = rightReadOnly;
    assert(m_diffctrl == nullptr);

    if(mode == TABLE)
        m_diffctrl = new cbTableCtrl(this);
    else if(mode == UNIFIED)
        m_diffctrl = new cbUnifiedCtrl(this);
    else if(mode == SIDEBYSIDE)
        m_diffctrl = new cbSideBySideCtrl(this);

    GetSizer()->Add(m_diffctrl, 1, wxEXPAND, 5);
    m_diffctrl->Init(m_colorset, leftReadOnly_, rightReadOnly_);
    m_viewingmode = mode;

    GetSizer()->Layout();
}

void cbDiffEditor::SetMode(int mode)
{
    if(m_viewingmode == mode)
        return;
    if(m_diffctrl)
    {
        GetSizer()->Detach(m_diffctrl);
        wxDELETE(m_diffctrl);
    }
    InitDiffCtrl(mode, leftReadOnly_, rightReadOnly_);
}

void cbDiffEditor::CloseAllEditors()
{
    EditorsSet s = m_AllEditors;
    for (EditorsSet::iterator i = s.begin(); i != s.end(); ++i )
    {
        EditorManager::Get()->QueryClose(*i);
        (*i)->Close();
    }
    assert(m_AllEditors.empty());
}

bool cbDiffEditor::GetModified() const
{
    return m_diffctrl->GetModified();
}

bool cbDiffEditor::QueryClose()
{
    return m_diffctrl->QueryClose();
}

bool cbDiffEditor::Save()
{
    return m_diffctrl->Save();
}

void cbDiffEditor::updateTitle()
{
    //SetTitle(...) calls Manager::Get()->GetEditorManager() which can fail during shutdown
    if(!Manager::Get()->IsAppShuttingDown())
        SetTitle(_T("Diff: ") +
                 (m_diffctrl->LeftModified() ? _("*") : _("")) + wxFileNameFromPath(m_fromfile) +
                 _T(" ") +
                 (m_diffctrl->RightModified() ? _("*") : _("")) + wxFileNameFromPath(m_tofile));
}

void cbDiffEditor::Undo()                      {m_diffctrl->Undo();}
void cbDiffEditor::Redo()                      {m_diffctrl->Redo();}
void cbDiffEditor::ClearHistory()              {m_diffctrl->ClearHistory();}
void cbDiffEditor::Cut()                       {m_diffctrl->Cut();}
void cbDiffEditor::Copy()                      {m_diffctrl->Copy();}
void cbDiffEditor::Paste()                     {m_diffctrl->Paste();}
bool cbDiffEditor::CanUndo() const      {return m_diffctrl->CanUndo();}
bool cbDiffEditor::CanRedo() const      {return m_diffctrl->CanRedo();}
bool cbDiffEditor::HasSelection() const {return m_diffctrl->HasSelection();}
bool cbDiffEditor::CanPaste() const     {return m_diffctrl->CanPaste();}
bool cbDiffEditor::CanSelectAll() const {return m_diffctrl->CanSelectAll();}
void cbDiffEditor::SelectAll()                 {m_diffctrl->SelectAll();}

void cbDiffEditor::NextDifference()
{
    m_diffctrl->NextDifference();
}

void cbDiffEditor::PrevDifference()
{
    m_diffctrl->PrevDifference();
}

bool cbDiffEditor::CanGotoNextDiff()
{
    return m_diffctrl->CanGotoNextDiff();
}

bool cbDiffEditor::CanGotoPrevDiff()
{
    return m_diffctrl->CanGotoPrevDiff();
}

//void EditorManager::MarkReadOnly(int page, bool readOnly)
//{
//    if (page > -1)
//    {
//        wxBitmap bmp = readOnly ? cbLoadBitmap(ConfigManager::GetDataFolder() + _T("/images/") + _T("readonly.png")) : wxNullBitmap;
//        if (m_pNotebook)
//            m_pNotebook->SetPageBitmap(page, bmp);
//    }
//}
