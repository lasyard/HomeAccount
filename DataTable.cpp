#include <wx/msgdlg.h>

#include "DataTable.h"

void DataTable::prepareData()
{
    m_initial = 0;
    m_income = 0;
    m_outlay = 0;
    m_rows.clear();
    m_balance.clear();
    if (m_data == nullptr) return;
    m_initial = m_data->getInitial();
    for (auto p = m_data->ItemBegin(); p != m_data->ItemEnd(); ++p) {
        m_rows.push_back(p);
    }
    m_balance.resize(m_rows.size());
    updateBalanceAndTotal();
}

void DataTable::updateBalanceAndTotal()
{
    m_income = 0;
    m_outlay = 0;
    money_t balance = m_initial;
    for (int i = 0; i < m_rows.size(); ++i) {
        struct item *p = m_rows[i];
        balance = cal_item_balance(p, balance);
        m_balance[i] = balance;
        if (p->money < 0) {
            m_income -= p->money;
        } else {
            m_outlay += p->money;
        }
    }
}

// wxString DataTable::GetTypeName(int row, int col)
// {
//     if (col == m_incomeColumn || col == m_outlayColumn) {
//         return wxString::Format("%s:12,2", wxGRID_VALUE_FLOAT);
//     }
//     return wxGRID_VALUE_STRING;
// }

wxString DataTable::GetValue(int row, int col)
{
    char buf[MONEY_LEN];
    if (row == 0) {
        if (col == m_balanceColumn) {
            money_to_str(buf, m_initial);
            return buf;
        }
    } else if (row > m_rows.size()) {
        if (col == m_incomeColumn) {
            money_to_str(buf, m_income);
            return buf;
        } else if (col == m_outlayColumn) {
            money_to_str(buf, m_outlay);
            return buf;
        }
    } else {
        struct item *p = m_rows[row - 1];
        if (col == m_incomeColumn) {
            if (p->money < 0) {
                money_to_str(buf, -p->money);
                return buf;
            }
        } else if (col == m_outlayColumn) {
            if (p->money > 0) {
                money_to_str(buf, p->money);
                return buf;
            }
        } else if (col == m_balanceColumn) {
            if (!is_dummy_item(p)) {
                money_to_str(buf, m_balance[row - 1]);
                return buf;
            }
        } else if (col == m_descColumn) {
            return p->desc.str;
        } else if (col == m_commentsColumn) {
            return p->comment.str;
        }
    }
    return "";
}

bool DataTable::doSetValue(int row, int col, const wxString &value)
{
    wxASSERT(row >= 1 && row <= m_rows.size());
    struct item *it = m_rows[row - 1];
    money_t money = 0;
    bool showMsg = false;
    bool setMoney = false;
    if (col == m_incomeColumn) {
        money = -str_to_money(value);
        setMoney = true;
        if (it->money > 0) showMsg = true;
    } else if (col == m_outlayColumn) {
        setMoney = true;
        money = str_to_money(value);
        if (it->money < 0) showMsg = true;
    }
    if (setMoney) {
        if (showMsg) {
            wxMessageBox(_("Cannot set both income and outlay in one line"), _("App name"), wxOK | wxICON_ERROR);
        }
        item_set_money(it, money);
        updateBalanceAndTotal();
        m_data->setModified();
        if (GetView() != nullptr) {
            GetView()->AutoSizeColumn(m_balanceColumn);
        }
        return showMsg;
    }
    struct string str;
    string_init(&str);
    if (col == m_descColumn) {
        string_mock_slice(&str, value, '\0');
        if (item_set_desc(it, &str) == NULL) throw std::bad_alloc();
        m_data->setModified();
    } else if (col == m_commentsColumn) {
        string_mock_slice(&str, value, '\0');
        if (item_set_comment(it, &str) == NULL) throw std::bad_alloc();
        m_data->setModified();
    }
    return false;
}

void DataTable::SetValue(int row, int col, const wxString &value)
{
    doSetValue(row, col, value);
    if (GetView() != nullptr) {
        GetView()->AutoSizeColumn(col);
    }
}

bool DataTable::InsertRows(size_t pos, size_t numRows)
{
    if (pos < 1 || pos > m_rows.size()) return true;
    struct item *it = m_rows[pos - 1];
    for (size_t i = 0; i < numRows; i++) {
        it = insert_dummy_item(it);
    }
    prepareData();
    if (GetView() != nullptr) {
        wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED, pos, numRows);
        GetView()->ProcessTableMessage(msg);
    }
    return true;
}

bool DataTable::DeleteRows(size_t pos, size_t numRows)
{
    if (pos < 1 || pos > m_rows.size()) return true;
    size_t rowDeleted = 0;
    for (size_t i = 0; i < numRows; i++) {
        int row = pos + i;
        if (row > m_rows.size()) break;
        struct item *it = m_rows[row - 1];
        if (is_single_item(it)) {
            clear_item(it);
        } else {
            rowDeleted++;
            delete_item(it);
        }
    }
    prepareData();
    if (GetView() != nullptr) {
        wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, pos, rowDeleted);
        GetView()->ProcessTableMessage(msg);
    }
    return true;
}

wxString DataTable::GetRowLabelValue(int row)
{
    if (row == 0) {
        return _("Initial balance");
    } else if (row > m_rows.size()) {
        return _("Total");
    }
    struct item *p = m_rows[row - 1];
    if (ulist_is_first(&p->owner->items, &p->ulist)) {
        return p->owner->title.str;
    } else {
        return "";
    }
}

wxString DataTable::GetColLabelValue(int col)
{
    return m_columnLabels[col];
}

// Return attr will be taken ownership, so inc ref to keep it.
wxGridCellAttr *DataTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind)
{
    if (row > 0 && row <= m_rows.size()) {
        if (col == m_incomeColumn || col == m_outlayColumn) {
            m_moneyAttr->IncRef();
            return m_moneyAttr;
        } else if (col == m_balanceColumn) {
            if (m_balance[row - 1] >= 0) {
                m_moneyRoAttr->IncRef();
                return m_moneyRoAttr;
            } else {
                m_moneyRedRoAttr->IncRef();
                return m_moneyRedRoAttr;
            }
        }
    } else {
        m_moneyBoldRoAttr->IncRef();
        return m_moneyBoldRoAttr;
    }
    return nullptr;
}
