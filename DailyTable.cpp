#include <wx/combobox.h>

#include "DailyTable.h"

wxString DailyTable::GetValue(int row, int col)
{
    wxString v = DataTable::GetValue(row, col);
    if (!v.IsEmpty()) return v;
    if (row > 0 && row <= m_rows.size() && col == m_categoryColumn) {
        struct item *it = m_rows[row - 1];
        if (setCatNotAllowed(it)) return "";
        const char *name = item_cat_name(it);
        if (name != NULL) return name;
        return _("Unclassified");
    }
    return "";
}

void DailyTable::changeCat(struct item *it, const char *name)
{
    int dup;
    struct cat_node *cat;
    mtree_node *root = &m_cat->getCatRoot()->root;
    if (root != nullptr) {
        if (it->money < 0) {
            cat = get_cat_from_cstr_name(mtree_first_child(root), name);
        } else {
            cat = get_cat_from_cstr_name(mtree_last_child(root), name);
        }
    } else {
        cat = NULL;
    }
    if (it->word != NULL) delete_word(it->word);
    if (cat != NULL && (it->word = add_word(cat, &it->desc, &dup)) == NULL) throw std::bad_alloc();
    m_cat->setModified();
}

void DailyTable::SetValue(int row, int col, const wxString &value)
{
    wxASSERT(m_cat != nullptr);
    struct item *it = m_rows[row - 1];
    if (col == m_categoryColumn) {
        if (string_is_empty(&it->desc)) {
            update_item_cat(it, m_cat->getCatRoot());
        } else {
            changeCat(it, value);
            updateCat();
        }
    } else if (doSetValue(row, col, value) || col == m_descColumn) {
        update_item_cat(it, m_cat->getCatRoot());
        if (GetView() != nullptr) {
            GetView()->AutoSizeColumn(m_categoryColumn);
        }
    }
    if (GetView() != nullptr) {
        GetView()->AutoSizeColumn(col);
    }
}

wxGridCellAttr *DailyTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind)
{
    wxGridCellAttr *attr = DataTable::GetAttr(row, col, kind);
    if (attr != nullptr) return attr;
    if (row > 0 && row <= m_rows.size() && col == m_categoryColumn) {
        struct item *it = m_rows[row - 1];
        if (setCatNotAllowed(it)) {
            m_moneyRoAttr->IncRef();
            return m_moneyRoAttr;
        }
        if (it->money > 0) {
            if (it->word == NULL) {
                m_categoryNoCatAttrO->IncRef();
                return m_categoryNoCatAttrO;
            } else {
                m_categoryAttrO->IncRef();
                return m_categoryAttrO;
            }
        } else {
            if (it->word == NULL) {
                m_categoryNoCatAttrI->IncRef();
                return m_categoryNoCatAttrI;
            } else {
                m_categoryAttrI->IncRef();
                return m_categoryAttrI;
            }
        }
    }
    return nullptr;
}
