#ifndef _CAT_MODEL_H_
#define _CAT_MODEL_H_

#include <wx/dataview.h>

#include "CatItem.h"

class CatModel : public wxDataViewModel
{
public:
    static const int NAME_COLUMN = 0;
    static const int COUNT_COLUMN = 1;
    static const int TOTAL_COLUMN = 2;
    static const int INUSE_COLUMN = 3;
    static const int COLUMN_NUM = 4;

    CatModel(struct cat_root *cat) : m_root()
    {
        m_root.buildRoot(cat);
    }

    virtual bool IsContainer(const wxDataViewItem &item) const
    {
        if (item.GetID() == nullptr) return true;
        CatItem *it = static_cast<CatItem *>(item.GetID());
        if (it->hasChild()) return true;
        return false;
    }

    virtual wxDataViewItem GetParent(const wxDataViewItem &item) const
    {
        CatItem *it = static_cast<CatItem *>(item.GetID());
        return wxDataViewItem(it->getParent());
    }

    virtual unsigned int GetChildren(const wxDataViewItem &item, wxDataViewItemArray &children) const
    {
        int count = 0;
        const CatItem *it;
        if (item.GetID() == nullptr) {
            it = &m_root;
        } else {
            it = static_cast<const CatItem *>(item.GetID());
        }
        for (auto i = it->getChildren().cbegin(); i != it->getChildren().cend(); ++i) {
            children.Add(wxDataViewItem(*i));
            count++;
        }
        return count;
    }

    virtual unsigned int GetColumnCount() const
    {
        return COLUMN_NUM;
    }

    /**
     * "bool"
     * "char"
     * "datetime"
     * "double"
     * "list"
     * "long"
     * "longlong"
     * "string"
     * "ulonglong"
     * "arrstring"
     * "void*"
     */
    virtual wxString GetColumnType(unsigned int col) const
    {
        // Seems no use, decided by column type when columns are added.
        return "string";
    }

    virtual void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const
    {
        CatItem *it = static_cast<CatItem *>(item.GetID());
        switch (col) {
            case NAME_COLUMN: {
                variant = it->getName();
            } break;
            case COUNT_COLUMN: {
                variant = wxString::Format("%d", it->getCount());
            } break;
            case TOTAL_COLUMN: {
                variant = it->getTotal();
            } break;
            case INUSE_COLUMN: {
                if (it->getCount() > 0) {
                    variant << wxArtProvider::GetBitmap("checkmark");
                } else {
                    variant << wxArtProvider::GetBitmap("cross");
                }
            } break;
            default:
                break;
        }
    }

    virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
    {
        return true;
    }

    void *income()
    {
        return m_root.getChildren()[0];
    }

    void *outlay()
    {
        return m_root.getChildren()[1];
    }

protected:
    CatItem m_root;
};

#endif
