#include "DataGrid.h"
#include "DataTable.h"

wxIMPLEMENT_DYNAMIC_CLASS(DataGrid, wxGrid);

BEGIN_EVENT_TABLE(DataGrid, wxGrid)
EVT_KEY_DOWN(DataGrid::onKeyDown)
END_EVENT_TABLE()

void DataGrid::setGrid()
{
    BeginBatch();
    SetColMinimalAcceptableWidth(80);
    SetRowMinimalAcceptableHeight(18);
    SetRowLabelSize(wxGRID_AUTOSIZE);
    SetColLabelSize(wxGRID_AUTOSIZE);
    if (GetRowLabelSize() < m_logo.GetWidth() + 2) SetRowLabelSize(m_logo.GetWidth() + 2);
    if (GetColLabelSize() < m_logo.GetHeight() + 2) SetColLabelSize(m_logo.GetHeight() + 2);
    AutoSizeColumns(false);
    AutoSizeRows(false);
    DisableDragColMove();
    EndBatch();
}

void DataGrid::onKeyDown(wxKeyEvent &event)
{
    if (event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER) {
        BeginBatch();
        int row = GetGridCursorRow();
        InsertRows(row);
        if (event.RawControlDown()) {
            for (int col = 0; col < GetNumberCols(); col++) {
                if (!IsReadOnly(row + 1, col)) {
                    SetCellValue(row + 1, col, GetCellValue(row, col));
                    if (!event.ShiftDown()) {
                        SetCellValue(row, col, "");
                    }
                }
            }
        }
        SetGridCursor(row, static_cast<DataTable *>(GetTable())->getOutlayColumn());
        EndBatch();
    } else if (event.GetKeyCode() == WXK_DELETE) {
        BeginBatch();
        if (IsSelection()) {
            wxGridCellCoordsArray cells = GetSelectedCells();
            for (size_t i = 0; i < cells.Count(); ++i) {
                safeClearCell(cells[i].GetRow(), cells[i].GetCol());
            }
            wxGridCellCoordsArray topLeft = GetSelectionBlockTopLeft();
            wxGridCellCoordsArray bottomRight = GetSelectionBlockBottomRight();
            int row, col;
            for (size_t i = 0; i < topLeft.Count(); ++i) {
                for (row = topLeft[i].GetRow(); row <= bottomRight[i].GetRow(); row++) {
                    for (col = topLeft[i].GetCol(); col <= bottomRight[i].GetCol(); col++) {
                        safeClearCell(row, col);
                    }
                }
            }
            wxArrayInt cols = GetSelectedCols();
            for (size_t i = 0; i < cols.Count(); ++i) {
                for (row = 0; row < GetNumberRows(); row++) {
                    safeClearCell(row, cols[i]);
                }
            }
            wxArrayInt rows = GetSelectedRows();
            if (rows.Count() > 0) {
                rows.Sort([](int a, int b) -> int { return a < b ? 1 : a > b ? -1 : 0; });
                int rowStart = rows[0];
                int numRows = 1;
                for (size_t i = 1; i < rows.Count(); ++i) {
                    if (rows[i] == rowStart - 1) {
                        rowStart--;
                        numRows++;
                    } else {
                        DeleteRows(rowStart, numRows);
                        rowStart = rows[i];
                        numRows = 1;
                    }
                }
                DeleteRows(rowStart, numRows);
            }
            ClearSelection();
        } else {
            safeClearCell(GetGridCursorRow(), GetGridCursorCol());
        }
        EndBatch();
    }
    wxGrid::OnKeyDown(event);
}

void DataGrid::DrawCornerLabel(wxDC &dc)
{
    dc.DrawBitmap(m_logo, 1, 1);
}
