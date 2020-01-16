#ifndef _DAILY_GRID_H_
#define _DAILY_GRID_H_

#include "DataGrid.h"

class DailyGrid : public DataGrid
{
    wxDECLARE_DYNAMIC_CLASS(DailyGrid);

public:
    DailyGrid()
    {
    }

    virtual ~DailyGrid()
    {
    }

protected:
    DECLARE_EVENT_TABLE()
};

#endif
