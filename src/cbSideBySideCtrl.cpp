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
    cbSideBySideCtrl *pane_;
public:
    LineChangedTimer(cbSideBySideCtrl *pane) : wxTimer()
    {
        pane_ = pane;
    }
    void Notify()
    {
        pane_->Synchronize();
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
    lastSyncedLHandle_(-1),
    lastSyncedRHandle_(-1),
    lineNumbersWidthLeft_(0),
    lineNumbersWidthRight_(0)
{
    wxBoxSizer *VBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *HBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    vScrollBar_ = new wxScrollBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
    hScrollBar_ = new wxScrollBar(this, wxID_ANY);
    wxSplitterWindow *splitWindow = new wxSplitterWindow(this, wxID_ANY);
    tcLeft_ = new cbStyledTextCtrl(splitWindow, wxID_ANY);
    tcRight_ = new cbStyledTextCtrl(splitWindow, wxID_ANY);
    splitWindow->SplitVertically(tcLeft_, tcRight_);
    HBoxSizer->Add(splitWindow, 1, wxEXPAND, 0);
    HBoxSizer->Add(vScrollBar_, 0, wxEXPAND, 0);
    VBoxSizer->Add(HBoxSizer, 1, wxEXPAND, 5);
    VBoxSizer->Add(hScrollBar_, 0, wxEXPAND, 0);
    SetSizer(VBoxSizer);
    VBoxSizer->Fit(this);
    VBoxSizer->SetSizeHints(this);
    splitWindow->SetSashGravity(0.5);

    tcLeft_->SetVScrollBar(vScrollBar_);
    tcLeft_->SetHScrollBar(hScrollBar_);
    tcRight_->SetVScrollBar(vScrollBar_);
    tcRight_->SetHScrollBar(hScrollBar_);

    timer_ = new LineChangedTimer(this);
    timer_->start();

    vscrollpos_ = vScrollBar_->GetThumbPosition();
    hscrollpos_ = hScrollBar_->GetThumbPosition();
}

cbSideBySideCtrl::~cbSideBySideCtrl()
{
    wxDELETE(timer_);

    Disconnect( tcLeft_->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));
    Disconnect( tcRight_->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));
}

void cbSideBySideCtrl::Init(cbDiffColors colset)
{
    Disconnect( tcLeft_->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));
    Disconnect( tcRight_->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));

    const wxColor marbkg = tcLeft_->StyleGetBackground(wxSCI_STYLE_LINENUMBER);

    cbEditor::ApplyStyles(tcLeft_);
    tcLeft_->SetMarginWidth(1, 16);
    tcLeft_->SetMarginType(1, wxSCI_MARGIN_SYMBOL);
    tcLeft_->SetMarginWidth(2,0);    // to hide the change and the fold margin
    tcLeft_->SetMarginWidth(3,0);    // made by cbEditor::ApplyStyles
    tcLeft_->MarkerDefine(MINUS_MARKER, wxSCI_MARK_MINUS, colset.removedlines_, colset.removedlines_);
    tcLeft_->MarkerDefine(EQUAL_MARKER, wxSCI_MARK_CHARACTER + 61, *wxWHITE, marbkg);
    tcLeft_->MarkerDefine(RED_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.removedlines_, colset.removedlines_);
    tcLeft_->MarkerSetAlpha(RED_BKG_MARKER, colset.removedlines_.Alpha());
    tcLeft_->AnnotationSetVisible(wxSCI_ANNOTATION_STANDARD);
    if(colset.caretlinetype_ == 0)
    {
        tcLeft_->MarkerDefine(CARET_LINE_MARKER, wxSCI_MARK_UNDERLINE, colset.caretline_, colset.caretline_);
    }
    else
    {
        tcLeft_->MarkerDefine(CARET_LINE_MARKER, wxSCI_MARK_BACKGROUND, colset.caretline_, colset.caretline_);
        tcLeft_->MarkerSetAlpha(CARET_LINE_MARKER, colset.caretline_.Alpha());
    }
    const auto lang = colset.hlang_;
    const bool isC = lang == "C/C++";
    theme_->Apply(theme_->GetHighlightLanguage(lang), tcLeft_, isC, true);

    cbEditor::ApplyStyles(tcRight_);
    tcRight_->SetMarginWidth(1, 16);
    tcRight_->SetMarginType(1, wxSCI_MARGIN_SYMBOL);
    tcRight_->SetMarginWidth(2,0);    // to hide the change and fold margin
    tcRight_->SetMarginWidth(3,0);    // made by cbEditor::ApplyStyles
    tcRight_->MarkerDefine(PLUS_MARKER, wxSCI_MARK_PLUS, colset.addedlines_, colset.addedlines_);
    tcRight_->MarkerDefine(EQUAL_MARKER, wxSCI_MARK_CHARACTER + 61, *wxWHITE, marbkg);
    tcRight_->MarkerDefine(GREEN_BKG_MARKER, wxSCI_MARK_BACKGROUND, colset.addedlines_, colset.addedlines_);
    tcRight_->MarkerSetAlpha(GREEN_BKG_MARKER, colset.addedlines_.Alpha());
    tcRight_->AnnotationSetVisible(wxSCI_ANNOTATION_STANDARD);
    if(colset.caretlinetype_ == 0)
    {
        tcRight_->MarkerDefine(CARET_LINE_MARKER, wxSCI_MARK_UNDERLINE, colset.caretline_, colset.caretline_);
    }
    else
    {
        tcRight_->MarkerDefine(CARET_LINE_MARKER, wxSCI_MARK_BACKGROUND, colset.caretline_, colset.caretline_);
        tcRight_->MarkerSetAlpha(CARET_LINE_MARKER, colset.caretline_.Alpha());
    }
    theme_->Apply(theme_->GetHighlightLanguage(colset.hlang_), tcRight_, isC, true);
}

void cbSideBySideCtrl::ShowDiff(wxDiff diff)
{
    Disconnect( tcLeft_->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));
    Disconnect( tcRight_->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));

    std::map<long, int> right_added  = diff.GetAddedLines();
    std::map<long, int> right_empty  = diff.GetRightEmptyLines();
    std::map<long, int> left_empty   = diff.GetLeftEmptyLines();
    std::map<long, int> left_removed = diff.GetRemovedLines();

    leftFilename_ = diff.GetLeftFilename();
    rightFilename_ = diff.GetRightFilename();
    leftReadOnly_ = diff.LeftReadOnly();
    rightReadOnly_ = diff.RightReadOnly();

    linesLeftWithDifferences_.clear();
    int leftCursorPos =   tcLeft_->GetCurrentPos();
    tcLeft_->SetReadOnly(false);
    tcLeft_->ClearAll();
    tcLeft_->LoadFile(diff.GetLeftFilename());
    tcLeft_->AnnotationClearAll();
    for(auto itr = left_removed.begin() ; itr != left_removed.end() ; ++itr)
    {
        long line_removed = itr->first;
        unsigned int removed = itr->second;
        for(unsigned int k = 0 ; k < removed ; ++k)
        {
            tcLeft_->MarkerAdd(line_removed+k, MINUS_MARKER);
            tcLeft_->MarkerAdd(line_removed+k, RED_BKG_MARKER);
        }
        auto it = left_empty.find(line_removed);
        if(it != left_empty.end())
        {
            long line_empty = line_removed + removed;
            unsigned int empty = it->second - removed;
            wxString annotationStr('\n', empty-1);
            tcLeft_->AnnotationSetText(line_empty-1, annotationStr);
            left_empty.erase(it);
        }
        linesLeftWithDifferences_.push_back(line_removed);
    }
    for(auto itr = left_empty.begin(); itr != left_empty.end() ; ++itr )
    {
        long line = itr->first;
        unsigned int len = itr->second;
        wxString annotationStr('\n', len-1);
        tcLeft_->AnnotationSetText(line-1, annotationStr);
        auto i = std::lower_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), line);
        linesLeftWithDifferences_.insert(i, line);
    }
    if(leftReadOnly_)
        tcLeft_->SetReadOnly(true);
    tcLeft_->SetMarginType(0, wxSCI_MARGIN_NUMBER);
    setLineNumberMarginWidth(tcLeft_, lineNumbersWidthLeft_);
    tcLeft_->GotoPos(leftCursorPos);

    linesRightWithDifferences_.clear();
    int rightCursorPos = tcRight_->GetCurrentPos();
    tcRight_->SetReadOnly(false);
    tcRight_->ClearAll();
    tcRight_->LoadFile(diff.GetRightFilename());
    tcRight_->AnnotationClearAll();
    for(auto itr = right_added.begin() ; itr != right_added.end() ; ++itr)
    {
        long line_added = itr->first;
        unsigned int added = itr->second;
        for(unsigned int k = 0 ; k < added ; ++k)
        {
            tcRight_->MarkerAdd(line_added+k, PLUS_MARKER);
            tcRight_->MarkerAdd(line_added+k, GREEN_BKG_MARKER);
        }
        auto it = right_empty.find(line_added);
        if(it != right_empty.end())
        {
            long line_empty = line_added + added;
            unsigned int empty = it->second - added;
            wxString annotationStr('\n', empty-1);
            tcRight_->AnnotationSetText(line_empty-1, annotationStr);
            right_empty.erase(it);
        }
        linesRightWithDifferences_.push_back(line_added);
    }
    for(auto itr = right_empty.begin(); itr != right_empty.end() ; ++itr )
    {
        long line = itr->first;
        unsigned int len = itr->second;
        wxString annotationStr('\n', len-1);
        tcRight_->AnnotationSetText(line-1, annotationStr);
        auto i = std::lower_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), line);
        linesRightWithDifferences_.insert(i, line);
    }
    if(rightReadOnly_)
        tcRight_->SetReadOnly(true);
    tcRight_->SetMarginType(0, wxSCI_MARGIN_NUMBER);
    setLineNumberMarginWidth(tcRight_, lineNumbersWidthRight_);
    tcRight_->GotoPos(rightCursorPos);

    Connect( tcLeft_->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));
    Connect( tcRight_->GetId(), wxEVT_SCI_CHANGE, wxScintillaEventHandler(cbSideBySideCtrl::OnEditorChange));
}

void cbSideBySideCtrl::Synchronize()
{
    int curr_line = 0;
    if(tcLeft_->GetSCIFocus())   // which scintilla control has the focus?
        curr_line = tcLeft_->VisibleFromDocLine(tcLeft_->GetCurrentLine());

    if(tcRight_->GetSCIFocus())
        curr_line = tcRight_->VisibleFromDocLine(tcRight_->GetCurrentLine());

    /* Caretline background synchronisation */
    if(curr_line != lastSyncedLine_)
    {
        tcLeft_->MarkerDeleteHandle(lastSyncedLHandle_);
        tcRight_->MarkerDeleteHandle(lastSyncedRHandle_);

        lastSyncedLHandle_ = tcLeft_->MarkerAdd(tcLeft_->DocLineFromVisible(curr_line), CARET_LINE_MARKER);
        lastSyncedRHandle_ = tcRight_->MarkerAdd(tcRight_->DocLineFromVisible(curr_line), CARET_LINE_MARKER);

        lastSyncedLine_ = curr_line;
        return;
    }

    int curr_scroll_focus = 0;
    // which wxcontrol has the scrollfocus?
    if(tcLeft_->GetRect().Contains(ScreenToClient(wxGetMousePosition())))
        curr_scroll_focus = 1;
    if(tcRight_->GetRect().Contains(ScreenToClient(wxGetMousePosition())))
        curr_scroll_focus = 2;

    /* Zoom synchronisation */
    if(tcRight_->GetZoom() != tcLeft_->GetZoom())
    {
        if(curr_scroll_focus == 1)
            tcRight_->SetZoom(tcLeft_->GetZoom());
        if(curr_scroll_focus == 2)
            tcLeft_->SetZoom(tcRight_->GetZoom());
    }

    /* Scroll synchronisation */
    if(vscrollpos_ != vScrollBar_->GetThumbPosition())
    {
        tcLeft_->SetFirstVisibleLine(vScrollBar_->GetThumbPosition());
        tcRight_->SetFirstVisibleLine(vScrollBar_->GetThumbPosition());
        vscrollpos_ = vScrollBar_->GetThumbPosition();
    }
    if(hscrollpos_ != hScrollBar_->GetThumbPosition())
    {
        tcLeft_->SetXOffset(hScrollBar_->GetThumbPosition());
        tcRight_->SetXOffset(hScrollBar_->GetThumbPosition());
        hscrollpos_ = hScrollBar_->GetThumbPosition();
    }
}

bool cbSideBySideCtrl::GetModified() const
{
    if(tcLeft_->HasFocus())
        return tcLeft_->GetModify();
    else if(tcRight_->HasFocus())
        return tcRight_->GetModify();
    else
        return tcLeft_->GetModify() || tcRight_->GetModify();
}

bool cbSideBySideCtrl::QueryClose()
{
    if (!GetModified())
        return true;

    if(tcLeft_->GetModify() && tcRight_->GetModify())
    {
        wxArrayString choices;
        choices.Add("none");
        choices.Add("left only");
        choices.Add("right only");
        choices.Add("both");
        switch(wxGetSingleChoiceIndex("save", "Confirm", choices))
        {
        case  0: tcLeft_->SetSavePoint(); tcRight_->SetSavePoint();break;
        case  1: tcRight_->SetSavePoint(); return SaveLeft();
        case  2: tcLeft_->SetSavePoint();  return SaveRight();
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
    else if ( tcLeft_->GetModify() )
    {
        int answer = wxMessageBox("Save Left?", "Confirm", wxYES_NO | wxCANCEL);
        if (answer == wxCANCEL)
            return false;
        else if (answer == wxYES)
            return SaveLeft();
        else
            tcLeft_->SetSavePoint();
    }
    else if ( tcRight_->GetModify() )
    {
        int answer = wxMessageBox("Save Right?", "Confirm", wxYES_NO | wxCANCEL);
        if (answer == wxCANCEL)
            return false;
        else if (answer == wxYES)
            return SaveRight();
        else
            tcRight_->SetSavePoint();
    }

    parent_->updateTitle();
    return true;
}

bool cbSideBySideCtrl::Save()
{
    if(tcLeft_->HasFocus())
        return SaveLeft();
    else if(tcRight_->HasFocus())
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
    if(tcLeft_->GetModify())
    {
        if(!tcLeft_->SaveFile(leftFilename_))
            return false;
        else
            parent_->updateTitle();
    }
    return true;
}

bool cbSideBySideCtrl::SaveRight()
{
    if(tcRight_->GetModify())
    {
        if(!tcRight_->SaveFile(rightFilename_))
            return false;
        else
            parent_->updateTitle();
    }
    return true;
}

bool cbSideBySideCtrl::LeftModified()
{
    return tcLeft_->GetModify();
}

bool cbSideBySideCtrl::RightModified()
{
    return tcRight_->GetModify();
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
    if(!leftReadOnly_ && tcLeft_->HasFocus())
        tcLeft_->Undo();

    if(!rightReadOnly_ && tcRight_->HasFocus())
        tcRight_->Undo();
}

void cbSideBySideCtrl::Redo()
{
    if(!leftReadOnly_ && tcLeft_->HasFocus())
        tcLeft_->Redo();

    if(!rightReadOnly_ && tcRight_->HasFocus())
        tcRight_->Redo();
}

void cbSideBySideCtrl::ClearHistory()
{
    if(tcLeft_->HasFocus())
        tcLeft_->EmptyUndoBuffer();

    if(tcRight_->HasFocus())
        tcRight_->EmptyUndoBuffer();
}

void cbSideBySideCtrl::Cut()
{
    if(!leftReadOnly_ && tcLeft_->HasFocus())
        tcLeft_->Cut();

    if(!rightReadOnly_ && tcRight_->HasFocus())
        tcRight_->Cut();
}

void cbSideBySideCtrl::Copy()
{
    if(tcLeft_->HasFocus())
        tcLeft_->Copy();

    if(tcRight_->HasFocus())
        tcRight_->Copy();
}

void cbSideBySideCtrl::Paste()
{
    if(!leftReadOnly_ && tcLeft_->HasFocus())
        tcLeft_->Paste();

    if(!rightReadOnly_ && tcRight_->HasFocus())
        tcRight_->Paste();
}

bool cbSideBySideCtrl::CanUndo() const
{
    if(tcLeft_->HasFocus())
        return !leftReadOnly_ && tcLeft_->CanUndo();

    if(tcRight_->HasFocus())
        return !rightReadOnly_ && tcRight_->CanUndo();

    return false;
}

bool cbSideBySideCtrl::CanRedo() const
{
    if(tcLeft_->HasFocus())
        return !leftReadOnly_ && tcLeft_->CanRedo();

    if(tcRight_->HasFocus())
        return !rightReadOnly_ && tcRight_->CanRedo();

    return false;
}

bool cbSideBySideCtrl::HasSelection() const
{
    if(tcLeft_->HasFocus())
        return tcLeft_->GetSelectionStart() != tcLeft_->GetSelectionEnd();

    if(tcRight_->HasFocus())
        return tcRight_->GetSelectionStart() != tcRight_->GetSelectionEnd();

    return false;
}

bool cbSideBySideCtrl::CanPaste() const
{
    if(tcLeft_->HasFocus())
    {
        if (platform::gtk)
            return !leftReadOnly_;
        return tcLeft_->CanPaste() && !leftReadOnly_;
    }

    if(tcRight_->HasFocus())
    {
        if (platform::gtk)
            return !rightReadOnly_;
        return tcRight_->CanPaste() && !rightReadOnly_;
    }

    return false;
}

bool cbSideBySideCtrl::CanSelectAll() const
{
    if(tcLeft_->HasFocus())
        return tcLeft_->GetLength() > 0;

    if(tcRight_->HasFocus())
        return tcLeft_->GetLength() > 0;

    return false;
}

void cbSideBySideCtrl::SelectAll()
{
    if(tcLeft_->HasFocus())
        tcLeft_->SelectAll();

    if(tcRight_->HasFocus())
        tcRight_->SelectAll();
}

void cbSideBySideCtrl::NextDifference()
{
    if(tcLeft_->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
            return;

        auto i = std::upper_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), tcLeft_->GetCurrentLine());

        if(i == linesLeftWithDifferences_.end())
            return;

        tcLeft_->GotoLine(*i);
    }
    else if(tcRight_->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return;

        auto i = std::upper_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), tcRight_->GetCurrentLine());

        if(i == linesRightWithDifferences_.end())
            return;

        tcRight_->GotoLine(*i);
    }
}

void cbSideBySideCtrl::PrevDifference()
{
    if(tcLeft_->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
            return;

        auto i = std::lower_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), tcLeft_->GetCurrentLine());

        if(i == linesLeftWithDifferences_.begin())
            return;

        tcLeft_->GotoLine(*(--i));
    }
    else if(tcRight_->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return;

        auto i = std::lower_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), tcRight_->GetCurrentLine());

        if(i == linesRightWithDifferences_.begin())
            return;

        tcRight_->GotoLine(*(--i));
    }
}

bool cbSideBySideCtrl::CanGotoNextDiff()
{
    if(tcLeft_->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
            return false;

        auto i = std::upper_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), tcLeft_->GetCurrentLine());

        return i != linesLeftWithDifferences_.end();
    }
    else if(tcRight_->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return false;

        auto i = std::upper_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), tcRight_->GetCurrentLine());

        return i != linesRightWithDifferences_.end();
    }
    return false;
}

bool cbSideBySideCtrl::CanGotoPrevDiff()
{
    if(tcLeft_->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
            return false;

        auto i = std::lower_bound(linesLeftWithDifferences_.begin(), linesLeftWithDifferences_.end(), tcLeft_->GetCurrentLine());

        return i != linesLeftWithDifferences_.begin();
    }
    else if(tcRight_->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return false;

        auto i = std::lower_bound(linesRightWithDifferences_.begin(), linesRightWithDifferences_.end(), tcRight_->GetCurrentLine());

        return i != linesRightWithDifferences_.begin();
    }
    return false;
}

void cbSideBySideCtrl::FirstDifference()
{
    if(tcLeft_->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty()) return;
        tcLeft_->GotoLine(linesLeftWithDifferences_.front());
    }
    else if(tcRight_->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty()) return;
        tcRight_->GotoLine(linesRightWithDifferences_.front());
    }
}

void cbSideBySideCtrl::LastDifference()
{
    if(tcLeft_->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty()) return;
        tcLeft_->GotoLine(linesLeftWithDifferences_.back());
    }
    else if(tcRight_->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty()) return;
        tcRight_->GotoLine(linesRightWithDifferences_.back());
    }
}

bool cbSideBySideCtrl::CanGotoFirstDiff()
{
    if(tcLeft_->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
           return false;

        return tcLeft_->GetCurrentLine() != linesLeftWithDifferences_.front();
    }
    else if(tcRight_->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return false;

        return tcRight_->GetCurrentLine() != linesRightWithDifferences_.front();
    }
    return false;
}

bool cbSideBySideCtrl::CanGotoLastDiff()
{
    if(tcLeft_->GetSCIFocus())
    {
        if(linesLeftWithDifferences_.empty())
           return false;

        return tcLeft_->GetCurrentLine() != linesLeftWithDifferences_.back();
    }
    else if(tcRight_->GetSCIFocus())
    {
        if(linesRightWithDifferences_.empty())
            return false;

        return tcRight_->GetCurrentLine() != linesRightWithDifferences_.back();
    }
    return false;
}

