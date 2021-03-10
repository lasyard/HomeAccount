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
    if ((p = (struct word *)malloc(sizeof(struct word))) == NULL) {
        return NULL;
    }
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
            if (p == NULL) {
                break;
            }
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
    if ((node = (struct cat_node *)malloc(sizeof(struct cat_node))) == NULL) {
        return NULL;
    }
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
    if (mtree_is_leaf(&root->root)) {
        return;
    }
    for (node = &root->root;;) {
        if (!mtree_is_leaf(node)) {
            node = mtree_first_child(node);
        } else {
            if ((parent = node->parent) == NULL) {
                break;
            }
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
    if ((node = __new_cat_node()) == NULL) {
        return NULL;
    }
    if (string_copy(&node->name, name) == NULL) {
        __release_cat_node(node);
        return NULL;
    }
    node->level = level;
    node->mtree.parent = parent;
    mtree_add(parent, &node->mtree);
    return node;
}

BOOL cat_is_leaf(const struct cat_node *node)
{
    return mtree_is_leaf(&node->mtree);
}

struct word *add_word(struct cat_node *node, struct string *name, err_code *err)
{
    struct word *p;
    if (err != NULL) {
        *err = ERR_OK;
    }
    if ((p = __new_word()) == NULL) {
        if (err != NULL) {
            *err = ERR_BAD_ALLOC;
        }
        return NULL;
    }
    if (string_copy(&p->name, name) == NULL) {
        __release_word(p);
        if (err != NULL) {
            *err = ERR_BAD_ALLOC;
        }
        return NULL;
    }
    if (__insert_word(&node->words, p) == NULL) {
        __release_word(p);
        if (err != NULL) {
            *err = ERR_CAT_DUP_WORD;
        }
        return NULL;
    }
    p->owner = node;
    return p;
}

void delete_word(struct word *word)
{
    rbtree_delete(&word->owner->words, &word->rbtree);
    __release_word(word);
}

struct word *get_word_by_name(struct mtree_node *root, const struct string *word)
{
    struct mtree_node *node;
    struct word *w;
    if (root == NULL) {
        return NULL;
    }
    for (node = mtree_pf_first(root); node != NULL; node = mtree_pf_next(root, node)) {
        if ((w = __search_cat_word(get_cat_node(node), word)) != NULL) {
            return w;
        }
    }
    return NULL;
}

struct cat_node *get_cat_from_name(struct mtree_node *root, const struct string *name)
{
    struct mtree_node *node;
    if (root == NULL) {
        return NULL;
    }
    for (node = mtree_pf_first(root); node != NULL; node = mtree_pf_next(root, node)) {
        if (string_compare(&get_cat_node(node)->name, name) == 0) {
            return get_cat_node(node);
        }
    }
    return NULL;
}

struct cat_node *get_cat_from_cstr_name(struct mtree_node *root, const char *name)
{
    struct mtree_node *node;
    if (root == NULL) {
        return NULL;
    }
    for (node = mtree_pf_first(root); node != NULL; node = mtree_pf_next(root, node)) {
        if (string_strcmp(&get_cat_node(node)->name, name) == 0) {
            return get_cat_node(node);
        }
    }
    return NULL;
}

void clear_total(struct cat_root *cat)
{
    struct mtree_node *node = mtree_pf_first(&cat->root);
    struct rbtree_node *rb;
    for (node = mtree_pf_next(&cat->root, node); node != NULL; node = mtree_pf_next(&cat->root, node)) {
        struct cat_node *cat_node = get_cat_node(node);
        for (rb = rbtree_first(&cat_node->words); rb != NULL; rb = rbtree_next(rb)) {
            get_word(rb)->total = 0;
            get_word(rb)->count = 0;
        }
    }
    cat->no_cat_in_sum = 0;
    cat->no_cat_out_sum = 0;
}

void sum_total(struct mtree_node *root)
{
    struct mtree_node *node;
    if (root == NULL) {
        return;
    }
    for (node = mtree_cf_first(root); node != NULL; node = mtree_cf_next(root, node)) {
        struct cat_node *cat_node = get_cat_node(node);
        struct rbtree_node *rb;
        cat_node->own_total = 0;
        for (rb = rbtree_first(&cat_node->words); rb != NULL; rb = rbtree_next(rb)) {
            cat_node->own_total += get_word(rb)->total;
        }
        cat_node->total = cat_node->own_total;
        if (!mtree_is_leaf(node)) {
            for (struct mtree_node *m = mtree_first_child(node); m != NULL; m = mtree_next_child(m)) {
                cat_node->total += get_cat_node(m)->total;
            }
        }
    }
}

err_code parse_cat(struct cat_root *cr, const char *line)
{
    const char *p;
    int i;
    int level;
    for (level = 0, p = line; *p == '#'; p++) {
        level++;
    }
    if (level > 3) {
        return ERR_CAT_FILE;
    }
    struct string name;
    string_mock_slice(&name, p, '\0');
    struct mtree_node *parent;
    if (level == 0) {
        err_code err;
        for (parent = &cr->root; !mtree_is_leaf(parent);) {
            parent = mtree_last_child(parent);
        }
        if (add_word(get_cat_node(parent), &name, &err) == NULL) {
            if (err != ERR_OK) {
                return err;
            }
        }
    } else {
        for (i = 3, parent = &cr->root; i > level && parent != NULL; i--) {
            parent = mtree_last_child(parent);
        }
        if (parent == NULL) {
            return ERR_CAT_FILE;
        }
        if (add_cat_node(parent, &name, level) == NULL) {
            return ERR_BAD_ALLOC;
        }
    }
    return ERR_OK;
}
