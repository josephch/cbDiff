#include "cbTableCtrl.h"

#include <manager.h>
#include <editormanager.h>
#include <configmanager.h>
#include <logmanager.h>
#include <cbstyledtextctrl.h>
#include <cbeditor.h>
#include <wx/textfile.h>

cbTableCtrl::cbTableCtrl(cbDiffEditor *parent):
    cbDiffCtrl(parent),
    lineNumbersWidthRight(0)
{
    wxBoxSizer *BoxSizer = new wxBoxSizer(wxHORIZONTAL);
    m_txtctrl = new cbStyledTextCtrl(this, wxID_ANY);
    BoxSizer->Add(m_txtctrl, 1,wxEXPAND, 0);
    SetSizer(BoxSizer);
}

cbTableCtrl::~cbTableCtrl()
{
    Disconnect( m_txtctrl->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbTableCtrl::OnEditorChange));
}

void cbTableCtrl::Init(cbDiffColors colset, bool, bool rightReadOnly)
{
    Disconnect( m_txtctrl->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbTableCtrl::OnEditorChange));

    leftReadOnly_ = true;
    rightReadOnly_ = rightReadOnly;

    linesWithDifferences_.clear();

    wxColor marbkg = m_txtctrl->StyleGetBackground(wxSCI_STYLE_LINENUMBER);

    cbEditor::ApplyStyles(m_txtctrl);
    //cbEditor::ApplyStyles(m_hiddenctrl);
    m_txtctrl->SetMarginType(0, wxSCI_MARGIN_NUMBER);
    m_txtctrl->SetMarginWidth(1, 16);
    m_txtctrl->SetMarginType(1, wxSCI_MARGIN_SYMBOL);
//    m_txtctrl->SetMarginType(2, wxSCI_MARGIN_RTEXT); line number of left file
//    m_txtctrl->SetMarginWidth(3, 16);
//    m_txtctrl->SetMarginType(3, wxSCI_MARGIN_SYMBOL); removed from left symbol
    m_txtctrl->SetMarginWidth(2, 0);
    m_txtctrl->SetMarginWidth(3, 0);
    m_txtctrl->MarkerDefine(PLUS_MARKER, wxSCI_MARK_PLUS, colset.m_addedlines, colset.m_addedlines);
    m_txtctrl->MarkerDefine(EQUAL_MARKER, wxSCI_MARK_CHARACTER + 61, *wxWHITE, marbkg);
    m_txtctrl->MarkerDefine(GREEN_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.m_addedlines, colset.m_addedlines);
    m_txtctrl->MarkerSetAlpha(GREEN_BKG_MARKER, colset.m_addedlines.Alpha());
//    m_txtctrl->MarkerDefine(MINUS_MARKER, wxSCI_MARK_MINUS, colset.m_removedlines, colset.m_removedlines);
//    m_txtctrl->MarkerDefine(RED_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.m_removedlines, colset.m_removedlines);
//    m_txtctrl->MarkerSetAlpha(RED_BKG_MARKER, colset.m_removedlines.Alpha());

    const auto lang = colset.m_hlang;
    const bool isC = lang == "C/C++";
    m_theme->Apply(m_theme->GetHighlightLanguage(lang), m_txtctrl, isC, true);
}

void cbTableCtrl::ShowDiff(wxDiff diff)
{
    Disconnect( m_txtctrl->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbTableCtrl::OnEditorChange));

    std::map<long, int> right_added  = diff.GetAddedLines();
    std::map<long, int> left_removed = diff.GetRemovedLines();
    std::map<long, long> line_pos    = diff.GetLinePositions();

    rightFilename_ = diff.GetToFilename();
    leftFilename_ = diff.GetFromFilename();

    m_txtctrl->SetReadOnly(false);
    m_txtctrl->ClearAll();
    m_txtctrl->LoadFile(diff.GetToFilename());
    m_txtctrl->AnnotationClearAll();
    m_txtctrl->AnnotationSetVisible(wxSCI_ANNOTATION_STANDARD);
    wxTextFile tff(diff.GetFromFilename());
    tff.Open();

    for(auto itr = right_added.begin() ; itr != right_added.end() ; ++itr)
    {
        long right_line = itr->first;
        unsigned int len = itr->second;
        for(unsigned int k = 0 ; k < len ; ++k)
        {
            m_txtctrl->MarkerAdd(right_line+k, PLUS_MARKER);
            m_txtctrl->MarkerAdd(right_line+k, GREEN_BKG_MARKER);
        }
        linesWithDifferences_.push_back(right_line);
    }
    for(auto itr = left_removed.begin() ; itr != left_removed.end() ; ++itr)
    {
        long left_line = itr->first;
        unsigned int removed = itr->second;
        if(line_pos.find(left_line) != line_pos.end())
        {
            int added = 0;
            long right_line = line_pos[left_line];
            if(right_added.find(right_line) != right_added.end())
                added = right_added[right_line];

            wxString annotationStr = wxEmptyString;
            std::vector<char> annotationStyles;
            for(size_t k = 0 ; k < removed ; ++k)
            {
                const wxString &lineText = tff.GetLine(left_line+k) + '\n';
                annotationStr += lineText;
            }
            annotationStr.RemoveLast();// remove last newline
            int annotLine = right_line+added-1 > 0 ? right_line+added-1 : 0;
            m_txtctrl->AnnotationSetText(annotLine, annotationStr);

            auto i = std::lower_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), right_line);
            if (i == linesWithDifferences_.end() || right_line < *i)
                linesWithDifferences_.insert(i, right_line);
        }
    }

    if(rightReadOnly_)
        m_txtctrl->SetReadOnly(true);
    setLineNumberMarginWidth();

    Connect( m_txtctrl->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbTableCtrl::OnEditorChange));
}

bool cbTableCtrl::GetModified() const
{
    return m_txtctrl->GetModify();
}

bool cbTableCtrl::QueryClose()
{
    if(!m_txtctrl->GetModify())
        return true;

    int answer = wxMessageBox("Save File?", "Confirm", wxYES_NO | wxCANCEL);
    if (answer == wxCANCEL)
        return false;
    else if (answer == wxYES)
        return Save();
    else
        m_txtctrl->SetSavePoint();

    parent_->updateTitle();
    return true;
}

bool cbTableCtrl::Save()
{
    if(m_txtctrl->GetModify())
    {
        if(!m_txtctrl->SaveFile(rightFilename_))
            return false;
        else
            parent_->updateTitle();
    }
    return true;
}

bool cbTableCtrl::RightModified()
{
    return m_txtctrl->GetModify();
}

void cbTableCtrl::setLineNumberMarginWidth()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("editor"));
    int pixelWidth = m_txtctrl->TextWidth(wxSCI_STYLE_LINENUMBER, _T("9"));
    if (cfg->ReadBool(_T("/margin/dynamic_width"), false))
    {
        int lineNumChars = 1;
        int lineCount = m_txtctrl->GetLineCount();
        while (lineCount >= 10)
        {
            lineCount /= 10;
            ++lineNumChars;
        }

        int lineNumWidth = lineNumChars * pixelWidth + pixelWidth * 0.75;
        if (lineNumWidth != lineNumbersWidthRight)
        {
            m_txtctrl->SetMarginWidth(0, lineNumWidth);
            lineNumbersWidthRight = lineNumWidth;
        }
    }
    else
        m_txtctrl->SetMarginWidth(0, pixelWidth * 0.75 + cfg->ReadInt(_T("/margin/width_chars"), 6) * pixelWidth);
}

void cbTableCtrl::OnEditorChange(wxScintillaEvent &event)
{
    parent_->updateTitle();
}

void cbTableCtrl::Undo()
{
    if(!rightReadOnly_)
        m_txtctrl->Undo();
}

void cbTableCtrl::Redo()
{
    if(!rightReadOnly_)
        m_txtctrl->Redo();
}

void cbTableCtrl::ClearHistory()
{
    m_txtctrl->EmptyUndoBuffer();
}

void cbTableCtrl::Cut()
{
    if(!rightReadOnly_)
        m_txtctrl->Cut();
}

void cbTableCtrl::Copy()
{
    m_txtctrl->Copy();
}

void cbTableCtrl::Paste()
{
    if(!rightReadOnly_)
        m_txtctrl->Paste();
}

bool cbTableCtrl::CanUndo() const
{
    return !rightReadOnly_ && m_txtctrl->CanUndo();
}

bool cbTableCtrl::CanRedo() const
{
    return !rightReadOnly_ && m_txtctrl->CanRedo();
}

bool cbTableCtrl::HasSelection() const
{
    return m_txtctrl->GetSelectionStart() != m_txtctrl->GetSelectionEnd();
}

bool cbTableCtrl::CanPaste() const
{
    if (platform::gtk)
        return !rightReadOnly_;

    return m_txtctrl->CanPaste() && !rightReadOnly_;
}

bool cbTableCtrl::CanSelectAll() const
{
    return m_txtctrl->GetLength() > 0;
}

void cbTableCtrl::SelectAll()
{
    m_txtctrl->SelectAll();
}

void cbTableCtrl::NextDifference()
{
    if(linesWithDifferences_.empty())
        return;

    auto i = std::upper_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), m_txtctrl->GetCurrentLine());

    if(i == linesWithDifferences_.end())
        return;

    m_txtctrl->GotoLine(*i);
}

bool cbTableCtrl::CanGotoNextDiff()
{
    if(linesWithDifferences_.empty())
        return false;

    auto i = std::upper_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), m_txtctrl->GetCurrentLine());

    return i != linesWithDifferences_.end();
}

void cbTableCtrl::PrevDifference()
{
    if(linesWithDifferences_.empty())
        return;

    auto i = std::lower_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), m_txtctrl->GetCurrentLine());

    if(i == linesWithDifferences_.begin())
        return;

    m_txtctrl->GotoLine(*(--i));
}

bool cbTableCtrl::CanGotoPrevDiff()
{
    if(linesWithDifferences_.empty())
        return false;

    auto i = std::lower_bound(linesWithDifferences_.begin(), linesWithDifferences_.end(), m_txtctrl->GetCurrentLine());

    return i != linesWithDifferences_.begin();
}

