#include "cbDiffCtrl.h"

BEGIN_EVENT_TABLE(cbDiffCtrl, wxPanel)
END_EVENT_TABLE()

cbDiffCtrl::cbDiffCtrl(cbDiffEditor* parent):
    wxPanel(parent, -1, wxPoint(1000, 1000)),
    parent_(parent),
    m_theme(nullptr)
{
    m_theme = new EditorColourSet( Manager::Get()->GetConfigManager(_T("editor"))->Read(_T("/colour_sets/active_colour_set"), COLORSET_DEFAULT));
}

cbDiffCtrl::~cbDiffCtrl()
{
    wxDELETE(m_theme);
}
