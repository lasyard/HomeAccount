#ifndef _DATA_GRID_H_
#define _DATA_GRID_H_

#include <wx/grid.h>

#include "c/utils.h"

class DataGrid : public wxGrid
{
    wxDECLARE_DYNAMIC_CLASS(DataGrid);

public:
    DataGrid()
    {
    }

    virtual ~DataGrid()
    {
    }

    void setGrid()
    {
        SetColMinimalAcceptableWidth(80);
        SetRowMinimalAcceptableHeight(18);
        SetRowLabelSize(wxGRID_AUTOSIZE);
        SetColLabelSize(wxGRID_AUTOSIZE);
        AutoSizeColumns(false);
        AutoSizeRows(false);
        DisableDragColMove();
    }

    void scrollToDay(int day)
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

    virtual void onKeyDown(wxKeyEvent &event);

protected:
    void safeClearCell(int row, int col)
    {
        if (!IsReadOnly(row, col)) SetCellValue(row, col, "");
    }

    DECLARE_EVENT_TABLE()
};

#endif
