
#include "cbDiff.h"

#include <sdk.h>
#include <configurationpanel.h>
#include <cbproject.h>
#include <editormanager.h>
#include <logmanager.h>
#include <cbeditor.h>
#include <configmanager.h>

#include "cbDiffMenu.h"
#include "cbDiffEditor.h"
#include "cbDiffUtils.h"
#include "cbDiffSelectFiles.h"
#include "cbDiffConfigPanel.h"

// Register the plugin with Code::Blocks.
namespace
{
    PluginRegistrant<cbDiff> reg(_T("cbDiff"));
    const int ID_MENU_DIFF_FILES        = wxNewId();
    const int ID_CONTEXT_DIFF_TWO_FILES = wxNewId();
    const int ID_MENU_SAVE_UNIFIED_DIFF = wxNewId();
}

/// Function for other plugins
EXPORT_FFP void DiffFiles(const wxString &firstfile, const wxString &secondfile, int viewmode, bool leftReadOnly, bool rightReadOnly)
{
    new cbDiffEditor(firstfile, secondfile, viewmode, leftReadOnly, rightReadOnly);
}

// events handling
BEGIN_EVENT_TABLE(cbDiff, cbPlugin)
    EVT_MENU        (ID_MENU_DIFF_FILES,        cbDiff::OnMenuDiffFiles)
    EVT_MENU        (ID_CONTEXT_DIFF_TWO_FILES, cbDiff::OnContextDiffFiles)
    EVT_MENU        (ID_MENU_SAVE_UNIFIED_DIFF, cbDiff::OnMenuSaveAsUnifiedDiff)
    EVT_UPDATE_UI   (ID_MENU_SAVE_UNIFIED_DIFF, cbDiff::OnUpdateUiSaveAsUnifiedDiff)
END_EVENT_TABLE()

// constructor
cbDiff::cbDiff()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("cbDiff.zip")))
    {
        NotifyMissingFile(_T("cbDiff.zip"));
    }
}

void cbDiff::OnAttach()
{
    m_prevSelectionValid = false;
    wxString m_prevFileName = wxEmptyString;

    wxCmdLineParser& parser = *Manager::GetCmdLineParser();
    parser.AddOption( _T("diff1"), _T("diff1"),_T("first file to compare"));
    parser.AddOption( _T("diff2"), _T("diff2"),_T("second file to compare"));

    Manager::Get()->RegisterEventSink(cbEVT_APP_STARTUP_DONE, new cbEventFunctor<cbDiff, CodeBlocksEvent>(this, &cbDiff::OnAppDoneStartup));
    Manager::Get()->RegisterEventSink(cbEVT_APP_CMDLINE, new cbEventFunctor<cbDiff, CodeBlocksEvent>(this, &cbDiff::OnAppCmdLine));
}

void cbDiff::OnAppDoneStartup(CodeBlocksEvent &event)
{
    EvalCmdLine();
    event.Skip();
}

void cbDiff::OnAppCmdLine(CodeBlocksEvent &event)
{
    EvalCmdLine();
    event.Skip();
}

void cbDiff::OnRelease(bool appShutDown)
{
    if (!appShutDown)
        cbDiffEditor::CloseAllEditors();
}

cbConfigurationPanel *cbDiff::GetConfigurationPanel(wxWindow *parent)
{
    if(!m_IsAttached)
        return NULL;
    return new cbDiffConfigPanel(parent);
}

void cbDiff::BuildMenu(wxMenuBar *menuBar)
{
    int fileMenuIndex = menuBar->FindMenu( _("&File") );
    if(fileMenuIndex == wxNOT_FOUND )
        return;

    wxMenu* fileMenu = menuBar->GetMenu(fileMenuIndex);
    if(!fileMenu)
        return;

    wxMenuItemList& list = fileMenu->GetMenuItems();
    int pos = 0;
    // Search for the Recent files entry and insert the diff entry after it
    for(wxMenuItemList::iterator i = list.begin(); i != list.end(); ++i, ++pos)
    {
        wxMenuItem* item = *i;
#if wxCHECK_VERSION(2, 9, 0)
        wxString label = item->GetItemLabelText();
#else
        wxString label = item->GetLabel();
#endif
        label.Replace( _T("_"), _T("") );
        if (label.Contains( _("Recent files")))
        {
            fileMenu->InsertSeparator(pos + 1);
            fileMenu->Insert(pos + 2, ID_MENU_DIFF_FILES, _("Diff files..."), _("Shows the differences between two files"));
            fileMenu->Insert(pos + 3, ID_MENU_SAVE_UNIFIED_DIFF, _("Save as unified diff file"), _("Saves the current diff as unified diff file"));
            return;
        }
    }
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_MENU_DIFF_FILES,        _("Diff files..."), _("Shows the differences between two files"));
    fileMenu->Append(ID_MENU_SAVE_UNIFIED_DIFF, _("Save as unified diff file"), _("Saves the current diff as unified diff file"));
}

void cbDiff::BuildModuleMenu(const ModuleType type, wxMenu *menu, const FileTreeData *data)
{
    if ( type == mtProjectManager && data != 0 && data->GetKind() == FileTreeData::ftdkFile )
    {
        ProjectFile *prjFile = data->GetProjectFile();
        if( prjFile )
        {
            wxMenu *diffmenu = new cbDiffMenu(this, prjFile->file.GetFullPath(), m_prevSelectionValid, m_prevFileName, MenuIds);
            menu->AppendSubMenu(diffmenu, _("Diff with"));
        }
    }
    else if ( type == mtEditorManager && Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor() )
    {
        wxString filename = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor()->GetFilename();
        wxMenu *diffmenu = new cbDiffMenu(this, filename, m_prevSelectionValid, m_prevFileName, MenuIds);
        menu->AppendSubMenu(diffmenu, _("Diff with"));
    }
    else if ( type == mtEditorTab && Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor() )
    {
        wxString filename = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor()->GetFilename();
        wxMenu *diffmenu = new cbDiffMenu(this, filename, m_prevSelectionValid, m_prevFileName, MenuIds);
        menu->AppendSubMenu(diffmenu, _("Diff with"));
    }
    else if ( type == mtUnknown ) // assuming FileExplorer
    {
        if ( data && (data->GetKind() == FileTreeData::ftdkFile) )
        {
            wxFileName f(data->GetFolder());
            wxString filename=f.GetFullPath();
            wxString name=f.GetFullName();
            menu->AppendSubMenu(new cbDiffMenu(this, name, m_prevSelectionValid, m_prevFileName, MenuIds), _("Diff with"));
	    }
	    else if( data && (data->GetKind() == FileTreeData::ftdkVirtualGroup) )
        {
            wxString paths = data->GetFolder(); //get folder contains a space separated list of the files/directories selected
            if( paths.Find('*') != wxNOT_FOUND && paths.Find('*') == paths.Find('*', true) )
            {
                m_names.file1 = paths.BeforeFirst('*');
                m_names.file2 = paths.AfterFirst('*');
                if( wxFileName::Exists(m_names.file1) && wxFileName::Exists(m_names.file2) )
                    menu->Append(ID_CONTEXT_DIFF_TWO_FILES, _("Diff"), _("Shows the differences between two files"));
            }
        }
    }
}

void cbDiff::OnMenuDiffFiles(wxCommandEvent &event)
{
    if(!m_IsAttached)
        return;

    cbDiffSelectFiles sdf(Manager::Get()->GetAppWindow());

    if(sdf.ShowModal() == wxID_OK && wxFile::Exists(sdf.GetFromFile()) && wxFile::Exists(sdf.GetToFile()))
        new cbDiffEditor(sdf.GetFromFile(), sdf.GetToFile(), sdf.GetViewingMode());
}

void cbDiff::OnContextDiffFiles(wxCommandEvent &event)
{
    if ( wxFileName::Exists(m_names.file1) && wxFileName::Exists(m_names.file2) )
    {
        int dm = cbDiffEditor::DEFAULT;
        ConfigManager *cfg = Manager::Get()->GetConfigManager(_T("cbdiffsettings"));
        if (cfg)
            dm = cfg->ReadInt(_T("viewmode"), dm);
        new cbDiffEditor(m_names.file1, m_names.file2, dm);
    }
}

void cbDiff::OnMenuSaveAsUnifiedDiff(wxCommandEvent &event)
{
    EditorManager *edMan = Manager::Get()->GetEditorManager();
    if(!edMan) return;

    cbDiffEditor *ed = dynamic_cast<cbDiffEditor *>(edMan->GetActiveEditor());
    if(!ed) return;

    ed->SaveAsUnifiedDiff();
}

void cbDiff::OnUpdateUiSaveAsUnifiedDiff(wxUpdateUIEvent &event)
{
    EditorManager *edMan = Manager::Get()->GetEditorManager();
    if(!edMan)
    {
        event.Enable(false);
        return;
    }

    cbDiffEditor *ed = dynamic_cast<cbDiffEditor *>(edMan->GetActiveEditor());
    event.Enable(ed != nullptr);
}

void cbDiff::EvalCmdLine()
{
    wxString file1, file2;
    wxCmdLineParser& parser = *Manager::GetCmdLineParser();

    if ( parser.Found(_T("diff1"), &file1) && parser.Found(_T("diff2"), &file2 ) )
    {
        if ( wxFile::Exists(file1) && wxFile::Exists(file2) )
        {
            int dm = cbDiffEditor::DEFAULT;
            ConfigManager *cfg = Manager::Get()->GetConfigManager(_T("cbdiffsettings"));
            if (cfg)
                dm = cfg->ReadInt(_T("viewmode"), dm);
            new cbDiffEditor(file1, file2, dm);
        }
    }
}

