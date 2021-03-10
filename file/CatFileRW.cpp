#include "CatFileRW.h"
#include "except/CatDupItem.h"
#include "except/CatFileError.h"

void CatFileRW::parseLine(const char *line)
{
    err_code err = parse_cat(m_cr, line);
    switch (err) {
    case ERR_BAD_ALLOC:
        throw std::bad_alloc();
    case ERR_CAT_DUP_WORD:
        throw CatDupItem();
    case ERR_CAT_FILE:
        throw CatFileError();
    default:
        break;
    }
}

void CatFileRW::writeWords(std::ostream &os, const struct rbtree_root *root) const
{
    struct rbtree_node *rb;
    for (rb = rbtree_first(root); rb != NULL; rb = rbtree_next(rb)) {
        os << get_word(rb)->name.str << std::endl;
    }
}

void CatFileRW::writeNode(std::ostream &os, const struct cat_node *node) const
{
    int i;
    for (i = 0; i < node->level; i++) {
        os << '#';
    }
    os << node->name.str << std::endl;
    writeWords(os, &node->words);
}

void CatFileRW::writeData(std::ostream &os) const
{
    struct mtree_node *node = mtree_pf_first(&m_cr->root);
    for (node = mtree_pf_next(&m_cr->root, node); node != NULL; node = mtree_pf_next(&m_cr->root, node)) {
        writeNode(os, get_cat_node(node));
    }
}
