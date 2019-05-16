#ifndef CBSIDEBYSIDECTRL_H
#define CBSIDEBYSIDECTRL_H

#include "cbDiffCtrl.h"

class LineChangedTimer;
class cbStyledTextCtrl;

class cbSideBySideCtrl : public cbDiffCtrl
{
public:
    cbSideBySideCtrl(wxWindow* parent);
    virtual ~cbSideBySideCtrl();
    virtual void Init(cbDiffColors colset, bool left_read_only=true, bool right_read_only=true) override;
    virtual void ShowDiff(wxDiff diff) override;
    void Synchronize();
private:
    cbStyledTextCtrl* TCLeft;
    cbStyledTextCtrl* TCRight;

    wxScrollBar* VScrollBar;
    wxScrollBar* HScrollBar;

    int lineNumbersWidthLeft;
    int lineNumbersWidthRight;

    bool left_read_only_;
    bool right_read_only_;

    static void setLineNumberMarginWidth(cbStyledTextCtrl* stc, int &currWidth);

    int m_vscrollpos;
    int m_hscrollpos;

    LineChangedTimer* m_timer;
    DECLARE_EVENT_TABLE()
};

#endif
