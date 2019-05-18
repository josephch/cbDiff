#ifndef CBDIFFCTRL_H
#define CBDIFFCTRL_H

#include <wx/panel.h>

#include <configmanager.h>
#include <editorcolourset.h>

#include "cbDiffEditor.h"
#include "wxDiff.h"

class cbDiffCtrl: public wxPanel
{
public:
    cbDiffCtrl(cbDiffEditor* parent);
    virtual ~cbDiffCtrl();
    virtual void Init(cbDiffColors colset, bool left_read_only=true, bool right_read_only=true) = 0;
    virtual void ShowDiff(wxDiff diff) = 0;

    virtual bool GetModified() const = 0;
    virtual bool QueryClose() = 0;
    virtual bool Save() = 0;
//    bool LeftReadOnly(){return leftReadOnly_;}
//    bool RightReadOnly(){return rightReadOnly_;}
    virtual bool LeftModified() = 0;
    virtual bool RightModified() = 0;
protected:
    cbDiffEditor* parent_;
    EditorColourSet *m_theme;
    wxString leftFilename_;
    wxString rightFilename_;
    bool leftReadOnly_;
    bool rightReadOnly_;
private:
    DECLARE_EVENT_TABLE()
};

#endif
