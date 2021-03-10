#ifndef _DATA_FILE_R_W_H_
#define _DATA_FILE_R_W_H_

#include "FileRW.h"
#include "except/InternalError.h"

#include "../c/data.h"
#include "../c/utils.h"

class DataFileRW : public FileRW
{
public:
    DataFileRW(const char *fileName, HaFile *file = nullptr) : FileRW(fileName, file), m_dt(new struct data)
    {
        init_data(m_dt);
    }

    virtual ~DataFileRW()
    {
        clear_data(m_dt);
        delete m_dt;
    }

    class PageIterator;
    friend class PageIterator;
    class PageIterator
    {
    public:
        PageIterator(struct ulist_item *p) : m_p(p)
        {
        }

        PageIterator &operator++()
        {
            m_p = m_p->next;
            return *this;
        }

        PageIterator &operator++(int)
        {
            ++(*this);
            return *this;
        }

        bool operator==(const PageIterator &obj)
        {
            return m_p == obj.m_p;
        }

        bool operator!=(const PageIterator &obj)
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

        operator struct page *()
        {
            return get_page(m_p);
        }

    protected:
        struct ulist_item *m_p;
    };

    PageIterator pageBegin()
    {
        return PageIterator(m_dt->pages.first);
    }

    PageIterator pageEnd()
    {
        return PageIterator(NULL);
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
                if (m_p != NULL) {
                    m_q = get_page(m_p)->items.first;
                }
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

        operator struct item *()
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

    struct data *getData() const
    {
        return m_dt;
    }

    int getItemsNum() const
    {
        return m_dt->items_num;
    }

    void setInitial(money_t value)
    {
        m_dt->initial = value;
    }

    money_t getInitial() const
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
    virtual void writeData(std::ostream &os) const;

    virtual const char *typeHeader() const
    {
        throw InternalError("Should not run to here: %s : %d", __FILE__, __LINE__);
    }

    virtual void writeInitial(std::ostream &os, money_t initial) const
    {
    }

private:
    void writeItem(std::ostream &os, const struct item *it) const;
    void writePage(std::ostream &os, const struct page *pg) const;
};

#endif
