#include "wxDiff.h"

#include "dtl/dtl.hpp"

#include <sstream>
#include <fstream>
#include <wx/textfile.h>
#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/tokenzr.h>

using dtl::Diff;
using dtl::elemInfo;
using dtl::uniHunk;
using std::pair;
using std::string;
using std::ifstream;
using std::vector;

wxDiff::wxDiff(wxString leftFilename, wxString rightFilename, bool leftReadOnly, bool rightReadOnly):
    leftFilename_(leftFilename),
    rightFilename_(rightFilename),
    leftReadOnly_(leftReadOnly),
    rightReadOnly_(rightReadOnly)
{
    typedef string elem;
    typedef pair<elem, elemInfo> sesElem;
    ifstream Aifs(leftFilename.mbc_str());
    ifstream Bifs(rightFilename.mbc_str());
    elem buf;
    vector<elem> ALines, BLines;

    while(getline(Aifs, buf))
    {
        ALines.push_back(buf);
    }
    while(getline(Bifs, buf))
    {
        BLines.push_back(buf);
    }

    Diff<elem, vector<elem> > diff(ALines, BLines);
    diff.onHuge();
    diff.compose();

    uniHunk< sesElem > hunk;

    diff.composeUnifiedHunks();
    std::ostringstream help;
    diff.printUnifiedFormat();
    diff.printUnifiedFormat(help);

    diff_ = wxString(help.str().c_str(), wxConvUTF8);

    vector<wxArrayString> diffs;
    wxStringTokenizer tkz(diff_, wxT("\n"));
    wxArrayString currdiff;
    while(tkz.HasMoreTokens())
    {
        wxString token = tkz.GetNextToken();
        if(token.StartsWith(_T("@@")) &&
           token.EndsWith(_T("@@")) &&
           !currdiff.IsEmpty())
        {
            diffs.push_back(currdiff);
            currdiff.clear();
        }
        currdiff.Add(token);
    }
    if(!currdiff.IsEmpty())
        diffs.push_back(currdiff);

    ParseDiff(diffs);
}

wxString wxDiff::IsDifferent()
{
    wxFileName filename(leftFilename_);
    wxDateTime modifyTime;
    wxDateTime modifyTime2;
    filename.GetTimes(0, &modifyTime, 0);
    filename.Assign(rightFilename_);
    filename.GetTimes(0, &modifyTime2, 0);
    if(modifyTime == modifyTime2 && leftFilename_ == rightFilename_)
        return _("Same file => Same content!");
    if(added_lines_.empty() && removed_lines_.empty())
        return _("Different files, but same content!");
    return wxEmptyString;
}

wxString wxDiff::CreateHeader()
{
    wxString header;
    wxFileName filename(leftFilename_);
    wxDateTime modifyTime;
    filename.GetTimes(0, &modifyTime, 0);
    header << _T("--- ") << leftFilename_
           << _T("\t") << modifyTime.Format(_T("%Y-%m-%d %H:%M:%S %z"))
           << _T("\n");
    filename.Assign(rightFilename_);
    filename.GetTimes(0, &modifyTime, 0);
    header << _T("+++ ") << rightFilename_
           << _T("\t") << modifyTime.Format(_T("%Y-%m-%d %H:%M:%S %z"))
           << _T("\n");
    return header;
}

void wxDiff::ParseDiff(vector<wxArrayString> diffs)
{
    for(unsigned int i = 0; i < diffs.size(); i++)
    {
        wxArrayString currdiff = diffs[i];
        wxString headline = currdiff[0];
        headline.Replace(_T("@@"),_T(""));
        headline.Trim();
        long start_left = -1;
        long start_right = -1;
        headline.Mid(headline.Find(_T("-")) + 1,
                     headline.Find(_T(","))).ToLong(&start_left);
        headline.Mid(headline.Find(_T("+")) + 1,
                     headline.Find(_T(","))).ToLong(&start_right);
        start_left -= 1; //using 0-based line numbers internally
        start_right -= 1;
        unsigned int added = 0;
        unsigned int removed = 0;
        unsigned int block_start_left = 0;
        unsigned int block_start_right = 0;
        for(unsigned int i = 1; i < currdiff.size(); ++i)
        {
            wxString line = currdiff[i];
            bool add = line.StartsWith(_T("+"));
            bool rem = line.StartsWith(_T("-"));
            if(add || rem)
            {
                if(added == 0 && removed == 0)
                {
                    block_start_left = start_left;
                    block_start_right = start_right;
                }
                if(add)
                {
                    ++added;
                    ++start_right;
                }
                if(rem)
                {
                    ++removed;
                    ++start_left;
                }
            }
            else
            {
                if (added > 0)
                {
                    added_lines_[block_start_right] = added;
                }
                if (removed > 0)
                {
                    removed_lines_[block_start_left] = removed;
                    line_pos_[block_start_left] = block_start_right;
                }

                if(added > removed)
                    left_empty_lines_[block_start_left] = added;
                if(removed > added)
                    right_empty_lines_[block_start_right] = removed;
                added = 0;
                removed = 0;
                ++start_left;
                ++start_right;
            }
        }
    }
}

wxString wxDiff::GetDiff()
{
    return CreateHeader() + diff_;
}

std::map<long, int> wxDiff::GetAddedLines()
{
    return added_lines_;
}

std::map<long, int> wxDiff::GetLeftEmptyLines()
{
    return left_empty_lines_;
}

std::map<long, int> wxDiff::GetRightEmptyLines()
{
    return right_empty_lines_;
}

std::map<long, long> wxDiff::GetLinePositions()
{
    return line_pos_;
}

std::map<long, int> wxDiff::GetRemovedLines()
{
    return removed_lines_;
}

wxString wxDiff::GetLeftFilename()
{
    return leftFilename_;
}

wxString wxDiff::GetRightFilename()
{
    return rightFilename_;
}
bool wxDiff::RightReadOnly()const
{
    return rightReadOnly_;
}

bool wxDiff::LeftReadOnly()const
{
    return leftReadOnly_;
}

