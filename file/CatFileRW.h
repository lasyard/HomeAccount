#ifndef _CAT_FILE_R_W_H_
#define _CAT_FILE_R_W_H_

#include "FileRW.h"

#include "../c/cat.h"

class CatFileRW : public FileRW
{
public:
    CatFileRW() : CatFileRW(HaFile::CAT_FILE_NAME)
    {
    }

    CatFileRW(const char *fileName, HaFile *file = nullptr) : FileRW(fileName, file)
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

    virtual const HaFile::FileType type()
    {
        return HaFile::CAT;
    }

protected:
    struct cat_root *m_cr;

    virtual void parseLine(const char *line);

private:
    void writeWords(std::ostream &os, const struct rbtree_root *root) const;
    void writeNode(std::ostream &os, const struct cat_node *node) const;
    virtual void writeData(std::ostream &os) const;

    virtual const char *header() const
    {
        return "HACT";
    }
};

#endif
