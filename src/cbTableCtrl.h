#ifndef CBTABLECTRL_H
#define CBTABLECTRL_H

#include "cbDiffCtrl.h"

class cbStyledTextCtrl;

class cbTableCtrl : public cbDiffCtrl
{
public:
    cbTableCtrl(wxWindow* parent);
    virtual ~cbTableCtrl(){}
    virtual void Init(cbDiffColors colset, bool, bool rightReadOnly) override;
    virtual void ShowDiff(wxDiff diff) override;
    virtual bool GetModified() const override;
    virtual bool QueryClose() override;
    virtual bool Save() override;
private:
    cbStyledTextCtrl* m_txtctrl;
    wxString rightFilename_;
    int lineNumbersWidthRight;
    bool rightReadOnly_;
    bool closeUnsaved_;
    void setLineNumberMarginWidth();
};

#endif
