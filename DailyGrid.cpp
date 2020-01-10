#include <wx/msgdlg.h>

#include "DailyGrid.h"

#include "c/cal.h"

wxIMPLEMENT_DYNAMIC_CLASS(DailyGrid, DataGrid);

BEGIN_EVENT_TABLE(DailyGrid, DataGrid)
EVT_GRID_CELL_CHANGED(DailyGrid::onCellChange)
END_EVENT_TABLE()

void DailyGrid::onCellChange(wxGridEvent &event)
{
    int row = event.GetRow();
    int col = event.GetCol();
    struct item *it = dataPosFromRow(row);
    bool needResizeCat = false;
    if (col != CLASS_COLUMN) {
        processItemChange(it, row, col);
        if (col == INCOME_COLUMN || col == OUTLAY_COLUMN || col == DESC_COLUMN) {
            updateCat(row, it);
            needResizeCat = true;
        }
    } else {
        if (string_is_empty(&it->desc)) {
            wxMessageBox(_("Cannot set category for an item with no description"), _("App name"), wxOK | wxICON_ERROR);
            updateCat(row, it);
        } else {
            changeCat(row, it->money);
            m_cat->setModified();
            updateCat();
        }
    }
    if (needResizeCat) {
        AutoSizeColumn(CLASS_COLUMN);
    }
}

void DailyGrid::initGrid()
{
    DataGrid::initGrid();
    AppendCols(1);
    SetColLabelValue(CLASS_COLUMN, _("Category"));
    SetColMinimalWidth(CLASS_COLUMN, 120);
    SetColSize(CLASS_COLUMN, 120);
    SetReadOnly(0, CLASS_COLUMN);
}

void DailyGrid::updateCat()
{
    BeginBatch();
    int row = 1;
    for (DataFileRW::ItemIterator i = m_data->ItemBegin(); i != m_data->ItemEnd(); ++i, row++) {
        updateCat(row, &*i);
    }
    EndBatch();
}

void DailyGrid::setCatChoice(wxArrayString &arr, struct mtree_node *root)
{
    arr.clear();
    arr.Add(_("Unclassified"));
    if (root == NULL) return;
    struct mtree_node *node;
    for (node = mtree_pf_first(root); node != NULL; node = mtree_pf_next(root, node)) {
        arr.Add(get_cat_node(node)->name.str);
    }
}

struct cat_node *DailyGrid::getCatNode(struct mtree_node *root, const char *name) const
{
    struct string s;
    string_mock_slice(&s, name, '\0');
    struct cat_node *cat = get_cat_from_name(root, &s);
    return cat;
}

struct cat_node *DailyGrid::getCatNode(long money, wxString str) const
{
    if (str.IsEmpty()) return NULL;
    if (money < 0) {
        return getCatNode(mtree_first_child(&m_cat->getCatRoot()->root), str);
    } else {
        return getCatNode(mtree_last_child(&m_cat->getCatRoot()->root), str);
    }
}

void DailyGrid::updateCat(int row, struct item *it)
{
    if (it->money < 0) {
        SetCellEditor(row, CLASS_COLUMN, new wxGridCellChoiceEditor(m_catChoiceI));
    } else if (it->money > 0) {
        SetCellEditor(row, CLASS_COLUMN, new wxGridCellChoiceEditor(m_catChoiceO));
    } else {
        clearCell(row, CLASS_COLUMN);
        SetReadOnly(row, CLASS_COLUMN);
        return;
    }
    update_item_cat(it, m_cat->getCatRoot());
    if (it->cat != NULL) {
        SetCellValue(row, CLASS_COLUMN, it->cat->name.str);
        SetCellTextColour(row, CLASS_COLUMN, *wxBLACK);
    } else {
        SetCellValue(row, CLASS_COLUMN, _("Unclassified"));
        SetCellTextColour(row, CLASS_COLUMN, *wxRED);
    }
    SetReadOnly(row, CLASS_COLUMN, false);
}

void DailyGrid::changeCat(int row, long money)
{
    int dup;
    if (m_data == NULL) return;
    struct item *it = dataPosFromRow(row);
    struct cat_node *cat = getCatNode(money, GetCellValue(row, CLASS_COLUMN));
    if (it->cat != NULL) delete_word(it->cat, &it->desc);
    if (cat != NULL && add_word(cat, &it->desc, &dup) == NULL) throw std::bad_alloc();
}
