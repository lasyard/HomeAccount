#ifndef _MTREE_H_
#define _MTREE_H_

#include "ulist.h"

struct mtree_node {
    struct ulist_item ulist;
    struct mtree_node *parent;
    struct ulist_head children;
};

#define get_mtree_node(ptr) ulist_entry(ptr, struct mtree_node, ulist)

#define mtree_entry(ptr, type, member) container_of(ptr, type, member)

#ifdef __cplusplus
extern "C" {
#endif

void mtree_node_init(struct mtree_node *node);

void mtree_add(struct mtree_node *parent, struct mtree_node *node);
void mtree_add_first(struct mtree_node *parent, struct mtree_node *node);
void mtree_del(struct mtree_node *node);

struct mtree_node *mtree_first_child(struct mtree_node *node);
struct mtree_node *mtree_last_child(struct mtree_node *node);

/* parent first tranversing */
struct mtree_node *mtree_pf_first(struct mtree_node *root);
struct mtree_node *mtree_pf_next(struct mtree_node *root, struct mtree_node *node);

/* child first tranversing */
struct mtree_node *mtree_cf_first(struct mtree_node *root);
struct mtree_node *mtree_cf_next(struct mtree_node *root, struct mtree_node *node);

BOOL mtree_is_root(const struct mtree_node *node);
BOOL mtree_is_leaf(const struct mtree_node *node);

#ifdef __cplusplus
}
#endif

#endif
