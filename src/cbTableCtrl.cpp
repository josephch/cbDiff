#include "cbTableCtrl.h"

#include <algorithm>

#include <manager.h>
#include <editormanager.h>
#include <configmanager.h>
#include <logmanager.h>
#include <cbstyledtextctrl.h>
#include <cbeditor.h>
#include <wx/textfile.h>

cbTableCtrl::cbTableCtrl(cbDiffEditor *parent):
    cbDiffCtrl(parent)
{
    wxBoxSizer *BoxSizer = new wxBoxSizer(wxHORIZONTAL);
    txtctrl_ = new cbStyledTextCtrl(this, wxID_ANY);
    BoxSizer->Add(txtctrl_, 1, wxEXPAND, 0);
    SetSizer(BoxSizer);
}

void cbTableCtrl::Init(cbDiffColors colset)
{
    linesWithDifferences_.clear();

    wxColor marbkg = txtctrl_->StyleGetBackground(wxSCI_STYLE_LINENUMBER);
    int width = 20 * txtctrl_->TextWidth(wxSCI_STYLE_LINENUMBER, _T("9"));

    cbEditor::ApplyStyles(txtctrl_);
    txtctrl_->SetMargins(0, 0);
    txtctrl_->SetMarginWidth(0, width);
    txtctrl_->SetMarginType(0, wxSCI_MARGIN_RTEXT);
    txtctrl_->SetMarginWidth(1, 0);
    txtctrl_->SetMarginWidth(2, 0);
    txtctrl_->SetMarginWidth(3, 0);
    txtctrl_->MarkerDefine(RED_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.removedlines_, colset.removedlines_);
    txtctrl_->MarkerSetAlpha(RED_BKG_MARKER, colset.removedlines_.Alpha());
    txtctrl_->MarkerDefine(GREEN_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.addedlines_, colset.addedlines_);
    txtctrl_->MarkerSetAlpha(GREEN_BKG_MARKER, colset.addedlines_.Alpha());

    const auto lang = colset.hlang_;
    const bool isC = lang == "C/C++";
    theme_->Apply(theme_->GetHighlightLanguage(lang), txtctrl_, isC, true);
}

void cbTableCtrl::ShowDiff(wxDiff diff)
{
    std::map<long, int> right_added  = diff.GetAddedLines();
    std::map<long, int> left_removed = diff.GetRemovedLines();
    std::map<long, int> left_empty   = diff.GetLeftEmptyLines();

    rightFilename_ = diff.GetRightFilename();
    leftFilename_ = diff.GetLeftFilename();
    leftReadOnly_ = true;
    rightReadOnly_ = diff.RightReadOnly();

    txtctrl_->SetReadOnly(false);
    int cursorPos = txtctrl_->GetCurrentPos();
    txtctrl_->ClearAll();
    wxTextFile left_text_file(diff.GetLeftFilename());
    wxTextFile right_text_file(diff.GetRightFilename());
    left_text_file.Open();
    right_text_file.Open();
    bool refillleft = false;
    bool refillright = false;
    int left_empty_lines=0;
    for(auto itr = left_empty.begin() ; itr != left_empty.end() ; ++itr)
        left_empty_lines += itr->second;
    wxString strleft = left_text_file.GetFirstLine();
    wxString strright = right_text_file.GetFirstLine();
    int linecount = left_text_file.GetLineCount() + left_empty_lines;
    int linenumberleft = 0;
    int linenumberright = 0;
    int removed = 0;
    int added = 0;

    for (int i = 0; i < linecount; ++i)
    {
        if(refillleft && !left_text_file.Eof())
            strleft = left_text_file.GetNextLine();
        if(refillright && !right_text_file.Eof())
            strright = right_text_file.GetNextLine();
        auto litr = left_removed.find(linenumberleft);
        if(litr != left_removed.end() || removed)
        {
            if(!removed)
            {
                linesWithDifferences_.push_back(i);
                removed = litr->second;
            }
            --removed;
            txtctrl_->AppendText(strleft + _T("\n"));
            refillleft = true;
            refillright = false;
            ++linenumberleft;
            txtctrl_->MarginSetStyle(i, wxSCI_STYLE_LINENUMBER);
            txtctrl_->MarginSetText(i, wxString::Format(_T("%9d%9c"), linenumberleft, ' '));
            txtctrl_->MarkerAdd(i, RED_BKG_MARKER);
            continue;
        }
        auto ritr = right_added.find(linenumberright);
        if(ritr != right_added.end() || added)
        {
            if(!added)
            {
                linesWithDifferences_.push_back(i);
                added = ritr->second;
            }
            --added;
            txtctrl_->AppendText(strright + _T("\n"));
            refillleft = false;
            refillright = true;
            ++linenumberright;
            txtctrl_->MarginSetStyle(i, wxSCI_STYLE_LINENUMBER);
            txtctrl_->MarginSetText(i, wxString::Format(_T("%9d"), linenumberright));
            txtctrl_->MarkerAdd(i, GREEN_BKG_MARKER);
            continue;
        }
        txtctrl_->AppendText(strleft + _T("\n"));
        refillleft = true;
        refillright = true;
        ++linenumberleft;
        ++linenumberright;
        txtctrl_->MarginSetStyle(i, wxSCI_STYLE_LINENUMBER);
        txtctrl_->MarginSetText(i, wxString::Format(_T("%9d%9d"), linenumberleft, linenumberright));
    }
    txtctrl_->SetReadOnly(true);
    txtctrl_->GotoPos(cursorPos);
}

void cbTableCtrl::Copy()
{
    txtctrl_->Copy();
}

bool cbTableCtrl::HasSelection() const
{
    return txtctrl_->GetSelectionStart() != txtctrl_->GetSelectionEnd();
}

void cbTableCtrl::NextDifference()
{
    if(linesWithDifferences_.empty()) return;

    auto i = std::upper_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), txtctrl_->GetCurrentLine());

    if(i == linesWithDifferences_.end()) return;

    txtctrl_->GotoLine(*i);
}

bool cbTableCtrl::CanGotoNextDiff()
{
    if(linesWithDifferences_.empty()) return false;

    auto i = std::upper_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), txtctrl_->GetCurrentLine());

    return i != linesWithDifferences_.end();
}

void cbTableCtrl::PrevDifference()
{
    if(linesWithDifferences_.empty()) return;

    auto i = std::lower_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), txtctrl_->GetCurrentLine());

    if(i == linesWithDifferences_.begin()) return;

    txtctrl_->GotoLine(*(--i));
}

bool cbTableCtrl::CanGotoPrevDiff()
{
    if(linesWithDifferences_.empty()) return false;

    auto i = std::lower_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), txtctrl_->GetCurrentLine());

    return i != linesWithDifferences_.begin();
}

void cbTableCtrl::FirstDifference()
{
    if(linesWithDifferences_.empty()) return;

    txtctrl_->GotoLine(linesWithDifferences_.front());
}

void cbTableCtrl::LastDifference()
{
   if(linesWithDifferences_.empty()) return;

    txtctrl_->GotoLine(linesWithDifferences_.back());
}

bool cbTableCtrl::CanGotoFirstDiff()
{
    if(linesWithDifferences_.empty())
        return false;

    return txtctrl_->GetCurrentLine() != linesWithDifferences_.front();
}

bool cbTableCtrl::CanGotoLastDiff()
{

    if(linesWithDifferences_.empty())
        return false;

    return txtctrl_->GetCurrentLine() != linesWithDifferences_.back();
}
