#ifndef _DATA_TABLE_H_
#define _DATA_TABLE_H_

#include <wx/grid.h>

#include <vector>

#include "c/data.h"
#include "file/DataFileRW.h"

class DataTable : public wxGridTableBase
{
public:
    DataTable() : m_data(nullptr)
    {
        setColumns();
        prepareData();
        initCellAttr();
    }

    DataTable(DataFileRW *data) : m_data(data)
    {
        setColumns();
        prepareData();
        initCellAttr();
    }

    virtual ~DataTable()
    {
        releaseCellAttr();
        safeDeleteData();
    }

    bool dataModified() const
    {
        if (m_data == nullptr) {
            return false;
        }
        return m_data->modified();
    }

    void setHaFile(HaFile *file)
    {
        if (m_data != nullptr) {
            m_data->setHaFile(file);
        }
    }

    void save()
    {
        if (m_data != nullptr) {
            m_data->save();
        }
    }

    void saveAs(const std::string &path)
    {
        if (m_data != nullptr) {
            m_data->saveAs(path);
        }
    }

    const char *dataFileName() const
    {
        return m_data->fileName();
    }

    int GetNumberRows()
    {
        return m_rows.size() + 2;
    }

    int GetNumberCols()
    {
        return m_columnLabels.size();
    }

    // virtual wxString GetTypeName(int row, int col);
    virtual wxString GetValue(int row, int col);
    virtual void SetValue(int row, int col, const wxString &value);
    virtual bool InsertRows(size_t pos = 0, size_t numRows = 1);
    virtual bool DeleteRows(size_t pos = 0, size_t numRows = 1);
    virtual wxString GetRowLabelValue(int row);
    virtual wxString GetColLabelValue(int col);

    virtual bool CanHaveAttributes()
    {
        return true;
    }

    virtual wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind);

    virtual void SetAttr(wxGridCellAttr *attr, int row, int col)
    {
    }

    int getOutlayColumn() const
    {
        return m_outlayColumn;
    }

protected:
    wxArrayString m_columnLabels;
    int m_timeColumn;
    int m_incomeColumn;
    int m_outlayColumn;
    int m_descColumn;
    int m_commentsColumn;
    int m_balanceColumn;

    wxGridCellFloatEditor *m_floatEditor;
    wxGridCellAttr *m_roAttr;
    wxGridCellAttr *m_moneyAttr;
    wxGridCellAttr *m_moneyRoAttr;
    wxGridCellAttr *m_moneyRedRoAttr;
    wxGridCellAttr *m_moneyBoldRoAttr;
    wxGridCellAttr *m_timeAttr;

    DataFileRW *m_data;

    std::vector<struct item *> m_rows;
    std::vector<money_t> m_balance;
    money_t m_initial;
    money_t m_income;
    money_t m_outlay;

    void safeDeleteData()
    {
        if (m_data != nullptr) {
            delete m_data;
        }
        m_data = nullptr;
        prepareData();
    }

    void initCellAttr()
    {
        m_roAttr = new wxGridCellAttr();
        m_roAttr->SetReadOnly();
        m_moneyAttr = new wxGridCellAttr();
        m_moneyAttr->SetAlignment(wxALIGN_RIGHT, wxALIGN_CENTER_VERTICAL);
        m_moneyBoldRoAttr = m_moneyAttr->Clone();
        wxFont monoFont(16, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        m_moneyAttr->SetFont(monoFont);
        m_moneyBoldRoAttr->SetFont(monoFont.MakeBold());
        m_moneyBoldRoAttr->SetReadOnly();
        m_moneyRoAttr = m_moneyAttr->Clone();
        m_moneyRoAttr->SetReadOnly();
        m_moneyRedRoAttr = m_moneyRoAttr->Clone();
        m_moneyRedRoAttr->SetTextColour(*wxRED);
        m_floatEditor = new wxGridCellFloatEditor(7, 2);
        // SetEditor will take the editor ownership, so inc the ref to keep it.
        m_floatEditor->IncRef();
        m_moneyAttr->SetEditor(m_floatEditor);
        m_timeAttr = m_roAttr->Clone();
        m_timeAttr->SetFont(monoFont.Smaller());
        m_timeAttr->SetTextColour(*wxBLUE);
    }

    void releaseCellAttr()
    {
        m_floatEditor->DecRef();
        m_moneyBoldRoAttr->DecRef();
        m_moneyRoAttr->DecRef();
        m_moneyAttr->DecRef();
    }

    void setColumns()
    {
        m_columnLabels.SetCount(6);
        m_columnLabels[m_incomeColumn = 0] = _("Income");
        m_columnLabels[m_outlayColumn = 1] = _("Outlay");
        m_columnLabels[m_descColumn = 2] = _("Description");
        m_columnLabels[m_commentsColumn = 3] = _("Comments");
        m_columnLabels[m_balanceColumn = 4] = _("Balance");
        m_columnLabels[m_timeColumn = 5] = _("Modification Time");
    }

    void prepareData();
    void updateBalanceAndTotal();
    bool doSetValue(int row, int col, const wxString &value);
    bool setMoney(struct item *it, money_t money);
};

#endif
