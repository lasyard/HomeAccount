#ifndef _STAT_HTML_H_
#define _STAT_HTML_H_

#include <fstream>
#include <string>

#include <wx/html/htmlwin.h>

#include "c/core_defs.h"

class DataFileRW;

class StatHtml : public wxHtmlWindow
{
    wxDECLARE_DYNAMIC_CLASS(StatHtml);

public:
    StatHtml() : m_src(), m_fileName()
    {
    }

    virtual ~StatHtml()
    {
    }

    void showTotal(struct cat_root *cat, int sYear, int sMonth, int eYear, int eMonth);
    void showIO(DataFileRW *data, const wxString &title);

    void saveAs(const std::string &path)
    {
        std::ofstream file(path);
        file << m_src.mbc_str();
    }

    const wxString &fileName() const
    {
        return m_fileName;
    }

    void setFileName(const wxString &fileName)
    {
        m_fileName = fileName;
    }

private:
    wxString m_src;
    wxString m_fileName;

    void firstLevelRow(const wxString &label, money_t money, const wxString &centLabel = "", const wxString &cent = "");
    void secondLevelRow(const wxString &label,
                        money_t money,
                        const wxString &centLabel = "",
                        const wxString &cent = "",
                        bool isBu = false);
    void totalTableHtml(struct mtree_node *root, money_t no_cat_sum);
    void firstCatStatHtml(struct mtree_node *root);
    void secondCatStatHtml(struct mtree_node *root);

    void htmlHeader(const wxString &title);
    void htmlFooter();

    wxString calCent(money_t sub, money_t total)
    {
        wxString tmp;
        tmp.Printf("%.2f%%", (total <= 0) ? 0.0 : (float)sub / (float)total * 100.0);
        return tmp;
    }

    void showIOLine(const wxString &label, money_t income, money_t outlay);

    void h2(const wxString &text)
    {
        m_src += "<h2>" + text + "</h2>\n";
    }

    void bu(const wxString &text, bool isBu = true)
    {
        if (isBu) m_src += "<b><u>";
        m_src += text;
        if (isBu) m_src += "</u></b>";
    }
};

#endif
