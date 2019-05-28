#ifndef CBUNIFIEDCTRL_H
#define CBUNIFIEDCTRL_H

#include "cbDiffCtrl.h"

class cbStyledTextCtrl;

class cbUnifiedCtrl : public cbDiffCtrl
{
public:
    cbUnifiedCtrl(cbDiffEditor *parent);
    virtual ~cbUnifiedCtrl(){}
    virtual void Init(cbDiffColors colset) override;
    virtual void ShowDiff(wxDiff diff) override;

    virtual void NextDifference()override{}
    virtual void PrevDifference()override{}
    virtual bool CanGotoNextDiff()override{return false;}
    virtual bool CanGotoPrevDiff()override{return false;}
    virtual void FirstDifference()override{}
    virtual void LastDifference()override{}
    virtual bool CanGotoFirstDiff()override{return false;}
    virtual bool CanGotoLastDiff()override{return false;}

    virtual bool GetModified() const override{return false;}
    virtual bool QueryClose() override{return true;}
    virtual bool Save() override{return true;}

    virtual void Copy()override;
    virtual bool HasSelection() const override;
    virtual bool CanSelectAll() const override;
    virtual void SelectAll() override;
protected:
    virtual bool LeftModified() override{return false;}
    virtual bool RightModified() override{return false;}

private:
    cbStyledTextCtrl *txtctrl_;
};

#endif
