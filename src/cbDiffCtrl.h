#ifndef CBDIFFCTRL_H
#define CBDIFFCTRL_H

#include <wx/panel.h>

#include <configmanager.h>
#include <editorcolourset.h>

#include "cbDiffEditor.h"
#include "wxDiff.h"

class cbDiffCtrl: public wxPanel
{
public:
    cbDiffCtrl(cbDiffEditor *parent);
    virtual ~cbDiffCtrl();
    virtual void Init(cbDiffColors colset) = 0;
    virtual void ShowDiff(wxDiff diff) = 0;

    virtual void NextDifference() = 0;
    virtual void PrevDifference() = 0;
    virtual bool CanGotoNextDiff() = 0;
    virtual bool CanGotoPrevDiff() = 0;

    virtual bool GetModified() const = 0;
    virtual bool QueryClose() = 0;
    virtual bool Save() = 0;
//    bool LeftReadOnly(){return leftReadOnly_;}
//    bool RightReadOnly(){return rightReadOnly_;}
    virtual bool LeftModified() = 0;
    virtual bool RightModified() = 0;

    virtual void Undo(){}
    virtual void Redo(){}
    virtual void ClearHistory(){}/** Clear Undo- (and Changebar-) history */
    virtual void Cut(){}
    virtual void Copy()=0;
    virtual void Paste(){}
    virtual bool CanUndo() const {return false;}
    virtual bool CanRedo() const {return false;}
    virtual bool HasSelection() const = 0;
    virtual bool CanPaste() const {return false;}
    virtual bool CanSelectAll() const = 0;
    virtual void SelectAll() = 0;

protected:
    cbDiffEditor *parent_;
    EditorColourSet *m_theme;
    wxString leftFilename_;
    wxString rightFilename_;
    bool leftReadOnly_;
    bool rightReadOnly_;
private:
    DECLARE_EVENT_TABLE()
};

#endif
