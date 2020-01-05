#include "mtree.h"

void mtree_node_init(struct mtree_node *node)
{
    node->parent = NULL;
    ulist_head_init(&node->children);
    ulist_item_init(&node->ulist);
}

void mtree_add(struct mtree_node *parent, struct mtree_node *node)
{
    node->parent = parent;
    ulist_add(&parent->children, &node->ulist);
}

void mtree_add_first(struct mtree_node *parent, struct mtree_node *node)
{
    node->parent = parent;
    ulist_add_head(&parent->children, &node->ulist);
}

void mtree_del(struct mtree_node *node)
{
    if (node->parent != NULL) ulist_del(&node->parent->children, &node->ulist);
    node->parent = NULL;
}

struct mtree_node *mtree_first_child(struct mtree_node *node)
{
    if (ulist_is_empty(&node->children)) return NULL;
    return get_mtree_node(ulist_first(&node->children));
}

struct mtree_node *mtree_last_child(struct mtree_node *node)
{
    if (ulist_is_empty(&node->children)) return NULL;
    return get_mtree_node(ulist_last(&node->children));
}

struct mtree_node *mtree_pf_first(struct mtree_node *root)
{
    return root;
}

struct mtree_node *mtree_pf_next(struct mtree_node *root, struct mtree_node *node)
{
    if (!ulist_is_empty(&node->children)) return get_mtree_node(ulist_first(&node->children));
    for (;;) {
        if (node == root) return NULL;
        if (node->ulist.next != NULL) break;
        node = node->parent;
    }
    return get_mtree_node(node->ulist.next);
}

struct mtree_node *mtree_cf_first(struct mtree_node *root)
{
    struct mtree_node *node = root;
    while (!ulist_is_empty(&node->children)) {
        node = get_mtree_node(ulist_first(&node->children));
    }
    return node;
}

struct mtree_node *mtree_cf_next(struct mtree_node *root, struct mtree_node *node)
{
    if (node == root) return NULL;
    if (node->ulist.next == NULL) return node->parent;
    return mtree_cf_first(get_mtree_node(node->ulist.next));
}

BOOL mtree_is_root(const struct mtree_node *node)
{
    return node->parent == NULL;
}

BOOL mtree_is_leaf(const struct mtree_node *node)
{
    return ulist_is_empty(&node->children);
}
