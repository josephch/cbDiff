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

    wxString leftFilename_;
    wxString rightFilename_;
    bool leftReadOnly_;
    bool rightReadOnly_;
    wxString diff_;
    std::map<long, int>  added_lines_;
    std::map<long, int>  removed_lines_;
    std::map<long, int>  left_empty_lines_;
    std::map<long, int>  right_empty_lines_;
    std::map<long, long> line_pos_;
};

#endif
