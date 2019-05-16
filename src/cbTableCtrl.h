#ifndef CBTABLECTRL_H
#define CBTABLECTRL_H

#include "cbDiffCtrl.h"

class cbStyledTextCtrl;

class cbTableCtrl : public cbDiffCtrl
{
public:
    cbTableCtrl(wxWindow* parent);
    virtual ~cbTableCtrl(){}
    virtual void Init(cbDiffColors colset, bool, bool right_read_only) override;
    virtual void ShowDiff(wxDiff diff) override;
private:
    cbStyledTextCtrl* m_txtctrl;
    int lineNumbersWidthRight;
    bool right_read_only_;
    void setLineNumberMarginWidth();
};

#endif
