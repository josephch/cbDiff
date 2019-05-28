#ifndef CBDIFF_H
#define CBDIFF_H

#include <wx/string.h>
#include <wx/toolbar.h>
#include <cbplugin.h>

class cbConfigurationPanel;

#ifdef EXPORT_FUNC
    #define EXPORT_FFP WXEXPORT
#else
    #define EXPORT_FFP WXIMPORT
#endif

extern "C" EXPORT_FFP void DiffFiles(const wxString &firstfile, const wxString &secondfile, int viewmode, bool left_readonly, bool right_readonly);

class cbDiff : public cbPlugin
{
    public:
        cbDiff();
        virtual ~cbDiff(){}

        virtual int GetConfigurationPriority() const  override{ return 50; }
        virtual int GetConfigurationGroup() const  override{ return cgUnknown; }
        virtual cbConfigurationPanel *GetConfigurationPanel(wxWindow *parent) override;
        virtual cbConfigurationPanel *GetProjectConfigurationPanel(wxWindow *parent, cbProject *project) override{ return 0; }
        virtual void BuildMenu(wxMenuBar *menuBar) override;
        virtual void BuildModuleMenu(const ModuleType type, wxMenu *menu, const FileTreeData *data = 0) override;

    protected:
        virtual void OnAttach() override;
        virtual void OnRelease(bool appShutDown) override;

        void OnMenuDiffFiles(wxCommandEvent &event);
        void OnContextDiffFiles(wxCommandEvent &event);
        void OnMenuSaveAsUnifiedDiff(wxCommandEvent &event);
        void OnUpdateUiSaveAsUnifiedDiff(wxUpdateUIEvent &event);
        void OnNextDifference(wxCommandEvent &event);
        void OnPrevDifference(wxCommandEvent &event);
        void OnUpdateNextDifference(wxUpdateUIEvent &event);
        void OnUpdatePrevDifference(wxUpdateUIEvent &event);
        void OnAppDoneStartup(CodeBlocksEvent &event);
        void OnAppCmdLine(CodeBlocksEvent &event);
        void EvalCmdLine();
        void OnSwitchView(wxCommandEvent &event);
        void OnUpdateSwitchView(wxUpdateUIEvent &event);
        void OnReloadFiles(wxCommandEvent &event);
        void OnUpdateReloadFiles(wxUpdateUIEvent &event);
        void OnSwapFiles(wxCommandEvent &event);
        void OnUpdateSwapFiles(wxUpdateUIEvent &event);

        void OnFirstDifference(wxCommandEvent &event);
        void OnUpdateFirstDifference(wxUpdateUIEvent &event);
        void OnLastDifference(wxCommandEvent &event);
        void OnUpdateLastDifference(wxUpdateUIEvent &event);



        bool m_prevSelectionValid;
        wxString m_prevFileName;
        std::vector<long> menuIds_;
        struct FileNames{
            wxString file1, file2;
        };
        FileNames names_;

        virtual bool BuildToolBar(wxToolBar* toolBar)override;
        wxToolBar* toolbar_;
    private:
        DECLARE_EVENT_TABLE();
};

#endif
