#ifndef _STAT_HTML_H_
#define _STAT_HTML_H_

#include <wx/html/htmlwin.h>

class DataFileRW;

class StatHtml : public wxHtmlWindow
{
public:
    StatHtml(wxWindow *parent, wxWindowID id) : wxHtmlWindow(parent, id)
    {
    }

    virtual ~StatHtml()
    {
    }

    void showTotal(struct cat_root *cat, int sYear, int sMonth, int eYear, int eMonth);
    void showIO(DataFileRW *data, const wxString &title);

private:
    wxString m_src;

    void firstLevelRow(const wxString &label, long money, const wxString &centLabel = "", const wxString &cent = "");
    void secondLevelRow(const wxString &label,
                        long money,
                        const wxString &centLabel = "",
                        const wxString &cent = "",
                        bool isBu = false);
    void totalTableHtml(struct mtree_node *root, long no_cat_sum);
    void firstCatStatHtml(struct mtree_node *root);
    void secondCatStatHtml(struct mtree_node *root);

    wxString calCent(long sub, long total)
    {
        wxString tmp;
        tmp.Printf("%.2f%%", (total <= 0) ? 0.0 : (float)sub / (float)total * 100.0);
        return tmp;
    }

    void showIOLine(const wxString &label, long income, long outlay);

    void h2(const wxString &text)
    {
        m_src += "<h2>";
        m_src += text;
        m_src += "</h2>";
    }

    void bu(const wxString &text, bool isBu = true)
    {
        if (isBu) m_src += "<b><u>";
        m_src += text;
        if (isBu) m_src += "</u></b>";
    }
};

#endif
