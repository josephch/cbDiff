#ifndef CBUNIFIEDCTRL_H
#define CBUNIFIEDCTRL_H

#include "cbDiffCtrl.h"

class cbStyledTextCtrl;

class cbUnifiedCtrl : public cbDiffCtrl
{
public:
    cbUnifiedCtrl(wxWindow* parent);
    virtual ~cbUnifiedCtrl(){}
    virtual void Init(cbDiffColors colset, bool, bool) override;
    virtual void ShowDiff(wxDiff diff) override;
private:
    cbStyledTextCtrl* m_txtctrl;
};

#endif
