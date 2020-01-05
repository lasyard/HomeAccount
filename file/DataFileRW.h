#ifndef _DATA_FILE_R_W_H_
#define _DATA_FILE_R_W_H_

#include "FileRW.h"

#include "../c/data.h"
#include "../c/utils.h"

class DataFileRW : public FileRW
{
public:
    DataFileRW(const char *fileName, HaFile *file = NULL) : FileRW(fileName, file), m_dt(new struct data)
    {
        init_data(m_dt);
    }

    virtual ~DataFileRW()
    {
        clear_data(m_dt);
        delete m_dt;
    }

    class pageIterator;
    friend class pageIterator;
    class pageIterator
    {
    public:
        pageIterator(struct ulist_item *p) : m_p(p)
        {
        }
        pageIterator &operator++()
        {
            m_p = m_p->next;
            return *this;
        }
        pageIterator &operator++(int)
        {
            ++(*this);
            return *this;
        }
        bool operator==(const pageIterator &obj)
        {
            return m_p == obj.m_p;
        }
        bool operator!=(const pageIterator &obj)
        {
            return !(*this == obj);
        }
        struct page &operator*()
        {
            return *get_page(m_p);
        }
        struct page *operator->()
        {
            return get_page(m_p);
        }

    protected:
        struct ulist_item *m_p;
    };

    pageIterator pageBegin()
    {
        return pageIterator(m_dt->pages.first);
    }

    pageIterator pageEnd()
    {
        return pageIterator(NULL);
    }

    class ItemIterator;
    friend class ItemIterator;
    class ItemIterator
    {
    public:
        ItemIterator(struct ulist_item *p, struct ulist_item *q) : m_p(p), m_q(q)
        {
        }
        ItemIterator &operator++()
        {
            m_q = m_q->next;
            if (m_q == NULL) {
                m_p = m_p->next;
                if (m_p != NULL) m_q = get_page(m_p)->items.first;
            }
            return *this;
        }
        ItemIterator &operator++(int)
        {
            ++(*this);
            return *this;
        }
        bool operator==(const ItemIterator &obj)
        {
            return m_p == obj.m_p && m_q == obj.m_q;
        }
        bool operator!=(const ItemIterator &obj)
        {
            return !(*this == obj);
        }
        struct item &operator*()
        {
            return *get_item(m_q);
        }
        struct item *operator->()
        {
            return get_item(m_q);
        }

    protected:
        struct ulist_item *m_p;
        struct ulist_item *m_q;
    };

    ItemIterator ItemBegin()
    {
        struct ulist_item *firstPagePtr = m_dt->pages.first;
        struct ulist_item *firstItemPtr = (firstPagePtr == NULL) ? NULL : get_page(firstPagePtr)->items.first;
        return ItemIterator(firstPagePtr, firstItemPtr);
    }

    ItemIterator ItemEnd()
    {
        return ItemIterator(NULL, NULL);
    }

    virtual void writeStream(std::ostream &os) const
    {
        writeData(os, m_dt);
    }

    struct data *getData() const
    {
        return m_dt;
    }

    int getItemsNum() const
    {
        return m_dt->items_num;
    }

    void setInitial(long value)
    {
        m_dt->initial = value;
    }

    long getInitial() const
    {
        return m_dt->initial;
    }

    virtual void afterLoad()
    {
        add_dummy_item_to_empty_page(m_dt);
    }

    virtual void clearData()
    {
        clear_data(m_dt);
    }

protected:
    struct data *m_dt;

    virtual void parseLine(const char *line);

    virtual void writeInitial(std::ostream &os, long initial) const
    {
        char buf[MONEY_LEN];
        money_to_str(buf, -initial);
        os << buf << std::endl;
    }

private:
    void writeItem(std::ostream &os, const struct item *it) const;
    void writePage(std::ostream &os, const struct page *pg) const;
    void writeData(std::ostream &os, const struct data *dt) const;
};

#endif
