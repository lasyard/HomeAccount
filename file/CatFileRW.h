#ifndef _CAT_FILE_R_W_H_
#define _CAT_FILE_R_W_H_

#include "FileRW.h"

#include "../c/cat.h"

class CatFileRW : public FileRW
{
public:
    CatFileRW(const char *fileName, HaFile *file = NULL) : FileRW(fileName, file)
    {
        m_cr = new struct cat_root;
        init_cat_root(m_cr);
    }

    virtual ~CatFileRW()
    {
        release_cat_root(m_cr);
        delete m_cr;
    }

    struct cat_root *getCatRoot() const
    {
        return m_cr;
    }

    virtual void clearData()
    {
        release_cat_root(m_cr);
    }

    virtual void afterSave() const
    {
    }

protected:
    struct cat_root *m_cr;

    virtual void writeStream(std::ostream &os) const
    {
        writeCat(os, m_cr);
    }

    virtual void parseLine(const char *line);

private:
    void writeWords(std::ostream &os, const struct rbtree_root *root) const;
    void writeNode(std::ostream &os, const struct cat_node *node) const;
    void writeCat(std::ostream &os, struct cat_root *cr) const;
};

#endif
