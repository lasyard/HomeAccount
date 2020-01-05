#include "StatHtml.h"
#include "file/DataFileRW.h"

#include "c/cat.h"

void StatHtml::showTotal(struct cat_root *cat, int sYear, int sMonth, int eYear, int eMonth)
{
    m_src = "<body>";
    wxString tmp;
    tmp.Printf(_("msgTotalTitle"), sYear, sMonth, eYear, eMonth);
    h2(tmp);
    if (!mtree_is_leaf(&cat->root)) {
        totalTableHtml(mtree_first_child(&cat->root), cat->no_cat_in_sum);
        m_src += "<p><br /></p>";
        totalTableHtml(mtree_last_child(&cat->root), cat->no_cat_out_sum);
    }
    m_src += "</body>";
    SetPage(m_src);
}

void StatHtml::showIO(DataFileRW *data, const wxString &title)
{
    m_src = "<body>";
    h2(title);
    m_src += "<table border='1' cellspacing='0' width='100%'>";
    m_src += "<tr>";
    m_src += "<th>" + _("strTime") + "</th>";
    m_src += "<th>" + _("strIncome") + "</th>";
    m_src += "<th>" + _("strOutlay") + "</th>";
    m_src += "<th>" + _("strNetIncome") + "</th>";
    m_src += "</tr>";
    long totalI = 0, totalO = 0;
    for (DataFileRW::pageIterator i = data->pageBegin(); i != data->pageEnd(); ++i) {
        long income, outlay;
        cal_page_income_outlay(&(*i), &income, &outlay);
        showIOLine(i->title.str, income, outlay);
        totalI += income;
        totalO += outlay;
    }
    showIOLine(_("strTotal"), totalI, totalO);
    m_src += "</table>";
    m_src += "</body>";
    SetPage(m_src);
}

void StatHtml::firstLevelRow(const wxString &label, long money, const wxString &centLabel, const wxString &cent)
{
    m_src += "<tr><td width='25%'>";
    m_src += label;
    m_src += "</td><td>";
    m_src += "<table border='0' width='100%'>";
    secondLevelRow("", money, centLabel, cent);
    m_src += "</table>";
    m_src += "</td></tr>";
}

void StatHtml::secondLevelRow(
    const wxString &label, long money, const wxString &centLabel, const wxString &cent, bool isBu)
{
    char buf[MONEY_LEN];
    m_src += "<tr><td width='25%'>";
    bu(label, isBu);
    m_src += "</td><td width='25%' align='right'>";
    money_to_str(buf, money);
    bu(buf, isBu);
    m_src += "</td><td width='25%' align='right'>";
    bu(centLabel, isBu);
    m_src += "</td><td align='right'>";
    bu(cent, isBu);
    m_src += "</td></tr>";
}

void StatHtml::totalTableHtml(struct mtree_node *root, long no_cat_sum)
{
    struct cat_node *cat = get_cat_node(root);
    m_src += "<table border='1' cellspacing='0' width='100%'>";
    m_src += "<tr><th colspan='2' bgcolor='black'><font color='white'>";
    m_src += cat->name.str;
    m_src += "</font></th></tr>";
    firstLevelRow(_("strTotal"), cat->total);
    firstLevelRow(_("strNoCat"), no_cat_sum, _("strTotalCent"), calCent(no_cat_sum, cat->total));
    firstLevelRow(_("strOther"), cat->sub_total, _("strTotalCent"), calCent(cat->sub_total, cat->total));
    firstCatStatHtml(root);
    m_src += "</table>";
}

void StatHtml::firstCatStatHtml(struct mtree_node *root)
{
    struct ulist_item *item;
    struct cat_node *root_cat = get_cat_node(root);
    for (item = root->children.first; item != NULL; item = item->next) {
        struct mtree_node *node = get_mtree_node(item);
        struct cat_node *cat = get_cat_node(node);
        m_src += "<tr><td>";
        m_src += cat->name.str;
        m_src += "</td><td>";
        m_src += "<table border='0' width='100%'>";
        secondLevelRow(_("strSubTotal"), cat->total, _("strTotalCent"), calCent(cat->total, root_cat->total), true);
        secondLevelRow(_("strOther"), cat->sub_total, _("strSubCent"), calCent(cat->sub_total, cat->total));
        secondCatStatHtml(node);
        m_src += "</table>";
        m_src += "</td></tr>";
    }
}

void StatHtml::secondCatStatHtml(struct mtree_node *root)
{
    struct ulist_item *item;
    struct cat_node *root_cat = get_cat_node(root);
    for (item = root->children.first; item != NULL; item = item->next) {
        struct mtree_node *node = get_mtree_node(item);
        struct cat_node *cat = get_cat_node(node);
        secondLevelRow(cat->name.str, cat->total, _("strSubCent"), calCent(cat->total, root_cat->total));
    }
}

void StatHtml::showIOLine(const wxString &label, long income, long outlay)
{
    char buf[MONEY_LEN];
    m_src += "<tr>";
    m_src += "<td>";
    m_src += label;
    m_src += "</td>";
    m_src += "<td>";
    money_to_str(buf, income);
    m_src += buf;
    m_src += "</td>";
    m_src += "<td>";
    money_to_str(buf, outlay);
    m_src += buf;
    m_src += "</td>";
    m_src += "<td>";
    long n = income - outlay;
    if (n < 0) {
        m_src += "<font color='red'>";
    } else {
        m_src += "<font color='green'>";
    }
    money_to_str(buf, n);
    m_src += buf;
    m_src += "</font>";
    m_src += "</td>";
    m_src += "</tr>";
}
