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
protected:
    virtual bool LeftModified() override{return false;}
    virtual bool RightModified() override;
private:
    void OnEditorChange(wxScintillaEvent &event);
    cbStyledTextCtrl* m_txtctrl;
    int lineNumbersWidthRight;
    void setLineNumberMarginWidth();
};

#endif
