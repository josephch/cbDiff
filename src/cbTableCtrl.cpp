#include "cbTableCtrl.h"

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
    m_txtctrl = new cbStyledTextCtrl(this, wxID_ANY);
    BoxSizer->Add(m_txtctrl, 1, wxEXPAND, 0);
    SetSizer(BoxSizer);
}

void cbTableCtrl::Init(cbDiffColors colset)
{
    linesWithDifferences_.clear();

    wxColor marbkg = m_txtctrl->StyleGetBackground(wxSCI_STYLE_LINENUMBER);
    int width = 20 * m_txtctrl->TextWidth(wxSCI_STYLE_LINENUMBER, _T("9"));

    cbEditor::ApplyStyles(m_txtctrl);
    m_txtctrl->SetMargins(0, 0);
    m_txtctrl->SetMarginWidth(0, width);
    m_txtctrl->SetMarginType(0, wxSCI_MARGIN_RTEXT);
    m_txtctrl->SetMarginWidth(1, 0);
    m_txtctrl->SetMarginWidth(2, 0);
    m_txtctrl->SetMarginWidth(3, 0);
    m_txtctrl->MarkerDefine(RED_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.m_removedlines, colset.m_removedlines);
    m_txtctrl->MarkerSetAlpha(RED_BKG_MARKER, colset.m_removedlines.Alpha());
    m_txtctrl->MarkerDefine(GREEN_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.m_addedlines, colset.m_addedlines);
    m_txtctrl->MarkerSetAlpha(GREEN_BKG_MARKER, colset.m_addedlines.Alpha());

    const auto lang = colset.m_hlang;
    const bool isC = lang == "C/C++";
    m_theme->Apply(m_theme->GetHighlightLanguage(lang), m_txtctrl, isC, true);
}

void cbTableCtrl::ShowDiff(wxDiff diff)
{
    std::map<long, int> right_added  = diff.GetAddedLines();
    std::map<long, int> left_removed = diff.GetRemovedLines();
    std::map<long, int> left_empty   = diff.GetLeftEmptyLines();
    //std::map<long, long> line_pos    = diff.GetLinePositions();

    rightFilename_ = diff.GetRightFilename();
    leftFilename_ = diff.GetLeftFilename();
    leftReadOnly_ = true;
    rightReadOnly_ = diff.RightReadOnly();

    m_txtctrl->SetReadOnly(false);
    int cursorPos = m_txtctrl->GetCurrentPos();
    m_txtctrl->ClearAll();
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
            m_txtctrl->AppendText(strleft + _T("\n"));
            refillleft = true;
            refillright = false;
            ++linenumberleft;
            m_txtctrl->MarginSetStyle(i, wxSCI_STYLE_LINENUMBER);
            m_txtctrl->MarginSetText(i, wxString::Format(_T("%9d%9c"), linenumberleft, ' '));
            m_txtctrl->MarkerAdd(i, RED_BKG_MARKER);
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
            m_txtctrl->AppendText(strright + _T("\n"));
            refillleft = false;
            refillright = true;
            ++linenumberright;
            m_txtctrl->MarginSetStyle(i, wxSCI_STYLE_LINENUMBER);
            m_txtctrl->MarginSetText(i, wxString::Format(_T("%9d"), linenumberright));
            m_txtctrl->MarkerAdd(i, GREEN_BKG_MARKER);
            continue;
        }
        m_txtctrl->AppendText(strleft + _T("\n"));
        refillleft = true;
        refillright = true;
        ++linenumberleft;
        ++linenumberright;
        m_txtctrl->MarginSetStyle(i, wxSCI_STYLE_LINENUMBER);
        m_txtctrl->MarginSetText(i, wxString::Format(_T("%9d%9d"), linenumberleft, linenumberright));
    }
    m_txtctrl->SetReadOnly(true);
    m_txtctrl->GotoPos(cursorPos);
}

void cbTableCtrl::Copy()
{
    m_txtctrl->Copy();
}

bool cbTableCtrl::HasSelection() const
{
    return m_txtctrl->GetSelectionStart() != m_txtctrl->GetSelectionEnd();
}

void cbTableCtrl::NextDifference()
{
    if(linesWithDifferences_.empty()) return;

    auto i = std::upper_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), m_txtctrl->GetCurrentLine());

    if(i == linesWithDifferences_.end()) return;

    m_txtctrl->GotoLine(*i);
}

bool cbTableCtrl::CanGotoNextDiff()
{
    if(linesWithDifferences_.empty()) return false;

    auto i = std::upper_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), m_txtctrl->GetCurrentLine());

    return i != linesWithDifferences_.end();
}

void cbTableCtrl::PrevDifference()
{
    if(linesWithDifferences_.empty()) return;

    auto i = std::lower_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), m_txtctrl->GetCurrentLine());

    if(i == linesWithDifferences_.begin()) return;

    m_txtctrl->GotoLine(*(--i));
}

bool cbTableCtrl::CanGotoPrevDiff()
{
    if(linesWithDifferences_.empty()) return false;

    auto i = std::lower_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), m_txtctrl->GetCurrentLine());

    return i != linesWithDifferences_.begin();
}

void cbTableCtrl::FirstDifference()
{
    if(linesWithDifferences_.empty()) return;

    m_txtctrl->GotoLine(linesWithDifferences_.front());
}

void cbTableCtrl::LastDifference()
{
   if(linesWithDifferences_.empty()) return;

    m_txtctrl->GotoLine(linesWithDifferences_.back());
}

bool cbTableCtrl::CanGotoFirstDiff()
{
    if(linesWithDifferences_.empty())
        return false;

    return m_txtctrl->GetCurrentLine() != linesWithDifferences_.front();
}

bool cbTableCtrl::CanGotoLastDiff()
{

    if(linesWithDifferences_.empty())
        return false;

    return m_txtctrl->GetCurrentLine() != linesWithDifferences_.back();
}
