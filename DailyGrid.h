#ifndef _DAILY_GRID_H_
#define _DAILY_GRID_H_

#include "DataGrid.h"
#include "file/CatFileRW.h"

class DailyGrid : public DataGrid
{
    wxDECLARE_DYNAMIC_CLASS(DailyGrid);

public:
    DailyGrid() : DataGrid(), m_cat(NULL)
    {
    }

    virtual ~DailyGrid()
    {
        safeDeleteCat();
    }

    virtual void initGrid();

    virtual bool catModified()
    {
        if (m_cat == NULL) return false;
        return m_cat->modified();
    }

    virtual void setCatFileRW(CatFileRW *cat)
    {
        safeDeleteCat();
        m_cat = cat;
        setCatChoices();
    }

    virtual CatFileRW *catFileRW() const
    {
        return m_cat;
    }

    virtual void setHaFile(HaFile *file)
    {
        DataGrid::setHaFile(file);
        if (m_cat != nullptr) {
            m_cat->setHaFile(file);
        }
    }

    virtual void onCellChange(wxGridEvent &event);
    virtual void updateCat();

    virtual void updateData()
    {
        DataGrid::updateData();
        updateCat();
    }

protected:
    static const int COLUMN_NUM = DataGrid::COLUMN_NUM + 1;
    static const int CLASS_COLUMN = DataGrid::COLUMN_NUM;

    CatFileRW *m_cat;
    wxArrayString m_catChoiceI;
    wxArrayString m_catChoiceO;

    void safeDeleteCat()
    {
        if (m_cat != nullptr) delete m_cat;
        m_cat = nullptr;
    }

    void setCatChoice(wxArrayString &arr, struct mtree_node *root);

    void setCatChoices()
    {
        if (mtree_is_leaf(&m_cat->getCatRoot()->root)) {
            setCatChoice(m_catChoiceI, NULL);
            setCatChoice(m_catChoiceO, NULL);
        } else {
            setCatChoice(m_catChoiceI, mtree_first_child(&m_cat->getCatRoot()->root));
            setCatChoice(m_catChoiceO, mtree_last_child(&m_cat->getCatRoot()->root));
        }
    }

    struct cat_node *getCatNode(struct mtree_node *root, const char *name) const;
    struct cat_node *getCatNode(long money, wxString str) const;
    void updateCat(int row, struct item *it);
    void changeCat(int row, long money);

    DECLARE_EVENT_TABLE()
};

#endif
