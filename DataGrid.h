#ifndef _DATA_GRID_H_
#define _DATA_GRID_H_

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/grid.h>

#include "c/utils.h"

class DataGrid : public wxGrid
{
    wxDECLARE_DYNAMIC_CLASS(DataGrid);

public:
    DataGrid()
    {
        m_logo = wxArtProvider::GetBitmap("logo");
    }

    virtual ~DataGrid()
    {
    }

    void setGrid();

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
    virtual void DrawCornerLabel(wxDC &dc);

    virtual wxPen GetRowGridLinePen(int row)
    {
        if (GetRowLabelValue(row + 1) != "") return *wxLIGHT_GREY_PEN;
        return *wxWHITE_PEN;
    }

    virtual wxPen GetColGridLinePen(int col)
    {
        return *wxLIGHT_GREY_PEN;
    }

protected:
    wxBitmap m_logo;

    void safeClearCell(int row, int col)
    {
        if (!IsReadOnly(row, col)) SetCellValue(row, col, "");
    }

    DECLARE_EVENT_TABLE()
};

#endif
