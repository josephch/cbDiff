#ifndef WXDIFF_H
#define WXDIFF_H

#include <wx/wx.h>

#include <vector>
#include <map>

class wxDiff
{
public:
    wxDiff(wxString filename1, wxString filename2, bool leftReadOnly, bool rightReadOnly);
    virtual ~wxDiff(){}

    wxString IsDifferent();

    wxString GetDiff();
    std::map<long, int> GetAddedLines();
    std::map<long, int> GetRemovedLines();
    std::map<long, int> GetLeftEmptyLines();
    std::map<long, int> GetRightEmptyLines();
    std::map<long, long> GetLinePositions();
    wxString GetLeftFilename();
    wxString GetRightFilename();
    bool RightReadOnly()const;
    bool LeftReadOnly()const;
private:
    wxString CreateHeader();
    void ParseDiff(std::vector<wxArrayString> diffs);

    wxString m_leftFilename;
    wxString m_rightFilename;
    bool m_leftReadOnly_;
    bool m_rightReadOnly_;
    wxString m_diff;
    wxArrayString m_diff_lines;
    std::map<long, int> m_added_lines;
    std::map<long, int> m_removed_lines;
    std::map<long, int> m_left_empty_lines;
    std::map<long, int> m_right_empty_lines;
    std::map<long, long> m_line_pos;
};

#endif
