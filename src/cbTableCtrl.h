#ifndef CBTABLECTRL_H
#define CBTABLECTRL_H

#include "cbDiffCtrl.h"

class cbStyledTextCtrl;

class cbTableCtrl : public cbDiffCtrl
{
public:
    cbTableCtrl(wxWindow* parent);
    virtual ~cbTableCtrl(){}
    virtual void Init(cbDiffColors colset) override;
    virtual void ShowDiff(wxDiff diff) override;
private:
    cbStyledTextCtrl* m_txtctrl;
};

#endif
