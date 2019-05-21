#ifndef CBSIDEBYSIDECTRL_H
#define CBSIDEBYSIDECTRL_H

#include "cbDiffCtrl.h"

class LineChangedTimer;
class cbStyledTextCtrl;

class cbSideBySideCtrl : public cbDiffCtrl
{
public:
    cbSideBySideCtrl(cbDiffEditor *parent);
    virtual ~cbSideBySideCtrl();
    virtual void Init(cbDiffColors colset) override;
    virtual void ShowDiff(wxDiff diff) override;
    void Synchronize();

    virtual void NextDifference()override;
    virtual void PrevDifference()override;
    virtual bool CanGotoNextDiff()override;
    virtual bool CanGotoPrevDiff()override;

    virtual bool GetModified() const override;
    virtual bool QueryClose() override;
    virtual bool Save() override;

    virtual void Undo()override;
    virtual void Redo()override;
    virtual void ClearHistory()override;
    virtual void Cut()override;
    virtual void Copy()override;
    virtual void Paste()override;
    virtual bool CanUndo() const override;
    virtual bool CanRedo() const override;
    virtual bool HasSelection() const override;
    virtual bool CanPaste() const override;
    virtual bool CanSelectAll() const override;
    virtual void SelectAll() override;
protected:
    virtual bool LeftModified() override;
    virtual bool RightModified() override;
private:
    int lastSyncedLine_;
    int lastSyncedLHandle;
    int lastSyncedRHandle;
    void OnEditorChange(wxScintillaEvent &event);
    bool SaveLeft();
    bool SaveRight();
    cbStyledTextCtrl *TCLeft;
    cbStyledTextCtrl *TCRight;

    wxScrollBar *VScrollBar;
    wxScrollBar *HScrollBar;

    std::vector<long> linesRightWithDifferences_;
    std::vector<long> linesLeftWithDifferences_;
    int lineNumbersWidthLeft;
    int lineNumbersWidthRight;

    static void setLineNumberMarginWidth(cbStyledTextCtrl *stc, int &currWidth);

    int m_vscrollpos;
    int m_hscrollpos;

    LineChangedTimer *m_timer;
    DECLARE_EVENT_TABLE()
};

#endif
