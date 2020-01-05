#ifndef _DAILY_GRID_H_
#define _DAILY_GRID_H_

#include "DataGrid.h"
#include "file/CatFileRW.h"

class DailyGrid : public DataGrid
{
public:
    DailyGrid(wxWindow *parent, wxWindowID id);

    virtual ~DailyGrid()
    {
        if (m_cat != NULL) delete m_cat;
        delete m_catChoiceI;
        delete m_catChoiceO;
    }

    bool catModified()
    {
        if (m_cat == NULL) return false;
        return m_cat->modified();
    }

    void setCatFileRW(CatFileRW *cat)
    {
        if (m_cat != NULL) delete m_cat;
        m_cat = cat;
        setCatChoices();
    }

    CatFileRW *catFileRW() const
    {
        return m_cat;
    }

    virtual void onCellChange(wxGridEvent &event);
    void updateCat();

protected:
    static const int ColumnNum = DataGrid::ColumnNum + 1;
    static const int ClassIndex = DataGrid::ColumnNum;

    CatFileRW *m_cat;
    wxArrayString *m_catChoiceI;
    wxArrayString *m_catChoiceO;

    virtual void updateData(bool setModified = false)
    {
        DataGrid::updateData(setModified);
        updateCat();
    }

    void setCatChoice(wxArrayString *arr, struct mtree_node *root);

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
    void showAllData();
    void changeCat(int row, long money);

    DECLARE_EVENT_TABLE()
};

#endif
