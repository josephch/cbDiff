#include "cbDiffEditor.h"

#include <manager.h>
#include <editormanager.h>
#include <configmanager.h>
#include <logmanager.h>
#include <cbeditor.h>
#include <cbauibook.h>

#include "wxDiff.h"

/// diffctrls
#include "cbTableCtrl.h"
#include "cbUnifiedCtrl.h"
#include "cbSideBySideCtrl.h"

//! static Editor set
cbDiffEditor::EditorsSet cbDiffEditor::allEditors_;

BEGIN_EVENT_TABLE(cbDiffEditor,EditorBase)
    EVT_CONTEXT_MENU(cbDiffEditor::OnContextMenu)
END_EVENT_TABLE()

cbDiffEditor::cbDiffEditor(const wxString &leftFile, const wxString &rightFile, int viewmode, bool leftReadOnly, bool rightReadOnly):
    EditorBase((wxWindow*)Manager::Get()->GetEditorManager()->GetNotebook(), leftFile + rightFile),
    diffctrl_(0),
    leftFile_(leftFile),
    rightFile_(rightFile),
    leftReadOnly_(leftReadOnly),
    rightReadOnly_(rightReadOnly)
{
    colorset_.addedlines_ = wxColour(0,255,0,50);
    colorset_.removedlines_ = wxColour(255,0,0,50);
    colorset_.selectedlines_ = wxColour(0,0,255,50);
    colorset_.caretlinetype_ = 0;
    colorset_.caretline_ = wxColor(122,122,0);

    ConfigManager *cfg = Manager::Get()->GetConfigManager(_T("cbdiffsettings"));
    if (cfg)
    {
        wxColour add = cfg->ReadColour(_T("addedlines"), wxColour(0,255,0,50));
        int addalpha = cfg->ReadInt(_T("addedlinesalpha"), 50);
        wxColour rem = cfg->ReadColour(_T("removedlines"), wxColour(255,0,0,50));
        int remalpha = cfg->ReadInt(_T("removedlinesalpha"), 50);
        wxColour sel = cfg->ReadColour(_T("selectedlines"), wxColour(0,0,255,50));
        int selalpha = cfg->ReadInt(_T("selectedlinesalpha"), 50);
        colorset_.caretlinetype_ = cfg->ReadInt(_T("caretlinetype"));
        wxColour car = cfg->ReadColour(_T("caretline"), wxColor(122,122,0));
        int caralpha = cfg->ReadInt(_T("caretlinealpha"), 50);
        colorset_.addedlines_ = wxColour(add.Red(), add.Green(), add.Blue(), addalpha);
        colorset_.removedlines_ = wxColour(rem.Red(), rem.Green(), rem.Blue(), remalpha);
        colorset_.selectedlines_ = wxColour(sel.Red(), sel.Green(), sel.Blue(), selalpha);
        colorset_.caretline_ = wxColour(car.Red(), car.Green(), car.Blue(), caralpha);

        if(viewmode == DEFAULT)
            viewmode = cfg->ReadInt(_T("viewmode"), 0) + TABLE;
    }
    HighlightLanguage hl = Manager::Get()->GetEditorManager()->GetColourSet()->GetLanguageForFilename(leftFile_);
    if (hl != HL_NONE)
        colorset_.hlang_ = Manager::Get()->GetEditorManager()->GetColourSet()->GetLanguageName(hl);

    wxBoxSizer* BoxSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(BoxSizer);
    InitDiffCtrl(viewmode);

    allEditors_.insert(this);

    Reload();

    BoxSizer->Layout();
    Layout();
}

cbDiffEditor::~cbDiffEditor()
{
    allEditors_.erase(this);
}

void cbDiffEditor::ShowDiff()
{
    /* Diff creation */
    std::vector<std::string> *leftElems = nullptr;
    if(diffctrl_->LeftModified())
        leftElems = diffctrl_->GetLeftLines();
    std::vector<std::string> *rightElems = nullptr;
    if(diffctrl_->RightModified())
        rightElems = diffctrl_->GetRightLines();
    wxDiff diff(leftFile_, rightFile_, leftReadOnly_, rightReadOnly_, leftElems, rightElems);
    updateTitle();

    wxString different = diff.IsDifferent();
    if(different != wxEmptyString)
        cbMessageBox(different, _("cbDiff"));

    diff_ = diff.GetDiff();

    if(leftElems || rightElems)
        diffctrl_->UpdateDiff(diff);
    else
        diffctrl_->ShowDiff(diff);
    diffctrl_->Layout();

    if(leftElems) delete leftElems;
    if(rightElems) delete rightElems;
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

    if(!cbSaveToFile(Filename, diff_))
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
    wxString temp = leftFile_;
    leftFile_ = rightFile_;
    rightFile_ = temp;
    bool tmp_ro = leftReadOnly_;
    leftReadOnly_ = rightReadOnly_;
    rightReadOnly_ = tmp_ro;
    Reload();
}

void cbDiffEditor::Reload()
{
    if(!leftFile_.IsEmpty() && !rightFile_.IsEmpty())
        ShowDiff();
}

int cbDiffEditor::GetMode()
{
    return viewingmode_;
}

void cbDiffEditor::InitDiffCtrl(int mode)
{
    assert(diffctrl_ == nullptr);

    if(mode == TABLE)
        diffctrl_ = new cbTableCtrl(this);
    else if(mode == UNIFIED)
        diffctrl_ = new cbUnifiedCtrl(this);
    else if(mode == SIDEBYSIDE)
        diffctrl_ = new cbSideBySideCtrl(this);

    GetSizer()->Add(diffctrl_, 1, wxEXPAND, 5);
    diffctrl_->Init(colorset_);
    viewingmode_ = mode;

    GetSizer()->Layout();
}

void cbDiffEditor::SetMode(int mode)
{
    if(viewingmode_ == mode)
        return;
    if(diffctrl_)
    {
        GetSizer()->Detach(diffctrl_);
        wxDELETE(diffctrl_);
    }
    InitDiffCtrl(mode);
}

void cbDiffEditor::CloseAllEditors()
{
    EditorsSet s = allEditors_;
    for (EditorsSet::iterator i = s.begin(); i != s.end(); ++i )
    {
        EditorManager::Get()->QueryClose(*i);
        (*i)->Close();
    }
    assert(allEditors_.empty());
}

bool cbDiffEditor::GetAnyModified() const
{
    return diffctrl_->LeftModified() || diffctrl_->RightModified();
}

bool cbDiffEditor::GetModified() const
{
    return diffctrl_->GetModified();
}

bool cbDiffEditor::QueryClose()
{
    return diffctrl_->QueryClose();
}

bool cbDiffEditor::Save()
{
    return diffctrl_->Save();
}

void cbDiffEditor::updateTitle()
{
    //SetTitle(...) calls Manager::Get()->GetEditorManager() which can fail during shutdown
    if(!Manager::Get()->IsAppShuttingDown())
        SetTitle(_T("Diff: ") +
                 (diffctrl_->LeftModified() ? _("*") : _("")) + wxFileNameFromPath(leftFile_) +
                 _T(" ") +
                 (diffctrl_->RightModified() ? _("*") : _("")) + wxFileNameFromPath(rightFile_));
}

void cbDiffEditor::Undo()                      {diffctrl_->Undo();}
void cbDiffEditor::Redo()                      {diffctrl_->Redo();}
void cbDiffEditor::ClearHistory()              {diffctrl_->ClearHistory();}
void cbDiffEditor::Cut()                       {diffctrl_->Cut();}
void cbDiffEditor::Copy()                      {diffctrl_->Copy();}
void cbDiffEditor::Paste()                     {diffctrl_->Paste();}
bool cbDiffEditor::CanUndo() const      {return diffctrl_->CanUndo();}
bool cbDiffEditor::CanRedo() const      {return diffctrl_->CanRedo();}
bool cbDiffEditor::HasSelection() const {return diffctrl_->HasSelection();}
bool cbDiffEditor::CanPaste() const     {return diffctrl_->CanPaste();}
bool cbDiffEditor::CanSelectAll() const {return diffctrl_->CanSelectAll();}
void cbDiffEditor::SelectAll()                 {diffctrl_->SelectAll();}

void cbDiffEditor::NextDifference()    {       diffctrl_->NextDifference();    }
bool cbDiffEditor::CanGotoNextDiff()   {return diffctrl_->CanGotoNextDiff();   }
void cbDiffEditor::PrevDifference()    {       diffctrl_->PrevDifference();    }
bool cbDiffEditor::CanGotoPrevDiff()   {return diffctrl_->CanGotoPrevDiff();   }
void cbDiffEditor::FirstDifference()   {       diffctrl_->FirstDifference();   }
bool cbDiffEditor::CanGotoFirstDiff()  {return diffctrl_->CanGotoFirstDiff();  }
void cbDiffEditor::LastDifference()    {       diffctrl_->LastDifference();    }
bool cbDiffEditor::CanGotoLastDiff()   {return diffctrl_->CanGotoLastDiff();   }

void cbDiffEditor::CopyToLeft()        {       diffctrl_->CopyToLeft();        }
bool cbDiffEditor::CanCopyToLeft()     {return diffctrl_->CanCopyToLeft();     }
void cbDiffEditor::CopyToRight()       {       diffctrl_->CopyToRight();       }
bool cbDiffEditor::CanCopyToRight()    {return diffctrl_->CanCopyToRight();    }
void cbDiffEditor::CopyToLeftNext()    {       diffctrl_->CopyToLeftNext();    }
bool cbDiffEditor::CanCopyToLeftNext() {return diffctrl_->CanCopyToLeftNext(); }
void cbDiffEditor::CopyToRightNext()   {       diffctrl_->CopyToRightNext();   }
bool cbDiffEditor::CanCopyToRightNext(){return diffctrl_->CanCopyToRightNext();}

