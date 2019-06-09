#include "cbUnifiedCtrl.h"

#include <manager.h>
#include <editormanager.h>
#include <configmanager.h>
#include <logmanager.h>
#include <cbstyledtextctrl.h>
#include <cbeditor.h>

cbUnifiedCtrl::cbUnifiedCtrl(cbDiffEditor* parent):
    cbDiffCtrl(parent)
{
    wxBoxSizer* BoxSizer = new wxBoxSizer(wxHORIZONTAL);
    txtctrl_ = new cbStyledTextCtrl(this, wxID_ANY);
    BoxSizer->Add(txtctrl_, 1, wxEXPAND, 0);
    SetSizer(BoxSizer);
}

void cbUnifiedCtrl::Init(cbDiffColors colset)
{
    leftReadOnly_ = true;
    rightReadOnly_ = true;

    cbEditor::ApplyStyles(txtctrl_);
    txtctrl_->SetMargins(0,0);
    txtctrl_->SetMarginWidth(0,0);    // don't show linenumber
    txtctrl_->SetMarginWidth(1,0);    // don't show +-= symbol margin
    txtctrl_->SetMarginWidth(2,0);    // to hide the change and fold
    txtctrl_->SetMarginWidth(3,0);    // margin made by cbEditor::ApplyStyles
    theme_->Apply(_T("DiffPatch"), txtctrl_, false, true);
}

void cbUnifiedCtrl::ShowDiff(const wxDiff &diff)
{
    txtctrl_->SetReadOnly(false);
    txtctrl_->ClearAll();
    txtctrl_->AppendText(diff.GetDiff());
    txtctrl_->SetReadOnly(true);
}

void cbUnifiedCtrl::UpdateDiff(const wxDiff &diff)
{
    ShowDiff(diff);
}

void cbUnifiedCtrl::Copy()
{
    txtctrl_->Copy();
}

bool cbUnifiedCtrl::HasSelection() const
{
    return txtctrl_->GetSelectionStart() != txtctrl_->GetSelectionEnd();
}

bool cbUnifiedCtrl::CanSelectAll() const
{
    return txtctrl_->GetLength() > 0;
}

void cbUnifiedCtrl::SelectAll()
{
    txtctrl_->SelectAll();
}

