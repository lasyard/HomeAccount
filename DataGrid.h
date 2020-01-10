#ifndef _DATA_GRID_H_
#define _DATA_GRID_H_

#include <wx/grid.h>

#include "file/DataFileRW.h"

class DataGrid : public wxGrid
{
    wxDECLARE_DYNAMIC_CLASS(DataGrid);

public:
    DataGrid();

    virtual ~DataGrid()
    {
        safeDeleteData();
    }

    virtual void initGrid();

    virtual bool dataModified()
    {
        if (m_data == nullptr) return false;
        return m_data->modified();
    }

    virtual void setDataFileRW(DataFileRW *data)
    {
        safeDeleteData();
        m_data = data;
        showAllData();
    }

    virtual void setHaFile(HaFile *file)
    {
        if (m_data != nullptr) {
            m_data->setHaFile(file);
        }
    }

    virtual DataFileRW *dataFileRW() const
    {
        return m_data;
    }

    virtual void onCellChange(wxGridEvent &event);
    virtual void onKeyDown(wxKeyEvent &event);

    virtual void scrollToDay(int day);
    virtual void updateData();

protected:
    static const int COLUMN_NUM = 5;
    static const int INCOME_COLUMN = 0;
    static const int OUTLAY_COLUMN = 1;
    static const int DESC_COLUMN = 2;
    static const int COMMENTS_COLUMN = 3;
    static const int BALANCE_COLUMN = 4;

    static const wxFont MONO_FONT;

    DataFileRW *m_data;

    void safeDeleteData()
    {
        if (m_data != nullptr) delete m_data;
        m_data = nullptr;
    }

    void clearCell(int row, int col)
    {
        SetCellValue(row, col, "");
    }

    void clearRow(int row)
    {
        for (int i = 0; i < GetNumberCols(); i++) clearCell(row, i);
    }

    void setNumberCell(int row, int col)
    {
        SetCellAlignment(row, col, wxALIGN_RIGHT, wxALIGN_CENTER);
        SetCellFont(row, col, MONO_FONT);
        SetCellTextColour(row, col, GetDefaultCellTextColour());
    }

    void setNormalCell(int row, int col)
    {
        SetCellFont(row, col, GetDefaultCellFont());
        SetCellTextColour(row, col, GetDefaultCellTextColour());
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
