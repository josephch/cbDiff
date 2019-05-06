#ifndef CBDIFF_H
#define CBDIFF_H

#include <wx/string.h>
#include <cbplugin.h>

class cbConfigurationPanel;

#ifdef EXPORT_FUNC
    #define EXPORT_FFP WXEXPORT
#else
    #define EXPORT_FFP WXIMPORT
#endif

extern "C" EXPORT_FFP void DiffFiles(const wxString& firstfile, const wxString& secondfile, int viewmode);

class cbDiff : public cbPlugin
{
    public:
        cbDiff();
        virtual ~cbDiff(){}

        virtual int GetConfigurationPriority() const  override{ return 50; }
        virtual int GetConfigurationGroup() const  override{ return cgUnknown; }
        virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent) override;
        virtual cbConfigurationPanel* GetProjectConfigurationPanel(wxWindow* parent, cbProject* project) override{ return 0; }
        virtual void BuildMenu(wxMenuBar* menuBar) override;
        virtual void BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data = 0) override;
        virtual bool BuildToolBar(wxToolBar* toolBar) override{ return false; }

    protected:
        virtual void OnAttach() override;
        virtual void OnRelease(bool appShutDown) override;

        void OnMenuDiffFiles(wxCommandEvent& event);
        void OnContextDiffFiles(wxCommandEvent& event);
        void OnAppDoneStartup(CodeBlocksEvent& event);
        void OnAppCmdLine(CodeBlocksEvent& event);
        void EvalCmdLine();
        bool m_prevSelectionValid;
        wxString m_prevFileName;
        std::vector<long> MenuIds;
        struct FileNames{
            wxString file1, file2;
        };
        FileNames m_names;

    private:
        DECLARE_EVENT_TABLE();
};

#endif
