#ifndef DIFFEDITOR_H
#define DIFFEDITOR_H

#include <editorbase.h>
#include <set>

class wxBitmapButton;
class cbDiffCtrl;

//! Margin markers
#define PLUS_MARKER          1
#define MINUS_MARKER         2
#define EQUAL_MARKER         3
#define RED_BKG_MARKER       4
#define GREEN_BKG_MARKER     5
#define GREY_BKG_MARKER      6
#define CARET_LINE_MARKER    7

class cbDiffColors
{
public:
    wxString m_hlang;       /// the highlightlanguage in Table/SidebySide mode
    wxColour m_addedlines;
    wxColour m_removedlines;
    int m_caretlinetype;
    wxColour m_caretline;
};

class cbDiffEditor : public EditorBase
{
public:

    cbDiffEditor(const wxString& firstfile, const wxString& secondfile, int diffmode = DEFAULT, bool leftReadOnly = true, bool rightReadOnly = true);

    virtual ~cbDiffEditor();

    bool SaveAsUnifiedDiff();
    static void CloseAllEditors();

    virtual bool GetModified() const override;
    virtual bool QueryClose() override;
    virtual bool Save() override;
    //virtual const wxString &GetShortName() const override;

    virtual bool VisibleToTree() const override{ return false; }

    void Swap();
    void Reload();
    int GetMode();
    void SetMode(int mode);

    enum
    {
        DEFAULT = -1,
        TABLE = 0,
        UNIFIED,
        SIDEBYSIDE
    };

    void updateTitle();
private:
    void InitDiffCtrl(int mode, bool leftReadOnly, bool rightReadOnly);
    void ShowDiff();        /// Makes the diff and shows it

    /// Diff properties
    wxString m_fromfile;    /// File to diff from
    wxString m_tofile;      /// File to diff to
    int m_viewingmode;      /// the diff viewingmode currently Table, Unified and SidebySide
    wxString m_diff;        /// the unified diff file, used to save

    /// graphical properties
    cbDiffColors m_colorset;

    /// Events
    void OnContextMenu(wxContextMenuEvent& event){} /// Just override it for now

    /// Controls
    cbDiffCtrl *m_diffctrl;
    bool leftReadOnly_;
    bool rightReadOnly_;

    /// static Members (thx to Bartlomiej Swiecki for hexedit!)
    typedef std::set< EditorBase* > EditorsSet;
    ///< \brief Set of all opened editors,
    ///   used to close all editors when plugin is being unloaded
    static EditorsSet        m_AllEditors;
    DECLARE_EVENT_TABLE()
};

#endif
