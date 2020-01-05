#ifndef _DATA_GRID_H_
#define _DATA_GRID_H_

#include <wx/grid.h>

#include "file/DataFileRW.h"

class DataGrid : public wxGrid
{
public:
    DataGrid(wxWindow *parent, wxWindowID id);

    virtual ~DataGrid()
    {
        if (m_data != NULL) delete m_data;
    }

    bool dataModified()
    {
        if (m_data == NULL) return false;
        return m_data->modified();
    }

    void setDataFileRW(DataFileRW *data)
    {
        if (m_data != NULL) delete m_data;
        m_data = data;
        showAllData();
    }

    DataFileRW *dataFileRW() const
    {
        return m_data;
    }

    virtual void onCellChange(wxGridEvent &event);
    void onKeyDown(wxKeyEvent &event);

    void onResizeCol(wxCommandEvent &event)
    {
        int col = event.GetInt();
        AutoSizeColumn(col, false);
    }

    void scrollToDay(int day);

protected:
    static const int ColumnNum = 5;
    static const int IncomeIndex = 0;
    static const int OutlayIndex = 1;
    static const int DescIndex = 2;
    static const int CommentIndex = 3;
    static const int BalanceIndex = 4;

    DataFileRW *m_data;

    virtual void updateData(bool setModified = false);

    void clearCell(int row, int col)
    {
        SetCellValue(row, col, "");
    }

    void clearRow(int row)
    {
        for (int i = 0; i < GetNumberCols(); i++) clearCell(row, i);
    }

    void setRow(int row);

    void setRows(int pos, int num)
    {
        for (int row = pos; row < pos + num; row++) setRow(row);
    }

    struct item *dataPosFromRow(int row)
    {
        struct item *it = nth_item_of_data(m_data->getData(), row);
        return it;
    }

    void showMainData();
    void updateBalanceAndTotal();
    void showAllData();
    void insertRowToData(int row);
    void insertRowToDataUp(int row);
    bool deleteRowsFromData(wxArrayInt &rows);
    bool clearCellData(int row, int col);
    void processItemChange(struct item *it, int row, int col);

    DECLARE_EVENT_TABLE()
};

#endif
