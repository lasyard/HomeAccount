#ifndef _CAT_MODEL_H_
#define _CAT_MODEL_H_

#include <wx/dataview.h>

#include "CatItem.h"

class CatModel : public wxDataViewModel
{
public:
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
        return 2;
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
        return "string";
    }

    virtual void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const
    {
        CatItem *it = static_cast<CatItem *>(item.GetID());
        variant = it->getName();
    }

    virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
    {
        return true;
    }

protected:
    CatItem m_root;
};

#endif
