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

wxDiff::wxDiff(wxString filename1, wxString filename2):
    m_filename1(filename1),
    m_filename2(filename2)
{
    typedef string elem;
    typedef pair<elem, elemInfo> sesElem;
    ifstream Aifs(filename1.mbc_str());
    ifstream Bifs(filename2.mbc_str());
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

    m_diff = wxString(help.str().c_str(), wxConvUTF8);

    vector<wxArrayString> diffs;
    wxStringTokenizer tkz(m_diff, wxT("\n"));
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
    wxFileName filename(m_filename1);
    wxDateTime modifyTime;
    wxDateTime modifyTime2;
    filename.GetTimes(0, &modifyTime, 0);
    filename.Assign(m_filename2);
    filename.GetTimes(0, &modifyTime2, 0);
    if(modifyTime == modifyTime2 && m_filename1 == m_filename2)
        return _("Same file => Same content!");
    if(m_added_lines.empty() && m_removed_lines.empty())
        return _("Different files, but same content!");
    return wxEmptyString;
}

wxString wxDiff::CreateHeader()
{
    wxString header;
    wxFileName filename(m_filename1);
    wxDateTime modifyTime;
    filename.GetTimes(0, &modifyTime, 0);
    header << _T("--- ") << m_filename1
           << _T("\t") << modifyTime.Format(_T("%Y-%m-%d %H:%M:%S %z"))
           << _T("\n");
    filename.Assign(m_filename2);
    filename.GetTimes(0, &modifyTime, 0);
    header << _T("+++ ") << m_filename2
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
                    m_added_lines[block_start_right] = added;
                }
                if (removed > 0)
                {
                    m_removed_lines[block_start_left] = removed;
                    m_line_pos[block_start_left] = block_start_right;
                }

                if(added > removed)
                    m_left_empty_lines[block_start_left+removed] = added - removed;
                if(removed > added)
                    m_right_empty_lines[block_start_right+added] = removed - added;
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
    return CreateHeader() + m_diff;
}

std::map<long, int> wxDiff::GetAddedLines()
{
    return m_added_lines;
}

std::map<long, int> wxDiff::GetLeftEmptyLines()
{
    return m_left_empty_lines;
}

std::map<long, int> wxDiff::GetRightEmptyLines()
{
    return m_right_empty_lines;
}

std::map<long, long> wxDiff::GetLinePositions()
{
    return m_line_pos;
}

std::map<long, int> wxDiff::GetRemovedLines()
{
    return m_removed_lines;
}

wxString wxDiff::GetFromFilename()
{
    return m_filename1;
}

wxString wxDiff::GetToFilename()
{
    return m_filename2;
}
