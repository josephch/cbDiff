#include "cbSideBySideCtrl.h"

#include <manager.h>
#include <editormanager.h>
#include <configmanager.h>
#include <logmanager.h>
#include <cbstyledtextctrl.h>
#include <cbeditor.h>

#include <wx/textfile.h>
#include <wx/splitter.h>
#include "cbDiffEditor.h"

/**
    We need to create this timer,
    because if we use the Scintilla events
    the graphic get messed up.
    And its easier to handle all the
    synchronisation stuff in one function.
    Trust me or try it self. ;-)
**/
class LineChangedTimer : public wxTimer
{
    cbSideBySideCtrl *m_pane;
public:
    LineChangedTimer(cbSideBySideCtrl *pane) : wxTimer()
    {
        m_pane = pane;
    }
    void Notify()
    {
        m_pane->Synchronize();
    }
    void start()
    {
        wxTimer::Start(20);
    }
};

BEGIN_EVENT_TABLE(cbSideBySideCtrl, cbDiffCtrl)
END_EVENT_TABLE()

cbSideBySideCtrl::cbSideBySideCtrl(cbDiffEditor *parent):
    cbDiffCtrl(parent),
    lastSyncedLine_(-1),
    lastSyncedLHandle(-1),
    lastSyncedRHandle(-1),
    lineNumbersWidthLeft(0),
    lineNumbersWidthRight(0)
{
    wxBoxSizer *VBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *HBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    VScrollBar = new wxScrollBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
    HScrollBar = new wxScrollBar(this, wxID_ANY);
    wxSplitterWindow *splitWindow = new wxSplitterWindow(this, wxID_ANY);
    TCLeft = new cbStyledTextCtrl(splitWindow, wxID_ANY);
    TCRight = new cbStyledTextCtrl(splitWindow, wxID_ANY);
    splitWindow->SplitVertically(TCLeft, TCRight);
    HBoxSizer->Add(splitWindow, 1, wxEXPAND, 0);
    HBoxSizer->Add(VScrollBar, 0, wxEXPAND, 0);
    VBoxSizer->Add(HBoxSizer, 1, wxEXPAND, 5);
    VBoxSizer->Add(HScrollBar, 0, wxEXPAND, 0);
    SetSizer(VBoxSizer);
    VBoxSizer->Fit(this);
    VBoxSizer->SetSizeHints(this);
    splitWindow->SetSashGravity(0.5);

    TCLeft->SetVScrollBar(VScrollBar);
    TCLeft->SetHScrollBar(HScrollBar);
    TCRight->SetVScrollBar(VScrollBar);
    TCRight->SetHScrollBar(HScrollBar);

    m_timer = new LineChangedTimer(this);
    m_timer->start();

    m_vscrollpos = VScrollBar->GetThumbPosition();
    m_hscrollpos = HScrollBar->GetThumbPosition();
}

cbSideBySideCtrl::~cbSideBySideCtrl()
{
    wxDELETE(m_timer);
}

void cbSideBySideCtrl::Init(cbDiffColors colset, bool leftReadOnly, bool rightReadOnly)
{
    leftReadOnly_ = leftReadOnly;
    rightReadOnly_ = rightReadOnly;
    const wxColor marbkg = TCLeft->StyleGetBackground(wxSCI_STYLE_LINENUMBER);

    cbEditor::ApplyStyles(TCLeft);
    TCLeft->SetMarginWidth(1, 16);
    TCLeft->SetMarginType(1, wxSCI_MARGIN_SYMBOL);
    TCLeft->SetMarginWidth(2,0);    // to hide the change and the fold margin
    TCLeft->SetMarginWidth(3,0);    // made by cbEditor::ApplyStyles
    TCLeft->MarkerDefine(MINUS_MARKER, wxSCI_MARK_MINUS, colset.m_removedlines, colset.m_removedlines);
    TCLeft->MarkerDefine(EQUAL_MARKER, wxSCI_MARK_CHARACTER + 61, *wxWHITE, marbkg);
    TCLeft->MarkerDefine(RED_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.m_removedlines, colset.m_removedlines);
    TCLeft->MarkerSetAlpha(RED_BKG_MARKER, colset.m_removedlines.Alpha());
    TCLeft->AnnotationSetVisible(wxSCI_ANNOTATION_STANDARD);
    if(colset.m_caretlinetype == 0)
    {
        TCLeft->MarkerDefine(CARET_LINE_MARKER, wxSCI_MARK_UNDERLINE, colset.m_caretline, colset.m_caretline);
    }
    else
    {
        TCLeft->MarkerDefine(CARET_LINE_MARKER, wxSCI_MARK_BACKGROUND, colset.m_caretline, colset.m_caretline);
        TCLeft->MarkerSetAlpha(CARET_LINE_MARKER, colset.m_caretline.Alpha());
    }
    const auto lang = colset.m_hlang;
    const bool isC = lang == "C/C++";
    m_theme->Apply(m_theme->GetHighlightLanguage(lang), TCLeft, isC, true);
    Connect( TCLeft->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));

    cbEditor::ApplyStyles(TCRight);
    TCRight->SetMarginWidth(1, 16);
    TCRight->SetMarginType(1, wxSCI_MARGIN_SYMBOL);
    TCRight->SetMarginWidth(2,0);    // to hide the change and fold margin
    TCRight->SetMarginWidth(3,0);    // made by cbEditor::ApplyStyles
    TCRight->MarkerDefine(PLUS_MARKER, wxSCI_MARK_PLUS, colset.m_addedlines, colset.m_addedlines);
    TCRight->MarkerDefine(EQUAL_MARKER, wxSCI_MARK_CHARACTER + 61, *wxWHITE, marbkg);
    TCRight->MarkerDefine(GREEN_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.m_addedlines, colset.m_addedlines);
    TCRight->MarkerSetAlpha(GREEN_BKG_MARKER, colset.m_addedlines.Alpha());
    TCRight->AnnotationSetVisible(wxSCI_ANNOTATION_STANDARD);
    if(colset.m_caretlinetype == 0)
    {
        TCRight->MarkerDefine(CARET_LINE_MARKER, wxSCI_MARK_UNDERLINE, colset.m_caretline, colset.m_caretline);
    }
    else
    {
        TCRight->MarkerDefine(CARET_LINE_MARKER, wxSCI_MARK_BACKGROUND, colset.m_caretline, colset.m_caretline);
        TCRight->MarkerSetAlpha(CARET_LINE_MARKER, colset.m_caretline.Alpha());
    }
    m_theme->Apply(m_theme->GetHighlightLanguage(colset.m_hlang), TCRight, isC, true);
    Connect( TCRight->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));
}

void cbSideBySideCtrl::ShowDiff(wxDiff diff)
{
    std::map<long, int> right_added  = diff.GetAddedLines();
    std::map<long, int> right_empty  = diff.GetRightEmptyLines();
    std::map<long, int> left_empty   = diff.GetLeftEmptyLines();
    std::map<long, int> left_removed = diff.GetRemovedLines();

    leftFilename_ = diff.GetFromFilename();
    rightFilename_ = diff.GetToFilename();

    linesLeftWithDifferences_.clear();
    TCLeft->SetReadOnly(false);
    TCLeft->ClearAll();
    TCLeft->LoadFile(diff.GetFromFilename());
    TCLeft->AnnotationClearAll();
    for(auto itr = left_removed.begin() ; itr != left_removed.end() ; ++itr)
    {
        long line_removed = itr->first;
        unsigned int removed = itr->second;
        for(unsigned int k = 0 ; k < removed ; ++k)
        {
            TCLeft->MarkerAdd(line_removed+k, MINUS_MARKER);
            TCLeft->MarkerAdd(line_removed+k, RED_BKG_MARKER);
        }
        auto it = left_empty.find(line_removed);
        if(it != left_empty.end())
        {
            long line_empty = line_removed + removed;
            unsigned int empty = it->second - removed;
            wxString annotationStr('\n', empty-1);
            TCLeft->AnnotationSetText(line_empty-1, annotationStr);
            left_empty.erase(it);
        }
        linesLeftWithDifferences_.push_back(line_removed);
    }
    for(auto itr = left_empty.begin(); itr != left_empty.end() ; ++itr )
    {
        long line = itr->first;
        unsigned int len = itr->second;
        wxString annotationStr('\n', len-1);
        TCLeft->AnnotationSetText(line-1, annotationStr);
        auto i = std::lower_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), line);
        linesLeftWithDifferences_.insert(i, line);
    }
    if(leftReadOnly_)
        TCLeft->SetReadOnly(true);
    TCLeft->SetMarginType(0, wxSCI_MARGIN_NUMBER);
    setLineNumberMarginWidth(TCLeft, lineNumbersWidthLeft);

    linesRightWithDifferences_.clear();
    TCRight->SetReadOnly(false);
    TCRight->ClearAll();
    TCRight->LoadFile(diff.GetToFilename());
    TCRight->AnnotationClearAll();
    for(auto itr = right_added.begin() ; itr != right_added.end() ; ++itr)
    {
        long line_added = itr->first;
        unsigned int added = itr->second;
        for(unsigned int k = 0 ; k < added ; ++k)
        {
            TCRight->MarkerAdd(line_added+k, PLUS_MARKER);
            TCRight->MarkerAdd(line_added+k, GREEN_BKG_MARKER);
        }
        auto it = right_empty.find(line_added);
        if(it != right_empty.end())
        {
            long line_empty = line_added + added;
            unsigned int empty = it->second - added;
            wxString annotationStr('\n', empty-1);
            TCRight->AnnotationSetText(line_empty-1, annotationStr);
            right_empty.erase(it);
        }
        linesRightWithDifferences_.push_back(line_added);
    }
    for(auto itr = right_empty.begin(); itr != right_empty.end() ; ++itr )
    {
        long line = itr->first;
        unsigned int len = itr->second;
        wxString annotationStr('\n', len-1);
        TCRight->AnnotationSetText(line-1, annotationStr);
        auto i = std::lower_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), line);
        linesRightWithDifferences_.insert(i, line);
    }
    if(rightReadOnly_)
        TCRight->SetReadOnly(true);
    TCRight->SetMarginType(0, wxSCI_MARGIN_NUMBER);
    setLineNumberMarginWidth(TCRight, lineNumbersWidthRight);
}

void cbSideBySideCtrl::Synchronize()
{
    int curr_line = 0;
    if(TCLeft->GetSCIFocus())   // which scintilla control has the focus?
        curr_line = TCLeft->VisibleFromDocLine(TCLeft->GetCurrentLine());

    if(TCRight->GetSCIFocus())
        curr_line = TCRight->VisibleFromDocLine(TCRight->GetCurrentLine());

    /* Caretline background synchronisation */
    if(curr_line != lastSyncedLine_)
    {
        TCLeft->MarkerDeleteHandle(lastSyncedLHandle);
        TCRight->MarkerDeleteHandle(lastSyncedRHandle);

        lastSyncedLHandle = TCLeft->MarkerAdd(TCLeft->DocLineFromVisible(curr_line), CARET_LINE_MARKER);
        lastSyncedRHandle = TCRight->MarkerAdd(TCRight->DocLineFromVisible(curr_line), CARET_LINE_MARKER);

        lastSyncedLine_ = curr_line;
        return;
    }

    int curr_scroll_focus = 0;
    // which wxcontrol has the scrollfocus?
    if(TCLeft->GetRect().Contains(ScreenToClient(wxGetMousePosition())))
        curr_scroll_focus = 1;
    if(TCRight->GetRect().Contains(ScreenToClient(wxGetMousePosition())))
        curr_scroll_focus = 2;

    /* Zoom synchronisation */
    if(TCRight->GetZoom() != TCLeft->GetZoom())
    {
        if(curr_scroll_focus == 1)
            TCRight->SetZoom(TCLeft->GetZoom());
        if(curr_scroll_focus == 2)
            TCLeft->SetZoom(TCRight->GetZoom());
    }

    /* Scroll synchronisation */
    if(m_vscrollpos != VScrollBar->GetThumbPosition())
    {
        TCLeft->SetFirstVisibleLine(VScrollBar->GetThumbPosition());
        TCRight->SetFirstVisibleLine(VScrollBar->GetThumbPosition());
        m_vscrollpos = VScrollBar->GetThumbPosition();
    }
    if(m_hscrollpos != HScrollBar->GetThumbPosition())
    {
        TCLeft->SetXOffset(HScrollBar->GetThumbPosition());
        TCRight->SetXOffset(HScrollBar->GetThumbPosition());
        m_hscrollpos = HScrollBar->GetThumbPosition();
    }
}

bool cbSideBySideCtrl::GetModified() const
{
    if(TCLeft->HasFocus())
        return TCLeft->GetModify();
    else if(TCRight->HasFocus())
        return TCRight->GetModify();
    else
        return TCLeft->GetModify() || TCRight->GetModify();
}

bool cbSideBySideCtrl::QueryClose()
{
    if (!GetModified())
        return true;

    if(TCLeft->GetModify() && TCRight->GetModify())
    {
        wxArrayString choices;
        choices.Add("none");
        choices.Add("left only");
        choices.Add("right only");
        choices.Add("both");
        switch(wxGetSingleChoiceIndex("save", "Confirm", choices))
        {
        case  0: TCLeft->SetSavePoint(); TCRight->SetSavePoint();break;
        case  1: TCRight->SetSavePoint(); return SaveLeft();
        case  2: TCLeft->SetSavePoint();  return SaveRight();
        case  3:
        {
            const bool leftOk = SaveLeft();
            const bool rightOk = SaveRight();
            return rightOk && leftOk;
        }
        default:
        case -1: return false;
        }
    }
    else if ( TCLeft->GetModify() )
    {
        int answer = wxMessageBox("Save Left?", "Confirm", wxYES_NO | wxCANCEL);
        if (answer == wxCANCEL)
            return false;
        else if (answer == wxYES)
            return SaveLeft();
        else
            TCLeft->SetSavePoint();
    }
    else if ( TCRight->GetModify() )
    {
        int answer = wxMessageBox("Save Right?", "Confirm", wxYES_NO | wxCANCEL);
        if (answer == wxCANCEL)
            return false;
        else if (answer == wxYES)
            return SaveRight();
        else
            TCRight->SetSavePoint();
    }

    parent_->updateTitle();
    return true;
}

bool cbSideBySideCtrl::Save()
{
    if(TCLeft->HasFocus())
        return SaveLeft();
    else if(TCRight->HasFocus())
        return SaveRight();
    else
    {
        bool sl = SaveLeft();
        bool sr = SaveRight();
        return sl && sr;
    }

}

bool cbSideBySideCtrl::SaveLeft()
{
    if(TCLeft->GetModify())
    {
        if(!TCLeft->SaveFile(leftFilename_))
            return false;
        else
            parent_->updateTitle();
    }
    return true;
}

bool cbSideBySideCtrl::SaveRight()
{
    if(TCRight->GetModify())
    {
        if(!TCRight->SaveFile(rightFilename_))
            return false;
        else
            parent_->updateTitle();
    }
    return true;
}

bool cbSideBySideCtrl::LeftModified()
{
    return TCLeft->GetModify();
}

bool cbSideBySideCtrl::RightModified()
{
    return TCRight->GetModify();
}

void cbSideBySideCtrl::setLineNumberMarginWidth(cbStyledTextCtrl *stc, int &currWidth)
{
    ConfigManager *cfg = Manager::Get()->GetConfigManager(_T("editor"));
    int pixelWidth = stc->TextWidth(wxSCI_STYLE_LINENUMBER, _T("9"));
    if (cfg->ReadBool(_T("/margin/dynamic_width"), false))
    {
        int lineNumChars = 1;
        int lineCount = stc->GetLineCount();

        while (lineCount >= 10)
        {
            lineCount /= 10;
            ++lineNumChars;
        }

        int lineNumWidth =  lineNumChars * pixelWidth + pixelWidth * 0.75;

        if (lineNumWidth != currWidth)
        {
            stc->SetMarginWidth(0, lineNumWidth);
            currWidth = lineNumWidth;
        }
    }
    else
        stc->SetMarginWidth(0, pixelWidth * 0.75 + cfg->ReadInt(_T("/margin/width_chars"), 6) * pixelWidth);
}

void cbSideBySideCtrl::OnEditorChange(wxScintillaEvent &event)
{
    parent_->updateTitle();
}

void cbSideBySideCtrl::Undo()
{
    if(!leftReadOnly_ && TCLeft->HasFocus())
        TCLeft->Undo();

    if(!rightReadOnly_ && TCRight->HasFocus())
        TCRight->Undo();
}

void cbSideBySideCtrl::Redo()
{
    if(!leftReadOnly_ && TCLeft->HasFocus())
        TCLeft->Redo();

    if(!rightReadOnly_ && TCRight->HasFocus())
        TCRight->Redo();
}

void cbSideBySideCtrl::ClearHistory()
{
    if(TCLeft->HasFocus())
        TCLeft->EmptyUndoBuffer();

    if(TCRight->HasFocus())
        TCRight->EmptyUndoBuffer();
}

void cbSideBySideCtrl::Cut()
{
    if(!leftReadOnly_ && TCLeft->HasFocus())
        TCLeft->Cut();

    if(!rightReadOnly_ && TCRight->HasFocus())
        TCRight->Cut();
}

void cbSideBySideCtrl::Copy()
{
    if(TCLeft->HasFocus())
        TCLeft->Copy();

    if(TCRight->HasFocus())
        TCRight->Copy();
}

void cbSideBySideCtrl::Paste()
{
    if(!leftReadOnly_ && TCLeft->HasFocus())
        TCLeft->Paste();

    if(!rightReadOnly_ && TCRight->HasFocus())
        TCRight->Paste();
}

bool cbSideBySideCtrl::CanUndo() const
{
    if(TCLeft->HasFocus())
        return !leftReadOnly_ && TCLeft->CanUndo();

    if(TCRight->HasFocus())
        return !rightReadOnly_ && TCRight->CanUndo();

    return false;
}

bool cbSideBySideCtrl::CanRedo() const
{
    if(TCLeft->HasFocus())
        return !leftReadOnly_ && TCLeft->CanRedo();

    if(TCRight->HasFocus())
        return !rightReadOnly_ && TCRight->CanRedo();

    return false;
}

bool cbSideBySideCtrl::HasSelection() const
{
    if(TCLeft->HasFocus())
        return TCLeft->GetSelectionStart() != TCLeft->GetSelectionEnd();

    if(TCRight->HasFocus())
        return TCRight->GetSelectionStart() != TCRight->GetSelectionEnd();

    return false;
}

bool cbSideBySideCtrl::CanPaste() const
{
    if(TCLeft->HasFocus())
    {
        if (platform::gtk)
            return !leftReadOnly_;
        return TCLeft->CanPaste() && !leftReadOnly_;
    }

    if(TCRight->HasFocus())
    {
        if (platform::gtk)
            return !rightReadOnly_;
        return TCRight->CanPaste() && !rightReadOnly_;
    }

    return false;
}

bool cbSideBySideCtrl::CanSelectAll() const
{
    if(TCLeft->HasFocus())
        return TCLeft->GetLength() > 0;

    if(TCRight->HasFocus())
        return TCLeft->GetLength() > 0;

    return false;
}

void cbSideBySideCtrl::SelectAll()
{
    if(TCLeft->HasFocus())
        TCLeft->SelectAll();

    if(TCRight->HasFocus())
        TCRight->SelectAll();
}

void cbSideBySideCtrl::NextDifference()
{
    if(TCLeft->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
            return;

        auto i = std::upper_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), TCLeft->GetCurrentLine());

        if(i == linesLeftWithDifferences_.end())
            return;

        TCLeft->GotoLine(*i);
    }
    else if(TCRight->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return;

        auto i = std::upper_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), TCRight->GetCurrentLine());

        if(i == linesRightWithDifferences_.end())
            return;

        TCRight->GotoLine(*i);
    }
}

void cbSideBySideCtrl::PrevDifference()
{
    if(TCLeft->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
            return;

        auto i = std::lower_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), TCLeft->GetCurrentLine());

        if(i == linesLeftWithDifferences_.begin())
            return;

        TCLeft->GotoLine(*(--i));
    }
    else if(TCRight->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return;

        auto i = std::lower_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), TCRight->GetCurrentLine());

        if(i == linesRightWithDifferences_.begin())
            return;

        TCRight->GotoLine(*(--i));
    }
}

bool cbSideBySideCtrl::CanGotoNextDiff()
{
    if(TCLeft->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
            return false;

        auto i = std::upper_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), TCLeft->GetCurrentLine());

        return i != linesLeftWithDifferences_.end();
    }
    else if(TCRight->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return false;

        auto i = std::upper_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), TCRight->GetCurrentLine());

        return i != linesRightWithDifferences_.end();
    }
    return false;
}

bool cbSideBySideCtrl::CanGotoPrevDiff()
{

    if(TCLeft->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
            return false;

        auto i = std::lower_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), TCLeft->GetCurrentLine());

        return i != linesLeftWithDifferences_.begin();
    }
    else if(TCRight->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return false;

        auto i = std::lower_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), TCRight->GetCurrentLine());

        return i != linesRightWithDifferences_.begin();
    }
    return false;
}

