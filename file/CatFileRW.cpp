#include "CatFileRW.h"
#include "except/CatDupItem.h"
#include "except/CatFileError.h"

void CatFileRW::parseLine(const char *line)
{
    const char *p;
    int i;
    int level;
    for (level = 0, p = line; *p == '#'; p++) level++;
    if (level > 3) throw CatFileError();
    struct string name;
    string_mock_slice(&name, p, '\0');
    struct mtree_node *parent;
    if (level == 0) {
        int dup;
        for (parent = &m_cr->root; !mtree_is_leaf(parent); parent = mtree_last_child(parent))
            ;
        if (add_word(get_cat_node(parent), &name, &dup) == NULL) {
            if (dup) {
                throw CatDupItem();
            } else {
                throw std::bad_alloc();
            }
        }
    } else {
        for (i = 3, parent = &m_cr->root; i > level && parent != NULL; i--) {
            parent = mtree_last_child(parent);
        }
        if (parent == NULL) throw CatFileError();
        if (add_cat_node(parent, &name, level) == NULL) {
            throw std::bad_alloc();
        }
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
    for (i = 0; i < node->level; i++) os << '#';
    os << node->name.str << std::endl;
    writeWords(os, &node->words);
}

void CatFileRW::writeCat(std::ostream &os, struct cat_root *cr) const
{
    struct mtree_node *node = mtree_pf_first(&cr->root);
    for (node = mtree_pf_next(&cr->root, node); node != NULL; node = mtree_pf_next(&cr->root, node)) {
        writeNode(os, get_cat_node(node));
    }
}
