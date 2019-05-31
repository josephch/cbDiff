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
    void SynchronizeSelection();
    void SynchronizeCaretline();
    void SynchronizeZoom();
    void SynchronizeScroll();

    virtual void NextDifference()override;
    virtual bool CanGotoNextDiff()override;
    virtual void PrevDifference()override;
    virtual bool CanGotoPrevDiff()override;
    virtual void FirstDifference()override;
    virtual bool CanGotoFirstDiff()override;
    virtual void LastDifference()override;
    virtual bool CanGotoLastDiff()override;

    virtual void CopyLeft()override;
    virtual bool CanCopyLeft()override;
    virtual void CopyRight()override;
    virtual bool CanCopyRight()override;
    virtual void CopyLeftNext()override;
    virtual bool CanCopyLeftNext()override;
    virtual void CopyRightNext()override;
    virtual bool CanCopyRightNext()override;


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
    void selectDiffRight(long line);
    void markDiffRight(long line);
    void unmarkDiffRight();
    void markEmptyPartRight(long line);
    void selectDiffLeft(long line);
    void markDiffLeft(long line);
    void unmarkDiffLeft();
    void markEmptyPartLeft(long line);

    int lastSyncedLine_;
    int lastSyncedLHandle_;
    int lastSyncedRHandle_;
    long lastLeftMarkedDiff_;
    long lastRightMarkedDiff_;
    long lastLeftMarkedEmptyDiff_;
    long lastRightMarkedEmptyDiff_;
    void OnEditorChange(wxScintillaEvent &event);
    bool SaveLeft();
    bool SaveRight();
    cbStyledTextCtrl *tcLeft_;
    cbStyledTextCtrl *tcRight_;

    wxScrollBar *vScrollBar_;
    wxScrollBar *hScrollBar_;

    struct Block
    {
        Block():len(0),empty(0), ref(0){}
        int len;
        int empty;
        long ref;
    };
    std::vector<long> linesRightWithDifferences_;
    std::map<long, Block> rightChanges_;
    std::vector<long> linesLeftWithDifferences_;
    std::map<long, Block> leftChanges_;

    int lineNumbersWidthLeft_;
    int lineNumbersWidthRight_;

    static void setLineNumberMarginWidth(cbStyledTextCtrl *stc, int &currWidth);

    int vscrollpos_;
    int hscrollpos_;

    LineChangedTimer *timer_;
    DECLARE_EVENT_TABLE()
};

#endif
