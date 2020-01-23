#ifndef _CAT_ITEM_H_
#define _CAT_ITEM_H_

#include <vector>

#include "wx/string.h"

#include "c/cat.h"
#include "c/utils.h"

class CatItem
{
public:
    CatItem() : m_parent(nullptr), m_children(), m_name(""), m_count(0), m_total("")
    {
    }

    CatItem(CatItem *parent, const struct string &name)
        : m_parent(parent), m_children(), m_name(name.str), m_count(0), m_total("")
    {
        m_parent = parent;
        m_name = name.str;
    }

    virtual ~CatItem()
    {
        for (auto it = m_children.cbegin(); it != m_children.cend(); ++it) {
            delete *it;
        }
    }

    CatItem *getParent() const
    {
        return m_parent;
    }

    void addChild(CatItem *item)
    {
        m_children.push_back(item);
    }

    const std::vector<CatItem *> &getChildren() const
    {
        return m_children;
    }

    bool hasChild()
    {
        return !m_children.empty();
    }

    const wxString &getName() const
    {
        return m_name;
    }

    void buildRoot(struct cat_root *cat_root)
    {
        buildSubCats(&cat_root->root);
    }

    void setCount(int count)
    {
        m_count = count;
    }

    int getCount() const
    {
        return m_count;
    }

    void setTotal(money_t total)
    {
        char buf[MONEY_LEN];
        money_to_str(buf, total);
        m_total = buf;
    }

    wxString getTotal() const
    {
        return m_total;
    }

protected:
    CatItem *m_parent;
    std::vector<CatItem *> m_children;
    wxString m_name;
    int m_count;
    wxString m_total;

    void buildSubCats(struct mtree_node *node)
    {
        if (mtree_is_leaf(node)) return;
        for (struct mtree_node *m = mtree_first_child(node); m != NULL; m = mtree_next_child(m)) {
            struct cat_node *n = get_cat_node(m);
            CatItem *item = new CatItem(this, n->name);
            item->setTotal(n->total);
            item->setCount(-1);
            addChild(item);
            item->buildChildren(n);
        }
    }

    void buildChildren(struct cat_node *node)
    {
        struct cat_node *cat = get_cat_node(node);
        for (struct rbtree_node *n = rbtree_first(&cat->words); n != NULL; n = rbtree_next(n)) {
            struct word *w = get_word(n);
            CatItem *item = new CatItem(this, w->name);
            item->setCount(w->count);
            item->setTotal(w->total);
            addChild(item);
        }
        buildSubCats(&cat->mtree);
    }
};

#endif
