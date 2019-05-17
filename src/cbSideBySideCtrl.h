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
    virtual void Init(cbDiffColors colset, bool leftReadOnly = true, bool rightReadOnly = true) override;
    virtual void ShowDiff(wxDiff diff) override;
    void Synchronize();

    virtual bool GetModified() const override;
    virtual bool QueryClose() override;
    virtual bool Save() override;
private:
    bool SaveLeft();
    bool SaveRight();
    cbStyledTextCtrl* TCLeft;
    cbStyledTextCtrl* TCRight;

    wxScrollBar* VScrollBar;
    wxScrollBar* HScrollBar;

    int lineNumbersWidthLeft;
    int lineNumbersWidthRight;

    bool leftReadOnly_;
    bool rightReadOnly_;
    wxString leftFilename_;
    wxString rightFilename_;

    static void setLineNumberMarginWidth(cbStyledTextCtrl* stc, int &currWidth);

    int m_vscrollpos;
    int m_hscrollpos;

    LineChangedTimer* m_timer;
    DECLARE_EVENT_TABLE()
};

#endif
