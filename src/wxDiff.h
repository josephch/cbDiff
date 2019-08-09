#ifndef WXDIFF_H
#define WXDIFF_H

#include <wx/wx.h>

#include <vector>
#include <map>

class wxDiff
{
public:
    wxDiff(wxString filename1, wxString filename2, bool leftReadOnly, bool rightReadOnly, std::vector<std::string> *letElems, std::vector<std::string> *rightElems);
    virtual ~wxDiff(){}

    wxString IsDifferent()const;

    wxString GetDiff()const;
    std::map<long, int> GetAddedLines()const;
    std::map<long, int> GetRemovedLines()const;
    std::map<long, int> GetLeftEmptyLines()const;
    std::map<long, int> GetRightEmptyLines()const;
    std::map<long, long> GetLinePositionsLeft()const;
    std::map<long, long> GetLinePositionsRight()const;
    wxString GetLeftFilename()const;
    wxString GetRightFilename()const;
    bool RightReadOnly()const;
    bool LeftReadOnly()const;
private:
    wxString CreateHeader()const;
    void ParseDiff(std::vector<wxArrayString> diffs);
    void LoadLines(std::vector<std::string> &Lines, std::vector<std::string> *elems, const wxString &filename);

    wxString leftFilename_;
    wxString rightFilename_;
    bool leftReadOnly_;
    bool rightReadOnly_;
    wxString diff_;
    std::map<long, int>  added_lines_;
    std::map<long, int>  removed_lines_;
    std::map<long, int>  left_empty_lines_;
    std::map<long, int>  right_empty_lines_;
    std::map<long, long> line_pos_right_;
    std::map<long, long> line_pos_left_;
};

#endif
