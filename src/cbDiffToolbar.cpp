#include "cbDiffToolbar.h"

#include "cbDiffEditor.h"

#include <manager.h>
#include <editormanager.h>
#include <logmanager.h>
#include "cbDiffUtils.h"

/// bitmaps
#include "../images/swap.h"
#include "../images/reload.h"
#include "../images/table.h"
#include "../images/unified.h"
#include "../images/sidebyside.h"



BEGIN_EVENT_TABLE(cbDiffToolbar, wxEvtHandler)
END_EVENT_TABLE()

cbDiffToolbar::cbDiffToolbar(cbDiffEditor* parent, int viewmode):
    wxPanel(parent),
    m_parent(parent)
{
#if ( wxCHECK_VERSION(3, 0, 0) || isUseWxAuiToolbar || isUseWxToolbar )
    // create some toolbars
    #if isUseWxAuiToolbar
    toolbar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_TEXT);
    #else
    toolbar = new wxToolBar(this, wxID_ANY);
    #endif
    toolbar->AddTool(cbDiffEditor::TABLE, wxT("Table"), wxGetBitmapFromMemory(table), wxT("Display as a table"), wxITEM_NORMAL);
    toolbar->AddTool(cbDiffEditor::UNIFIED, wxT("Unified"), wxGetBitmapFromMemory(unified), wxT("Display as unified diff"), wxITEM_NORMAL);
    toolbar->AddTool(cbDiffEditor::SIDEBYSIDE, wxT("Side by Side"), wxGetBitmapFromMemory(sidebyside), wxT("Display side by side"), wxITEM_NORMAL);
    toolbar->AddSeparator();
    toolbar->AddTool(ID_BBRELOAD, wxT("Reload"), wxGetBitmapFromMemory(reload), wxT("Reload files"), wxITEM_NORMAL);
    toolbar->AddTool(ID_BBSWAP, wxT("Swap"), wxGetBitmapFromMemory(swap), wxT("Swap files"), wxITEM_NORMAL);
#else
    BBTable = new wxBitmapButton(this, ID_BUTTON_TABLE, wxGetBitmapFromMemory(table), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBTable->SetToolTip(_("Display as a table"));

    BBUnified = new wxBitmapButton(this, ID_BUTTON_UNIFIED, wxGetBitmapFromMemory(unified), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBUnified->SetToolTip(_("Display as unified diff"));

    BBSideBySide = new wxBitmapButton(this, ID_BUTTON_SIDEBYSIDE, wxGetBitmapFromMemory(sidebyside), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBSideBySide->SetToolTip(_("Display side by side"));

    wxBitmapButton* BBReload = new wxBitmapButton(this, ID_BBRELOAD, wxGetBitmapFromMemory(reload), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBReload->SetToolTip(_("Reload files"));

    wxBitmapButton* BBSwap = new wxBitmapButton(this, ID_BBSWAP, wxGetBitmapFromMemory(swap), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    BBSwap->SetToolTip(_("Swap files"));
#endif

    wxBoxSizer* boxsizer = new wxBoxSizer(wxHORIZONTAL);
#if ( wxCHECK_VERSION(3, 0, 0) || isUseWxAuiToolbar || isUseWxToolbar )
    // after adding the buttons to the toolbar, must call Realize() to reflect
    // the changes
    #if ( wxCHECK_VERSION(3, 0, 0) && isUseWxAuiToolbar && !defined(_WIN32) )
    toolbar->SetInitialSize();
    #endif
    toolbar->Realize();
    #if ! isUseWxAuiToolbar
    toolbar->SetInitialSize();
    #endif

    boxsizer->Add(toolbar, 0, wxALL|wxEXPAND, 5);
    SetSizer(boxsizer);
#else
    boxsizer->Add(BBTable, 0, wxALL, 5);
    boxsizer->Add(BBUnified, 0, wxALL, 5);
    boxsizer->Add(BBSideBySide, 0, wxALL, 5);
    boxsizer->Add(-1,-1,0, wxALL, 5);
    boxsizer->Add(BBReload, 0, wxALL, 5);
    boxsizer->Add(BBSwap, 0, wxALL, 5);
    SetSizer(boxsizer);
    boxsizer->Layout();
#endif

    switch (viewmode)
    {
#if ( wxCHECK_VERSION(3, 0, 0) || isUseWxAuiToolbar || isUseWxToolbar )
    case cbDiffEditor::TABLE:
    case cbDiffEditor::UNIFIED:
    case cbDiffEditor::SIDEBYSIDE:
        toolbar->EnableTool(viewmode, false);
        break;
#else
    case cbDiffEditor::TABLE:
        BBTable->Enable(false);
        break;
    case cbDiffEditor::UNIFIED:
        BBUnified->Enable(false);
        break;
    case cbDiffEditor::SIDEBYSIDE:
        BBSideBySide->Enable(false);
        break;
#endif
    default:
        break;
    }

#if ( wxCHECK_VERSION(3, 0, 0) || isUseWxAuiToolbar || isUseWxToolbar )
    Connect(wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&cbDiffToolbar::OnButton);
#else
    Connect(wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&cbDiffToolbar::OnButton);
#endif
}

void cbDiffToolbar::OnButton(wxCommandEvent& event)
{
    if(event.GetId() == ID_BBSWAP)
    {
        m_parent->Swap();
        return;
    }
    else if(event.GetId() != m_parent->GetMode() &&
           (event.GetId() == cbDiffEditor::TABLE ||
            event.GetId() == cbDiffEditor::UNIFIED ||
            event.GetId() == cbDiffEditor::SIDEBYSIDE))
    {
#if ( wxCHECK_VERSION(3, 0, 0) || isUseWxAuiToolbar || isUseWxToolbar )
        toolbar->EnableTool(cbDiffEditor::TABLE, true);
        toolbar->EnableTool(cbDiffEditor::UNIFIED, true);
        toolbar->EnableTool(cbDiffEditor::SIDEBYSIDE, true);
#else
        BBTable->Enable();
        BBUnified->Enable();
        BBSideBySide->Enable();
#endif
        m_parent->SetMode(event.GetId());
        switch (event.GetId())
        {
        case cbDiffEditor::TABLE:
#if ( wxCHECK_VERSION(3, 0, 0) || isUseWxAuiToolbar || isUseWxToolbar )
            toolbar->EnableTool(cbDiffEditor::TABLE, false);
#else
            BBTable->Enable(false);
#endif
            break;
        case cbDiffEditor::UNIFIED:
#if ( wxCHECK_VERSION(3, 0, 0) || isUseWxAuiToolbar || isUseWxToolbar )
            toolbar->EnableTool(cbDiffEditor::UNIFIED, false);
#else
            BBUnified->Enable(false);
#endif
            break;
        case cbDiffEditor::SIDEBYSIDE:
#if ( wxCHECK_VERSION(3, 0, 0) || isUseWxAuiToolbar || isUseWxToolbar )
            toolbar->EnableTool(cbDiffEditor::SIDEBYSIDE, false);
#else
            BBSideBySide->Enable(false);
#endif
            break;
        default:
            break;
        }
    }
    m_parent->Reload();
}
