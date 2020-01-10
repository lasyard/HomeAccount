#include <wx/msgdlg.h>

#include "DataGrid.h"

const wxFont DataGrid::MONO_FONT(14, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

wxIMPLEMENT_DYNAMIC_CLASS(DataGrid, wxGrid);

BEGIN_EVENT_TABLE(DataGrid, wxGrid)
EVT_GRID_CELL_CHANGED(DataGrid::onCellChange)
EVT_KEY_DOWN(DataGrid::onKeyDown)
END_EVENT_TABLE()

DataGrid::DataGrid() : m_data(nullptr)
{
}

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
    if (!(text = GetCellValue(row, INCOME_COLUMN)).IsEmpty()) {
        moneyI = -str_to_money(text);
    }
    if (!(text = GetCellValue(row, OUTLAY_COLUMN)).IsEmpty()) {
        moneyO = str_to_money(text);
    }
    if (moneyI != 0 && moneyO != 0) {
        wxMessageBox(_("Cannot set both income and outlay in one line"), _("App name"), wxOK | wxICON_ERROR);
        if (col == INCOME_COLUMN) {
            money = moneyI;
            SetCellValue(row, OUTLAY_COLUMN, "");
        } else if (col == OUTLAY_COLUMN) {
            money = moneyO;
            SetCellValue(row, INCOME_COLUMN, "");
        }
    } else if (moneyI != 0) {
        money = moneyI;
        SetCellValue(row, OUTLAY_COLUMN, "");
    } else {
        money = moneyO;
        SetCellValue(row, INCOME_COLUMN, "");
    }
    wxString desc = GetCellValue(row, DESC_COLUMN);
    desc.Trim(true).Trim(false);
    wxString comment = GetCellValue(row, COMMENTS_COLUMN);
    comment.Trim(true).Trim(false);
    struct string s_desc, s_comment;
    string_mock_slice(&s_desc, desc, '\0');
    string_mock_slice(&s_comment, comment, '\0');
    if (item_set(it, money, &s_desc, &s_comment) == NULL) throw std::bad_alloc();
    m_data->setModified();
    if (col == INCOME_COLUMN || col == OUTLAY_COLUMN) {
        updateBalanceAndTotal();
    }
    if (is_dummy_item(it)) clearRow(row);
    AutoSizeColumn(col);
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
    if (modified) {
        m_data->setModified();
        updateData();
    }
    wxGrid::OnKeyDown(event);
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

void DataGrid::initGrid()
{
    SetColMinimalAcceptableWidth(70);
    DisableDragRowSize();
    CreateGrid(0, COLUMN_NUM);
    SetColLabelValue(INCOME_COLUMN, _("Income"));
    SetColLabelValue(OUTLAY_COLUMN, _("Outlay"));
    SetColLabelValue(BALANCE_COLUMN, _("Balance"));
    SetColLabelValue(DESC_COLUMN, _("Description"));
    SetColLabelValue(COMMENTS_COLUMN, _("Comments"));
    SetColFormatFloat(INCOME_COLUMN, MONEY_LEN, 2);
    SetColFormatFloat(OUTLAY_COLUMN, MONEY_LEN, 2);
    SetColFormatFloat(BALANCE_COLUMN, MONEY_LEN, 2);
    AppendRows(2);
    for (int i = 0; i < COLUMN_NUM; i++) SetReadOnly(0, i);
    SetCellFont(0, BALANCE_COLUMN, MONO_FONT);
}

void DataGrid::updateData()
{
    showMainData();
    updateBalanceAndTotal();
    SetGridCursor(GetGridCursorRow(), OUTLAY_COLUMN);
}

void DataGrid::setRow(int row)
{
    setNumberCell(row, INCOME_COLUMN);
    setNumberCell(row, OUTLAY_COLUMN);
    setNumberCell(row, BALANCE_COLUMN);
    setNormalCell(row, DESC_COLUMN);
    setNormalCell(row, COMMENTS_COLUMN);
    SetReadOnly(row, INCOME_COLUMN, false);
    SetReadOnly(row, OUTLAY_COLUMN, false);
    SetReadOnly(row, DESC_COLUMN, false);
    SetReadOnly(row, COMMENTS_COLUMN, false);
    SetReadOnly(row, BALANCE_COLUMN);
    SetCellEditor(row, INCOME_COLUMN, new wxGridCellFloatEditor(MONEY_LEN, 2));
    SetCellEditor(row, OUTLAY_COLUMN, new wxGridCellFloatEditor(MONEY_LEN, 2));
    clearRow(row);
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
    SetRowLabelValue(0, _("Initial balance"));
    money_to_str(buf, m_data->getInitial());
    SetCellValue(0, BALANCE_COLUMN, buf);
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
            SetCellValue(row, INCOME_COLUMN, buf);
        } else {
            money_to_str(buf, i->money);
            SetCellValue(row, OUTLAY_COLUMN, buf);
        }
        SetCellValue(row, DESC_COLUMN, i->desc.str);
        SetCellValue(row, COMMENTS_COLUMN, i->comment.str);
    }
    SetRowLabelValue(row, _("Total"));
    for (int i = 0; i < COLUMN_NUM; i++) SetReadOnly(row, i);
    wxFont font = MONO_FONT;
    font.MakeBold();
    SetCellFont(row, INCOME_COLUMN, font);
    SetCellFont(row, OUTLAY_COLUMN, font);
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
        SetCellValue(row, BALANCE_COLUMN, buf);
        if (i->money < 0) {
            income -= i->money;
        } else {
            outlay += i->money;
        }
    }
    money_to_str(buf, income);
    SetCellValue(row, INCOME_COLUMN, buf);
    money_to_str(buf, outlay);
    SetCellValue(row, OUTLAY_COLUMN, buf);
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
