#ifndef _CAT_H_
#define _CAT_H_

#include "mtree.h"
#include "rbtree.h"
#include "str.h"

struct word {
    struct rbtree_node rbtree;
    struct string name;
    struct cat_node *owner;
    money_t total;
    int count;
};

struct cat_node {
    struct mtree_node mtree;
    struct string name;
    int level;
    struct rbtree_root words;
    money_t own_total;
    money_t total;
};

struct cat_root {
    struct mtree_node root;
    money_t no_cat_in_sum;
    money_t no_cat_out_sum;
};

#define get_word(ptr) rbtree_entry(ptr, struct word, rbtree)
#define get_cat_node(ptr) mtree_entry(ptr, struct cat_node, mtree)

#ifdef __cplusplus
extern "C" {
#endif

void init_cat_root(struct cat_root *root);
void release_cat_root(struct cat_root *root);

struct cat_node *add_cat_node(struct mtree_node *parent, struct string *name, int level);
BOOL cat_is_leaf(const struct cat_node *node);

struct word *add_word(struct cat_node *node, struct string *name, int *dup);
void delete_word(struct word *word);

struct word *get_word_by_name(struct mtree_node *root, const struct string *word);
struct cat_node *get_cat_from_name(struct mtree_node *root, const struct string *name);
struct cat_node *get_cat_from_cstr_name(struct mtree_node *root, const char *name);

void clear_total(struct cat_root *cat);
void sum_total(struct mtree_node *root);

#ifdef __cplusplus
}
#endif

#endif
