#ifndef WXDIFF_H
#define WXDIFF_H

#include <wx/wx.h>

#include <vector>
#include <map>

class wxDiff
{
public:
    wxDiff(wxString filename1, wxString filename2);
    virtual ~wxDiff(){}

    wxString IsDifferent();

    wxString GetDiff();
    std::map<long, int> GetAddedLines();
    std::map<long, int> GetRemovedLines();
    std::map<long, int> GetLeftEmptyLines();
    std::map<long, int> GetRightEmptyLines();
    std::map<long, long> GetLinePositions();
    wxString GetFromFilename();
    wxString GetToFilename();
private:
    wxString CreateHeader();
    void ParseDiff(std::vector<wxArrayString> diffs);

    wxString m_filename1;
    wxString m_filename2;
    wxString m_diff;
    wxArrayString m_diff_lines;
    std::map<long, int> m_added_lines;
    std::map<long, int> m_removed_lines;
    std::map<long, int> m_left_empty_lines;
    std::map<long, int> m_right_empty_lines;
    std::map<long, long> m_line_pos;
};

#endif
