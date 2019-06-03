#ifndef CBTABLECTRL_H
#define CBTABLECTRL_H

#include "cbDiffCtrl.h"

class cbStyledTextCtrl;

class cbTableCtrl : public cbDiffCtrl
{
public:
    cbTableCtrl(cbDiffEditor* parent);
    virtual ~cbTableCtrl(){}
    virtual void Init(cbDiffColors colset) override;
    virtual void ShowDiff(wxDiff diff) override;

    virtual bool GetModified() const override{return false;}
    virtual bool QueryClose() override{return true;}
    virtual bool Save() override{return true;}
    virtual bool CanSelectAll() const override{return false;}
    virtual void SelectAll() override{}

    virtual void NextDifference()override;
    virtual bool CanGotoNextDiff()override;
    virtual void PrevDifference()override;
    virtual bool CanGotoPrevDiff()override;
    virtual void FirstDifference()override;
    virtual bool CanGotoFirstDiff()override;
    virtual void LastDifference()override;
    virtual bool CanGotoLastDiff()override;

    virtual void CopyToLeft()override{}
    virtual bool CanCopyToLeft()override{return false;}
    virtual void CopyToRight()override{}
    virtual bool CanCopyToRight()override{return false;}
    virtual void CopyToLeftNext()override{}
    virtual bool CanCopyToLeftNext()override{return false;}
    virtual void CopyToRightNext()override{}
    virtual bool CanCopyToRightNext()override{return false;}

    virtual void Copy()override;
    virtual bool HasSelection() const override;
protected:
    virtual bool LeftModified() override{return false;}
    virtual bool RightModified() override{return false;}
private:
    std::vector<long> linesWithDifferences_;
    cbStyledTextCtrl *txtctrl_;
};

#endif
