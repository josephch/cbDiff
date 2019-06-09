#ifndef CBSIDEBYSIDECTRL_H
#define CBSIDEBYSIDECTRL_H

#include "cbDiffCtrl.h"

class LineChangedTimer;
class cbStyledTextCtrl;
class cbEditor;

class cbSideBySideCtrl : public cbDiffCtrl
{
public:
    cbSideBySideCtrl(cbDiffEditor *parent);
    virtual ~cbSideBySideCtrl();
    virtual void Init(cbDiffColors colset) override;
    virtual void ShowDiff(const wxDiff &diff) override;
    virtual void UpdateDiff(const wxDiff &diff) override;
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

    virtual void CopyToLeft()override{CopyTo(false);}
    virtual bool CanCopyToLeft()override;
    virtual void CopyToRight()override{CopyTo(true);}
    virtual bool CanCopyToRight()override;
    virtual void CopyToLeftNext()override;
    virtual bool CanCopyToLeftNext()override;
    virtual void CopyToRightNext()override;
    virtual bool CanCopyToRightNext()override;

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
    virtual std::vector<std::string> *GetLeftLines()override{return GetLines(tcLeft_);}
    virtual std::vector<std::string> *GetRightLines()override{return GetLines(tcRight_);}

protected:
    static std::vector<std::string> *GetLines(cbStyledTextCtrl *tc);
    virtual bool LeftModified() override;
    virtual bool RightModified() override;
private:
    struct Block
    {
        Block():len(0),empty(0), ref(0){}
        int len;
        int empty;
        long ref;
    };
    enum AddOrRem {Added, Removed};
    void selectDiff(long lline, long rline);
    static void markSelectionDiff(long line, long &lastMarkedDiff, std::map<long, Block> &changes, cbStyledTextCtrl *tc, cbStyledTextCtrl *tcOther, long &lastMarkedEmptyDiff, int &lastSyncedHandle, AddOrRem ar);
    static void markSelectionEmptyPart(long line, long &lastMarkedEmptyDiff, cbStyledTextCtrl *tc, std::map<long, Block> &changes);
    static void DeleteMarksForSelection(std::map<long, Block> &changes, const long &lastMarkedDiff, cbStyledTextCtrl *tc, const long &lastMarkedEmptyDiff, AddOrRem ar);

    static void unmarkSelectionDiff(const long &lastMarkedDiff, std::map<long, Block> &changes,  cbStyledTextCtrl *tc, const long &lastMarkedEmptyDiff, AddOrRem ar);

    static long NextDifference(const std::vector<long> &linesWithDifferences, int currline);
    static bool CanGotoNextDiff(const std::vector<long> &linesWithDifferences, int currline);
    static long PrevDifference(int curr_line, const std::vector<long> &linesWithDifferences, std::map<long, Block> &changes);
    static bool CanGotoPrevDiff(int curr_line, std::vector<long> linesWithDifferences, std::map<long, Block> &changes);
    void ShowDiff(const wxDiff &diff, bool reloadFile);

    cbEditor *GetCbEditorIfActive(const wxString &filename);

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

    std::vector<long> linesRightWithDifferences_;
    std::map<long, Block> rightChanges_;
    std::vector<long> linesLeftWithDifferences_;
    std::map<long, Block> leftChanges_;

    int lineNumbersWidthLeft_;
    int lineNumbersWidthRight_;
    static void doCopy(const Block &srcBlock, const Block &dstBlock, long &lastSrcMarkedDiff, long &lastDstMarkedDiff, cbStyledTextCtrl *tcSrc, cbStyledTextCtrl *tcDst);
    void CopyTo(bool toRight);
    bool HasDiffSelected();

    static void setLineNumberMarginWidth(cbStyledTextCtrl *stc, int &currWidth);

    int vscrollpos_;
    int hscrollpos_;

    LineChangedTimer *timer_;
    DECLARE_EVENT_TABLE()
};

#endif
