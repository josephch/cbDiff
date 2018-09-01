#include "cbDiffEditor.h"

#include <manager.h>
#include <editormanager.h>
#include <configmanager.h>
#include <logmanager.h>
#include <cbeditor.h>

#include "wxDiff.h"

/// diffctrls
#include "cbTableCtrl.h"
#include "cbUnifiedCtrl.h"
#include "cbSideBySideCtrl.h"

#include "cbDiffUtils.h"

/// bitmaps
#include "../images/swap.h"
#include "../images/reload.h"
#include "../images/table.h"
#include "../images/unified.h"
#include "../images/sidebyside.h"

//! static Editor set
cbDiffEditor::EditorsSet cbDiffEditor::m_AllEditors;


namespace {

    /// IDs
    const long int ID_BBRELOAD = wxNewId();
    const long int ID_BBSWAP = wxNewId();
    const long int ID_BUTTON_TABLE = wxNewId();
    const long int ID_BUTTON_UNIFIED = wxNewId();
    const long int ID_BUTTON_SIDEBYSIDE = wxNewId();

};

BEGIN_EVENT_TABLE(cbDiffEditor,EditorBase)
    EVT_CONTEXT_MENU(cbDiffEditor::OnContextMenu)
END_EVENT_TABLE()

cbDiffEditor::cbDiffEditor(const wxString& firstfile, const wxString& secondfile, int viewmode):
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

    SetSizer(new wxBoxSizer(wxVERTICAL));
    CreateDiffToolButtons(viewmode);
    InitDiffCtrl(viewmode);

    m_AllEditors.insert(this);

    ShowDiff();

    Layout();
}

cbDiffEditor::~cbDiffEditor()
{
    m_AllEditors.erase(this);
    Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&cbDiffEditor::OnToolButton);
}

void cbDiffEditor::ShowDiff()
{
    /* Diff creation */
    wxDiff diff(m_fromfile, m_tofile);
    SetTitle(_T("Diff: ") + wxFileNameFromPath(m_fromfile) + _T(" ") + wxFileNameFromPath(m_tofile));

    wxString different = diff.IsDifferent();
    if(different != wxEmptyString)
        cbMessageBox(different, _("cbDiff"));

    m_diffctrl->ShowDiff(diff);
}

bool cbDiffEditor::Save()
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
        return false; // failed; file is read-only?
    }
    return true;;
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

void cbDiffEditor::InitDiffCtrl(int mode)
{
    assert(m_diffctrl == nullptr);

    if(mode == TABLE)
        m_diffctrl = new cbTableCtrl(this);
    else if(mode == UNIFIED)
        m_diffctrl = new cbUnifiedCtrl(this);
    else if(mode == SIDEBYSIDE)
        m_diffctrl = new cbSideBySideCtrl(this);

    GetSizer()->Add(m_diffctrl, 1, wxEXPAND, 5);
    m_diffctrl->Init(m_colorset);
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
    InitDiffCtrl(mode);
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

void cbDiffEditor::CreateDiffToolButtons(int viewmode)
{
    BBTable = new wxBitmapButton(this, ID_BUTTON_TABLE, wxGetBitmapFromMemory(table), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBTable->SetToolTip(_("Display as a table"));

    BBUnified = new wxBitmapButton(this, ID_BUTTON_UNIFIED, wxGetBitmapFromMemory(unified), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBUnified->SetToolTip(_("Display as unified diff"));

    BBSideBySide = new wxBitmapButton(this, ID_BUTTON_SIDEBYSIDE, wxGetBitmapFromMemory(sidebyside), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBSideBySide->SetToolTip(_("Display side by side"));

    wxBitmapButton* BBReload = new wxBitmapButton(this, ID_BBRELOAD, wxGetBitmapFromMemory(reload), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBReload->SetToolTip(_("Reload files"));

    wxBitmapButton* BBSwap = new wxBitmapButton(this, ID_BBSWAP, wxGetBitmapFromMemory(swap), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBSwap->SetToolTip(_("Swap files"));

    wxBoxSizer* boxsizer = new wxBoxSizer(wxHORIZONTAL);
    boxsizer->Add(BBTable, 0, wxALL, 5);
    boxsizer->Add(BBUnified, 0, wxALL, 5);
    boxsizer->Add(BBSideBySide, 0, wxALL, 5);
    boxsizer->Add(-1,-1,0, wxALL, 5);
    boxsizer->Add(BBReload, 0, wxALL, 5);
    boxsizer->Add(BBSwap, 0, wxALL, 5);
    boxsizer->Add(-1,-1,0, wxALL, 5);

    GetSizer()->Add(boxsizer);

    switch (viewmode)
    {
    case cbDiffEditor::TABLE:
        BBTable->Enable(false);
        break;
    case cbDiffEditor::UNIFIED:
        BBUnified->Enable(false);
        break;
    case cbDiffEditor::SIDEBYSIDE:
        BBSideBySide->Enable(false);
        break;
    default:
        break;
    }

    Connect(wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&cbDiffEditor::OnToolButton);
}

void cbDiffEditor::OnToolButton(wxCommandEvent& event)
{
    if(event.GetId() == ID_BBSWAP)
    {
        Swap();
        return;
    }
    else
    {
        if (event.GetId() == ID_BUTTON_TABLE)
        {
            if ( GetMode() != cbDiffEditor::TABLE)
            {
                SetMode(cbDiffEditor::TABLE);
                BBTable->Enable(false);
		        BBUnified->Enable();
		        BBSideBySide->Enable();
            }
        }
        else if (event.GetId() == ID_BUTTON_UNIFIED)
        {
            if ( GetMode() != cbDiffEditor::UNIFIED)
            {
                SetMode(cbDiffEditor::UNIFIED);
		        BBTable->Enable();
                BBUnified->Enable(false);
		        BBSideBySide->Enable();
            }
        }
        else if (event.GetId() == ID_BUTTON_SIDEBYSIDE)
        {
            if ( GetMode() != cbDiffEditor::SIDEBYSIDE)
            {
                SetMode(cbDiffEditor::SIDEBYSIDE);
		        BBTable->Enable();
		        BBUnified->Enable();
                BBSideBySide->Enable(false);
            }
        }
    }

    Reload();
}

