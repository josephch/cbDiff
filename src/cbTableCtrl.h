#ifndef CBTABLECTRL_H
#define CBTABLECTRL_H

#include "cbDiffCtrl.h"

class cbStyledTextCtrl;

class cbTableCtrl : public cbDiffCtrl
{
public:
    cbTableCtrl(cbDiffEditor* parent);
    virtual ~cbTableCtrl(){}
    virtual void Init(cbDiffColors colset, bool, bool rightReadOnly) override;
    virtual void ShowDiff(wxDiff diff) override;
    virtual bool GetModified() const override;
    virtual bool QueryClose() override;
    virtual bool Save() override;

    virtual void Undo()override;
    virtual void Redo()override;
    virtual void ClearHistory()override;/** Clear Undo- (and Changebar-) history */
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
    virtual bool LeftModified() override{return false;}
    virtual bool RightModified() override;
private:
    void OnEditorChange(wxScintillaEvent &event);
    cbStyledTextCtrl *m_txtctrl;
    int lineNumbersWidthRight;
    void setLineNumberMarginWidth();
};

#endif
