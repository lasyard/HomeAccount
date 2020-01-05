#include <wx/msgdlg.h>

#include "DataGrid.h"
#include "HaDefs.h"

DataGrid::DataGrid(wxWindow *parent, wxWindowID id) : wxGrid(parent, id), m_data(NULL)
{
    SetColMinimalAcceptableWidth(70);
    DisableDragRowSize();
    CreateGrid(0, ColumnNum);
    SetColLabelValue(IncomeIndex, _("strIncome"));
    SetColLabelValue(OutlayIndex, _("strOutlay"));
    SetColLabelValue(BalanceIndex, _("strBalance"));
    SetColLabelValue(DescIndex, _("strDesc"));
    SetColLabelValue(CommentIndex, _("strComment"));
    SetColFormatFloat(IncomeIndex, MONEY_LEN, 2);
    SetColFormatFloat(OutlayIndex, MONEY_LEN, 2);
    SetColFormatFloat(BalanceIndex, MONEY_LEN, 2);
    AppendRows(2);
    for (int i = 0; i < ColumnNum; i++) SetReadOnly(0, i);
}

DEFINE_EVENT_TYPE(wxEVT_RESIZE_COL)

BEGIN_EVENT_TABLE(DataGrid, wxGrid)
EVT_GRID_CELL_CHANGED(DataGrid::onCellChange)
EVT_KEY_DOWN(DataGrid::onKeyDown)
EVT_COMMAND(ID_GRID, wxEVT_RESIZE_COL, DataGrid::onResizeCol)
END_EVENT_TABLE()

void DataGrid::onCellChange(wxGridEvent &event)
{
    int row = event.GetRow();
    int col = event.GetCol();
    processItemChange(dataPosFromRow(row), row, col);
}

void DataGrid::processItemChange(struct item *it, int row, int col)
{
    if (m_data == NULL || row == 0) return;
    wxString text;
    long money = 0, moneyI = 0, moneyO = 0;
    if (!(text = GetCellValue(row, IncomeIndex)).IsEmpty()) {
        moneyI = -str_to_money(text);
    }
    if (!(text = GetCellValue(row, OutlayIndex)).IsEmpty()) {
        moneyO = str_to_money(text);
    }
    if (moneyI != 0 && moneyO != 0) {
        wxMessageBox(_("errDupMoney"), _("appName"), wxOK | wxICON_ERROR);
        if (col == IncomeIndex) {
            money = moneyI;
            SetCellValue(row, OutlayIndex, "");
        } else if (col == OutlayIndex) {
            money = moneyO;
            SetCellValue(row, IncomeIndex, "");
        }
    } else if (moneyI != 0) {
        money = moneyI;
        SetCellValue(row, OutlayIndex, "");
    } else {
        money = moneyO;
        SetCellValue(row, IncomeIndex, "");
    }
    wxString desc = GetCellValue(row, DescIndex);
    desc.Trim(true).Trim(false);
    wxString comment = GetCellValue(row, CommentIndex);
    comment.Trim(true).Trim(false);
    struct string s_desc, s_comment;
    string_mock_slice(&s_desc, desc, '\0');
    string_mock_slice(&s_comment, comment, '\0');
    if (item_set(it, money, &s_desc, &s_comment) == NULL) throw std::bad_alloc();
    m_data->setModified();
    if (col == IncomeIndex || col == OutlayIndex) {
        updateBalanceAndTotal();
    }
    if (is_dummy_item(it)) clearRow(row);
    wxCommandEvent ev(wxEVT_RESIZE_COL, GetId());
    ev.SetEventObject(this);
    ev.SetInt(col);
    wxPostEvent(this, ev);
}

void DataGrid::onKeyDown(wxKeyEvent &event)
{
    bool modified = false;
    if (event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER) {
        if (event.ControlDown()) {
            if (!IsCellEditControlEnabled()) {
                insertRowToDataUp(GetGridCursorRow());
                modified = true;
            }
        } else {
            insertRowToData(GetGridCursorRow());
            modified = true;
        }
    } else if (event.GetKeyCode() == WXK_DELETE) {
        if (IsSelection()) {
            wxArrayInt rows = GetSelectedRows();
            if (rows.Count() > 0) {
                modified = deleteRowsFromData(rows);
            } else {
                unsigned int i;
                int row, col;
                wxGridCellCoordsArray cells = GetSelectedCells();
                for (i = 0; i < cells.Count(); i++) {
                    clearCellData(cells[i].GetRow(), cells[i].GetCol());
                }
                wxGridCellCoordsArray topLeft = GetSelectionBlockTopLeft();
                wxGridCellCoordsArray bottomRight = GetSelectionBlockBottomRight();
                for (i = 0; i < topLeft.Count(); i++) {
                    for (row = topLeft[i].GetRow(); row <= bottomRight[i].GetRow(); row++) {
                        for (col = topLeft[i].GetCol(); col <= bottomRight[i].GetCol(); col++) {
                            clearCellData(row, col);
                        }
                    }
                }
                wxArrayInt cols = GetSelectedCols();
                for (i = 0; i < cols.Count(); i++) {
                    for (row = 1; row < GetNumberRows(); row++) {
                        clearCellData(row, cols[i]);
                    }
                }
            }
            ClearSelection();
        } else {
            clearCellData(GetGridCursorRow(), GetGridCursorCol());
        }
    }
    wxGrid::OnKeyDown(event);
    if (modified) updateData(true);
}

void DataGrid::scrollToDay(int day)
{
    int i;
    for (i = 0; i < GetNumberRows(); i++) {
        wxString title = GetRowLabelValue(i);
        if (title.length() == DATE_LEN) {
            int year, month, d;
            str_to_ymd(title, &year, &month, &d, '.');
            if (d == day) {
                MakeCellVisible(i, 1);
                break;
            }
        }
    }
}

void DataGrid::updateData(bool setModified)
{
    if (setModified) m_data->setModified();
    showMainData();
    updateBalanceAndTotal();
    SetGridCursor(GetGridCursorRow(), OutlayIndex);
}

void DataGrid::setRow(int row)
{
    SetCellAlignment(row, IncomeIndex, wxALIGN_RIGHT, wxALIGN_CENTER);
    SetCellAlignment(row, OutlayIndex, wxALIGN_RIGHT, wxALIGN_CENTER);
    SetCellAlignment(row, BalanceIndex, wxALIGN_RIGHT, wxALIGN_CENTER);
    SetReadOnly(row, IncomeIndex, false);
    SetReadOnly(row, OutlayIndex, false);
    SetReadOnly(row, DescIndex, false);
    SetReadOnly(row, CommentIndex, false);
    SetReadOnly(row, BalanceIndex);
    SetCellEditor(row, IncomeIndex, new wxGridCellFloatEditor(MONEY_LEN, 2));
    SetCellEditor(row, OutlayIndex, new wxGridCellFloatEditor(MONEY_LEN, 2));
    clearRow(row);
    for (int i = 0; i < ColumnNum; i++) {
        SetCellFont(row, i, GetDefaultCellFont());
        SetCellTextColour(row, i, GetDefaultCellTextColour());
    }
}

void DataGrid::showMainData()
{
    char buf[MONEY_LEN];
    BeginBatch();
    int diff = GetNumberRows() - (m_data->getItemsNum() + 1 + 1);  // +initial and total
    if (diff < 0) {
        if (!AppendRows(-diff)) return;
        setRows(m_data->getItemsNum() + 1 + diff, -diff);
    } else if (diff > 0) {
        if (!DeleteRows(m_data->getItemsNum() + 1, diff)) return;
    }
    SetRowLabelValue(0, _("strInitial"));
    money_to_str(buf, m_data->getInitial());
    SetCellValue(0, BalanceIndex, buf);
    int row = 1;
    for (DataFileRW::ItemIterator i = m_data->ItemBegin(); i != m_data->ItemEnd(); ++i, row++) {
        if (ulist_is_first(&i->owner->items, &i->ulist)) {
            SetRowLabelValue(row, i->owner->title.str);
        } else {
            SetRowLabelValue(row, "");
        }
        clearRow(row);
        if (is_dummy_item(&*i)) continue;
        if (i->money < 0) {
            money_to_str(buf, -i->money);
            SetCellValue(row, IncomeIndex, buf);
        } else {
            money_to_str(buf, i->money);
            SetCellValue(row, OutlayIndex, buf);
        }
        SetCellValue(row, DescIndex, i->desc.str);
        SetCellValue(row, CommentIndex, i->comment.str);
    }
    SetRowLabelValue(row, _("strTotal"));
    for (int i = 0; i < ColumnNum; i++) SetReadOnly(row, i);
    wxFont font = GetCellFont(row, 0);
    font.MakeBold();
    SetCellFont(row, IncomeIndex, font);
    SetCellFont(row, OutlayIndex, font);
    EndBatch();
}

void DataGrid::updateBalanceAndTotal()
{
    char buf[MONEY_LEN];
    BeginBatch();
    long balance = m_data->getInitial();
    long income = 0;
    long outlay = 0;
    int row = 1;
    for (DataFileRW::ItemIterator i = m_data->ItemBegin(); i != m_data->ItemEnd(); ++i, row++) {
        if (is_dummy_item(&*i)) continue;
        balance = cal_item_balance(&*i, balance);
        money_to_str(buf, balance);
        SetCellValue(row, BalanceIndex, buf);
        if (i->money < 0) {
            income -= i->money;
        } else {
            outlay += i->money;
        }
    }
    money_to_str(buf, income);
    SetCellValue(row, IncomeIndex, buf);
    money_to_str(buf, outlay);
    SetCellValue(row, OutlayIndex, buf);
    EndBatch();
}

void DataGrid::showAllData()
{
    updateData();
    SetRowLabelSize(wxGRID_AUTOSIZE);
    AutoSizeRows(false);
    AutoSizeColumns(false);
}

void DataGrid::insertRowToData(int row)
{
    if (row == 0) return;
    if (insert_dummy_item(dataPosFromRow(row)) == NULL) throw std::bad_alloc();
}

void DataGrid::insertRowToDataUp(int row)
{
    if (row == 0) return;
    struct item *it = dataPosFromRow(row);
    if (ulist_is_first(&it->owner->items, &it->ulist)) {
        if (insert_dummy_item_head(it->owner) == NULL) throw std::bad_alloc();
    } else {
        insertRowToData(row - 1);
    }
}

bool DataGrid::deleteRowsFromData(wxArrayInt &rows)
{
    bool modified = false;
    unsigned int i;
    for (i = 0; i < rows.Count(); i++) {
        if (rows[i] == 0) continue;
        set_item_delete(dataPosFromRow(rows[i]));
        modified = true;
    }
    delete_items_from_data(m_data->getData());
    return modified;
}

bool DataGrid::clearCellData(int row, int col)
{
    if (!IsReadOnly(row, col) && !GetCellValue(row, col).IsEmpty()) {
        clearCell(row, col);
        wxGridEvent ev(GetId(), wxEVT_GRID_CELL_CHANGED, this, row, col);
        wxPostEvent(this, ev);
    }
    return false;
}
