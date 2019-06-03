#ifndef DIFFEDITOR_H
#define DIFFEDITOR_H

#include <editorbase.h>
#include <set>

class wxBitmapButton;
class cbDiffCtrl;

//! Margin markers
#define PLUS_MARKER          1
#define MINUS_MARKER         2
#define EQUAL_MARKER         3
#define RED_BKG_MARKER       4
#define GREEN_BKG_MARKER     5
#define GREY_BKG_MARKER      6
#define CARET_LINE_MARKER    7
#define BLUE_BKG_MARKER      8

class cbDiffColors
{
public:
    wxString hlang_;       /// the highlightlanguage in Table/SidebySide mode
    wxColour addedlines_;
    wxColour removedlines_;
    wxColour selectedlines_;
    int caretlinetype_;
    wxColour caretline_;
};

class cbDiffEditor : public EditorBase
{
public:

    cbDiffEditor(const wxString &leftFile, const wxString &rightFile, int diffmode = DEFAULT, bool leftReadOnly = true, bool rightReadOnly = true);

    virtual ~cbDiffEditor();

    bool SaveAsUnifiedDiff();
    static void CloseAllEditors();

    virtual bool GetModified() const override;
    virtual bool QueryClose() override;
    virtual bool Save() override;
    virtual bool SaveAs()override{return SaveAsUnifiedDiff();}

    virtual bool VisibleToTree() const override{ return false; }


    virtual void Undo()override;
    virtual void Redo()override;
 /** Clear Undo- (and Changebar-) history */
    virtual void ClearHistory()override;
    virtual void Cut()override;
    virtual void Copy()override;
    virtual void Paste()override;
    virtual bool CanUndo() const override;
    virtual bool CanRedo() const override;
    virtual bool HasSelection() const override;
    virtual bool CanPaste() const override;
    virtual bool CanSelectAll() const override;
    virtual void SelectAll()override;

//    /** @return True if the editor is read-only, false if not.*/
//    virtual bool IsReadOnly() const override;
//    /** @param readonly If true, mark as readonly. If false, mark as read-write.*/
//    virtual void SetReadOnly(bool /*readonly*/ = true)override;

    void Swap();
    void Reload();
    int GetMode();
    void SetMode(int mode);

    void NextDifference();
    bool CanGotoNextDiff();
    void PrevDifference();
    bool CanGotoPrevDiff();

    void FirstDifference();
    void LastDifference();
    bool CanGotoFirstDiff();
    bool CanGotoLastDiff();

    void CopyToLeft();
    bool CanCopyToLeft();
    void CopyToRight();
    bool CanCopyToRight();
    void CopyToLeftNext();
    bool CanCopyToLeftNext();
    void CopyToRightNext();
    bool CanCopyToRightNext();

    enum
    {
        DEFAULT = -1,
        TABLE = 0,
        UNIFIED,
        SIDEBYSIDE
    };

    void updateTitle();
private:
    void MarkReadOnly();
    void InitDiffCtrl(int mode);
    void ShowDiff();        /// Makes the diff and shows it

    cbDiffCtrl *diffctrl_;

    wxString leftFile_;
    wxString rightFile_;
    int viewingmode_;      /// the diff viewingmode currently Table, Unified and SideBySide
    wxString diff_;        /// the unified diff file, used to save

    cbDiffColors colorset_;

    void OnContextMenu(wxContextMenuEvent& event){} /// Just override it for now

    bool leftReadOnly_;
    bool rightReadOnly_;

    /// static Members (thx to Bartlomiej Swiecki for hexedit!)
    typedef std::set< EditorBase* > EditorsSet;
    ///< \brief Set of all opened editors,
    ///   used to close all editors when plugin is being unloaded
    static EditorsSet        allEditors_;
    DECLARE_EVENT_TABLE()
};

#endif
