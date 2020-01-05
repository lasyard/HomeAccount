#include <wx/msgdlg.h>

#include "DailyGrid.h"
#include "HaDefs.h"

#include "c/cal.h"

DailyGrid::DailyGrid(wxWindow *parent, wxWindowID id)
    : DataGrid(parent, id), m_cat(NULL), m_catChoiceI(new wxArrayString), m_catChoiceO(new wxArrayString)
{
    AppendCols();
    SetColLabelValue(ClassIndex, _("strClass"));
    SetColMinimalWidth(ClassIndex, 120);
    SetColSize(ClassIndex, 120);
    SetReadOnly(0, ClassIndex);
}

DEFINE_EVENT_TYPE(wxEVT_RESIZE_COL)

BEGIN_EVENT_TABLE(DailyGrid, DataGrid)
EVT_GRID_CELL_CHANGED(DailyGrid::onCellChange)
END_EVENT_TABLE()

void DailyGrid::onCellChange(wxGridEvent &event)
{
    int row = event.GetRow();
    int col = event.GetCol();
    struct item *it = dataPosFromRow(row);
    bool needResizeCat = false;
    if (col != ClassIndex) {
        processItemChange(it, row, col);
        if (col == IncomeIndex || col == OutlayIndex || col == DescIndex) {
            updateCat(row, it);
            needResizeCat = true;
        }
    } else {
        if (string_is_empty(&it->desc)) {
            wxMessageBox(_("errNullDesc"), _("appName"), wxOK | wxICON_ERROR);
            updateCat(row, it);
        } else {
            changeCat(row, it->money);
            m_cat->setModified();
            updateCat();
        }
    }
    if (needResizeCat) {
        wxCommandEvent ev(wxEVT_RESIZE_COL, GetId());
        ev.SetEventObject(this);
        ev.SetInt(ClassIndex);
        wxPostEvent(this, ev);
    }
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

void DailyGrid::setCatChoice(wxArrayString *arr, struct mtree_node *root)
{
    arr->Add(_("strNoCat"));
    if (root == NULL) return;
    struct mtree_node *node;
    for (node = mtree_pf_first(root); node != NULL; node = mtree_pf_next(root, node)) {
        arr->Add(get_cat_node(node)->name.str);
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
        SetCellEditor(row, ClassIndex, new wxGridCellChoiceEditor(*m_catChoiceI));
    } else if (it->money > 0) {
        SetCellEditor(row, ClassIndex, new wxGridCellChoiceEditor(*m_catChoiceO));
    } else {
        clearCell(row, ClassIndex);
        SetReadOnly(row, ClassIndex);
        return;
    }
    update_item_cat(it, m_cat->getCatRoot());
    if (it->cat != NULL) {
        SetCellValue(row, ClassIndex, it->cat->name.str);
        SetCellTextColour(row, ClassIndex, *wxBLACK);
    } else {
        SetCellValue(row, ClassIndex, _("strNoCat"));
        SetCellTextColour(row, ClassIndex, *wxRED);
    }
    SetReadOnly(row, ClassIndex, false);
}

void DailyGrid::showAllData()
{
    showMainData();
    updateBalanceAndTotal();
    updateCat();
    SetRowLabelSize(wxGRID_AUTOSIZE);
    AutoSizeRows(false);
    AutoSizeColumns(false);
}

void DailyGrid::changeCat(int row, long money)
{
    int dup;
    if (m_data == NULL) return;
    struct item *it = dataPosFromRow(row);
    struct cat_node *cat = getCatNode(money, GetCellValue(row, ClassIndex));
    if (it->cat != NULL) delete_word(it->cat, &it->desc);
    if (cat != NULL && add_word(cat, &it->desc, &dup) == NULL) throw std::bad_alloc();
}
