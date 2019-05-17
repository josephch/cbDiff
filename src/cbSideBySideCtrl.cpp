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
    cbSideBySideCtrl* m_pane;
public:
    LineChangedTimer(cbSideBySideCtrl* pane) : wxTimer()
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

cbSideBySideCtrl::cbSideBySideCtrl(wxWindow* parent):
    cbDiffCtrl(parent),
    lineNumbersWidthLeft(0),
    lineNumbersWidthRight(0)
{
    wxBoxSizer* VBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* HBoxSizer = new wxBoxSizer(wxHORIZONTAL);
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
}

void cbSideBySideCtrl::ShowDiff(wxDiff diff)
{
    std::map<long, int> right_added  = diff.GetAddedLines();
    std::map<long, int> right_empty  = diff.GetRightEmptyLines();
    std::map<long, int> left_empty   = diff.GetLeftEmptyLines();
    std::map<long, int> left_removed = diff.GetRemovedLines();

    leftFilename_ = diff.GetFromFilename();
    rightFilename_ = diff.GetToFilename();

    TCLeft->SetReadOnly(false);
    TCLeft->ClearAll();
    TCLeft->LoadFile(diff.GetFromFilename());
    TCLeft->AnnotationClearAll();
    for(auto itr = left_removed.begin() ; itr != left_removed.end() ; ++itr)
    {
        long line = itr->first;
        unsigned int len = itr->second;
        for(unsigned int k = 0 ; k < len ; ++k)
        {
            TCLeft->MarkerAdd(line+k, MINUS_MARKER);
            TCLeft->MarkerAdd(line+k, RED_BKG_MARKER);
        }
    }
    for(auto itr = left_empty.begin(); itr != left_empty.end() ; ++itr )
    {
        long line = itr->first;
        unsigned int len = itr->second;
        wxString annotationStr('\n', len-1);
        TCLeft->AnnotationSetText(line-1, annotationStr);
    }
    if(leftReadOnly_)
        TCLeft->SetReadOnly(true);
    TCLeft->SetMarginType(0, wxSCI_MARGIN_NUMBER);
    setLineNumberMarginWidth(TCLeft, lineNumbersWidthLeft);

    TCRight->SetReadOnly(false);
    TCRight->ClearAll();
    TCRight->LoadFile(diff.GetToFilename());
    TCRight->AnnotationClearAll();
    for(auto itr = right_added.begin() ; itr != right_added.end() ; ++itr)
    {
        long line = itr->first;
        unsigned int len = itr->second;
        for(unsigned int k = 0 ; k < len ; ++k)
        {
            TCRight->MarkerAdd(line+k, PLUS_MARKER);
            TCRight->MarkerAdd(line+k, GREEN_BKG_MARKER);
        }
    }
    for(auto itr = right_empty.begin(); itr != right_empty.end() ; ++itr )
    {
        long line = itr->first;
        unsigned int len = itr->second;
        wxString annotationStr('\n', len-1);
        TCRight->AnnotationSetText(line-1, annotationStr);
    }
    if(rightReadOnly_)
        TCRight->SetReadOnly(true);
    TCRight->SetMarginType(0, wxSCI_MARGIN_NUMBER);
    setLineNumberMarginWidth(TCRight, lineNumbersWidthRight);
}

void cbSideBySideCtrl::Synchronize()
{
    static int last_lhandle = -1;
    static int last_rhandle = -1;
    static int last_line = -1;

    int curr_line = 0;
    if(TCLeft->GetSCIFocus())   // which scintilla control has the focus?
        curr_line = TCLeft->GetCurrentLine();

    if(TCRight->GetSCIFocus())
        curr_line = TCRight->GetCurrentLine();

    /* Caretline background synchronisation */
    if(curr_line != last_line)
    {
        TCLeft->MarkerDeleteHandle(last_lhandle);
        TCRight->MarkerDeleteHandle(last_rhandle);

        last_lhandle = TCLeft->MarkerAdd(curr_line, CARET_LINE_MARKER);
        last_rhandle = TCRight->MarkerAdd(curr_line, CARET_LINE_MARKER);

        last_line = curr_line;
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
        default:
        case -1: return false;
        case  0: TCLeft->SetSavePoint(); TCRight->SetSavePoint(); return true;
        case  1: TCRight->SetSavePoint(); return SaveLeft();
        case  2: TCLeft->SetSavePoint();  return SaveRight();
        case  3: return Save();
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

    return true;
}

bool cbSideBySideCtrl::Save()
{
    const bool leftOk = SaveLeft();
    const bool rightOk = SaveRight();
    return rightOk && leftOk;
}

bool cbSideBySideCtrl::SaveLeft()
{
    if(TCLeft->GetModify())
        if(!TCLeft->SaveFile(leftFilename_))
            return false;
    return true;
}

bool cbSideBySideCtrl::SaveRight()
{
    if(TCRight->GetModify())
        if(!TCRight->SaveFile(rightFilename_))
            return false;
    return true;
}

void cbSideBySideCtrl::setLineNumberMarginWidth(cbStyledTextCtrl* stc, int &currWidth)
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("editor"));
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
