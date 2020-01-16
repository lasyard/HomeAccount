#include <stdlib.h>

#include "cat.h"

static void __init_word(struct word *w)
{
    string_init(&w->name);
    rbtree_node_init(&w->rbtree);
}

static void __release_word(struct word *w)
{
    string_release(&w->name);
    free(w);
}

static struct word *__new_word()
{
    struct word *p;
    if ((p = (struct word *)malloc(sizeof(struct word))) == NULL) return NULL;
    __init_word(p);
    return p;
}

static struct word *__insert_word(struct rbtree_root *root, struct word *w)
{
    struct rbtree_node **p = &root->node;
    struct rbtree_node *parent = NULL;
    struct word *ww;
    int res;
    while (*p) {
        parent = *p;
        ww = rbtree_entry(parent, struct word, rbtree);
        if ((res = string_compare(&w->name, &ww->name)) < 0) {
            p = &(*p)->left;
        } else if (res > 0) {
            p = &(*p)->right;
        } else {
            return NULL;
        }
    }
    w->rbtree.parent = parent;
    *p = &w->rbtree;
    rbtree_insert(root, &w->rbtree);
    return w;
}

static void __init_cat_node(struct cat_node *node)
{
    mtree_node_init(&node->mtree);
    string_init(&node->name);
    node->level = 0;
    rbtree_root_init(&node->words);
}

static void __release_cat_node(struct cat_node *node)
{
    struct rbtree_node *p;
    struct rbtree_node *rb;
    string_release(&node->name);
    mtree_del(&node->mtree);
    for (rb = node->words.node; rb != NULL;) {
        if (rb->left != NULL) {
            rb = rb->left;
        } else if (rb->right != NULL) {
            rb = rb->right;
        } else {
            p = rb->parent;
            __release_word(get_word(rb));
            if (p == NULL) break;
            if (p->left == rb) {
                p->left = NULL;
            } else {
                p->right = NULL;
            }
            rb = p;
        }
    }
    free(node);
}

static struct cat_node *__new_cat_node()
{
    struct cat_node *node;
    if ((node = (struct cat_node *)malloc(sizeof(struct cat_node))) == NULL) return NULL;
    __init_cat_node(node);
    return node;
}

static struct word *__search_cat_word(const struct cat_node *node, const struct string *word)
{
    struct rbtree_node *n;
    struct word *w;
    int res;
    for (n = node->words.node; n != NULL;) {
        w = get_word(n);
        if ((res = string_compare(word, &w->name)) < 0) {
            n = n->left;
        } else if (res > 0) {
            n = n->right;
        } else {
            return w;
        }
    }
    return NULL;
}

void init_cat_root(struct cat_root *root)
{
    mtree_node_init(&root->root);
}

void release_cat_root(struct cat_root *root)
{
    struct mtree_node *node;
    struct mtree_node *parent;
    if (mtree_is_leaf(&root->root)) return;
    for (node = &root->root;;) {
        if (!mtree_is_leaf(node)) {
            node = mtree_first_child(node);
        } else {
            if ((parent = node->parent) == NULL) break;
            __release_cat_node(get_cat_node(node));
            if (mtree_is_leaf(parent)) {
                node = parent;
            } else {
                node = mtree_first_child(parent);
            }
        }
    }
    init_cat_root(root);
}

struct cat_node *add_cat_node(struct mtree_node *parent, struct string *name, int level)
{
    struct cat_node *node;
    if ((node = __new_cat_node()) == NULL) return NULL;
    if (string_copy(&node->name, name) == NULL) {
        __release_cat_node(node);
        return NULL;
    }
    node->level = level;
    node->mtree.parent = parent;
    mtree_add(parent, &node->mtree);
    return node;
}

struct word *add_word(struct cat_node *node, struct string *name, int *dup)
{
    struct word *p;
    *dup = 0;
    if ((p = __new_word()) == NULL) return NULL;
    if (string_copy(&p->name, name) == NULL) {
        __release_word(p);
        return NULL;
    }
    if (__insert_word(&node->words, p) == NULL) {
        *dup = 1;
        __release_word(p);
        return NULL;
    }
    return p;
}

void delete_word(struct cat_node *node, const struct string *word)
{
    struct word *w;
    if ((w = __search_cat_word(node, word)) == NULL) return;
    rbtree_delete(&node->words, &w->rbtree);
    __release_word(w);
}

struct cat_node *get_cat_from_word(struct mtree_node *root, const struct string *word)
{
    struct mtree_node *node;
    if (root == NULL) return NULL;
    for (node = mtree_pf_first(root); node != NULL; node = mtree_pf_next(root, node)) {
        if (__search_cat_word(get_cat_node(node), word) != NULL) return get_cat_node(node);
    }
    return NULL;
}

struct cat_node *get_cat_from_name(struct mtree_node *root, const struct string *name)
{
    struct mtree_node *node;
    if (root == NULL) return NULL;
    for (node = mtree_pf_first(root); node != NULL; node = mtree_pf_next(root, node)) {
        if (string_compare(&get_cat_node(node)->name, name) == 0) return get_cat_node(node);
    }
    return NULL;
}

struct cat_node *get_cat_from_cstr_name(struct mtree_node *root, const char *name)
{
    struct mtree_node *node;
    if (root == NULL) return NULL;
    for (node = mtree_pf_first(root); node != NULL; node = mtree_pf_next(root, node)) {
        if (string_strcmp(&get_cat_node(node)->name, name) == 0) return get_cat_node(node);
    }
    return NULL;
}

void clear_total(struct cat_root *cat)
{
    struct mtree_node *node = mtree_pf_first(&cat->root);
    for (node = mtree_pf_next(&cat->root, node); node != NULL; node = mtree_pf_next(&cat->root, node)) {
        get_cat_node(node)->total = 0;
        get_cat_node(node)->sub_total = 0;
    }
    cat->no_cat_in_sum = 0;
    cat->no_cat_out_sum = 0;
}

void sum_total(struct mtree_node *root)
{
    struct mtree_node *node;
    if (root == NULL) return;
    for (node = mtree_cf_first(root); node != NULL; node = mtree_cf_next(root, node)) {
        struct cat_node *cat = get_cat_node(node);
        cat->total = cat->sub_total;
        if (!mtree_is_leaf(node)) {
            struct ulist_item *item;
            for (item = node->children.first; item != NULL; item = item->next) {
                cat->total += get_cat_node(get_mtree_node(item))->total;
            }
        }
    }
}
