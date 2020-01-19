#ifndef _DAILY_TABLE_H_
#define _DAILY_TABLE_H_

#include "DataTable.h"
#include "file/CatFileRW.h"

#include "c/cal.h"

class DailyTable : public DataTable
{
public:
    DailyTable() : DataTable(), m_cat(nullptr)
    {
        setColumns();
        initCellAttr();
    }

    DailyTable(DataFileRW *data, CatFileRW *cat) : DataTable(data), m_cat(cat)
    {
        setColumns();
        updateCat();
        initCellAttr();
    }

    virtual ~DailyTable()
    {
        releaseCellAttr();
        safeDeleteCat();
    }

    bool catModified()
    {
        if (m_cat == nullptr) return false;
        return m_cat->modified();
    }

    void setHaFile(HaFile *file)
    {
        DataTable::setHaFile(file);
        if (m_cat != nullptr) {
            m_cat->setHaFile(file);
        }
    }

    struct cat_root *getCatRoot() const
    {
        return m_cat->getCatRoot();
    }

    void saveCat()
    {
        if (m_cat != nullptr) m_cat->save();
    }

    void saveCatAs(const std::string &path)
    {
        if (m_cat != nullptr) m_cat->saveAs(path);
    }

    const char *catFileName() const
    {
        return m_cat->fileName();
    }

    virtual wxString GetValue(int row, int col);
    virtual void SetValue(int row, int col, const wxString &value);
    virtual wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind);

protected:
    int m_categoryColumn;

    wxGridCellChoiceEditor *m_choiceEditorI;
    wxGridCellChoiceEditor *m_choiceEditorO;
    wxGridCellAttr *m_categoryAttrI;
    wxGridCellAttr *m_categoryAttrO;
    wxGridCellAttr *m_categoryNoCatAttrI;
    wxGridCellAttr *m_categoryNoCatAttrO;

    CatFileRW *m_cat;

    void safeDeleteCat()
    {
        if (m_cat != nullptr) delete m_cat;
        m_cat = nullptr;
    }

    void setColumns()
    {
        m_categoryColumn = m_columnLabels.size();
        m_columnLabels.Add(_("Category"));
    }

    void updateCat()
    {
        for (DataFileRW::ItemIterator i = m_data->ItemBegin(); i != m_data->ItemEnd(); ++i) {
            update_item_cat(i, m_cat->getCatRoot());
        }
    }

    void setCatChoice(wxArrayString &arr, struct mtree_node *root)
    {
        arr.clear();
        arr.Add(_("Unclassified"));
        if (root == NULL) return;
        struct mtree_node *node;
        for (node = mtree_pf_first(root); node != NULL; node = mtree_pf_next(root, node)) {
            arr.Add(get_cat_node(node)->name.str);
        }
    }

    void setCatChoices(wxArrayString &choiceI, wxArrayString &choiceO)
    {
        if (m_cat == nullptr) {
            setCatChoice(choiceI, NULL);
            setCatChoice(choiceO, NULL);
            return;
        }
        mtree_node *root = &m_cat->getCatRoot()->root;
        if (mtree_is_leaf(root)) {
            setCatChoice(choiceI, NULL);
            setCatChoice(choiceO, NULL);
        } else {
            setCatChoice(choiceI, mtree_first_child(root));
            setCatChoice(choiceO, mtree_last_child(root));
        }
    }

    void initCellAttr()
    {
        m_categoryAttrI = new wxGridCellAttr();
        m_categoryAttrO = m_categoryAttrI->Clone();
        m_categoryAttrI->SetTextColour(wxColor(0x00008800));
        m_categoryAttrO->SetTextColour(wxColor(0x000000CC));
        wxArrayString choiceI, choiceO;
        setCatChoices(choiceI, choiceO);
        m_choiceEditorI = new wxGridCellChoiceEditor(choiceI);
        m_choiceEditorI->IncRef();
        m_categoryAttrI->SetEditor(m_choiceEditorI);
        m_choiceEditorO = new wxGridCellChoiceEditor(choiceO);
        m_choiceEditorO->IncRef();
        m_categoryAttrO->SetEditor(m_choiceEditorO);
        m_categoryNoCatAttrI = m_categoryAttrI->Clone();
        m_categoryNoCatAttrI->SetBackgroundColour(*wxYELLOW);
        m_categoryNoCatAttrO = m_categoryAttrO->Clone();
        m_categoryNoCatAttrO->SetBackgroundColour(*wxYELLOW);
    }

    void releaseCellAttr()
    {
        m_categoryAttrI->DecRef();
        m_categoryAttrO->DecRef();
        m_categoryNoCatAttrI->DecRef();
        m_categoryNoCatAttrO->DecRef();
        m_choiceEditorI->DecRef();
        m_choiceEditorO->DecRef();
    }

    bool setCatNotAllowed(const struct item *it) const
    {
        return string_is_empty(&it->desc) || it->money == 0;
    }

    void changeCat(struct item *it, const char *name);
};

#endif
