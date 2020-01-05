#include "rbtree.h"

static void __rotate_left(struct rbtree_root *root, struct rbtree_node *node)
{
    struct rbtree_node *right = node->right;
    struct rbtree_node *parent = node->parent;
    node->right = right->left;
    if (right->left != NULL) right->left->parent = node;
    right->left = node;
    right->parent = parent;
    if (parent != NULL) {
        if (node == parent->left) {
            parent->left = right;
        } else {
            parent->right = right;
        }
    } else {
        root->node = right;
    }
    node->parent = right;
}

static void __rotate_right(struct rbtree_root *root, struct rbtree_node *node)
{
    struct rbtree_node *left = node->left;
    struct rbtree_node *parent = node->parent;
    node->left = left->right;
    if (left->right != NULL) left->right->parent = node;
    left->right = node;
    left->parent = parent;
    if (parent != NULL) {
        if (node == parent->right) {
            parent->right = left;
        } else {
            parent->left = left;
        }
    } else {
        root->node = left;
    }
    node->parent = left;
}

static void __rb_delete_color(struct rbtree_root *root, struct rbtree_node *node, struct rbtree_node *parent)
{
    struct rbtree_node *other;
    while ((node == NULL || node->color == RBTREE_BLACK) && node != root->node) {
        if (parent->left == node) {
            other = parent->right;
            if (other->color == RBTREE_RED) {
                other->color = RBTREE_BLACK;
                parent->color = RBTREE_RED;
                __rotate_left(root, parent);
                other = parent->right;
            }
            if ((other->left == NULL || other->left->color == RBTREE_BLACK) &&
                (other->right == NULL || other->right->color == RBTREE_BLACK)) {
                other->color = RBTREE_RED;
                node = parent;
                parent = node->parent;
            } else {
                if (other->right == NULL || other->right->color == RBTREE_BLACK) {
                    other->left->color = RBTREE_BLACK;
                    other->color = RBTREE_RED;
                    __rotate_right(root, other);
                    other = parent->right;
                }
                other->color = parent->color;
                parent->color = RBTREE_BLACK;
                other->right->color = RBTREE_BLACK;
                __rotate_left(root, parent);
                node = root->node;
                break;
            }
        } else {
            other = parent->left;
            if (other->color == RBTREE_RED) {
                other->color = RBTREE_BLACK;
                parent->color = RBTREE_RED;
                __rotate_right(root, parent);
                other = parent->left;
            }
            if ((other->left == NULL || other->left->color == RBTREE_BLACK) &&
                (other->right == NULL || other->right->color == RBTREE_BLACK)) {
                other->color = RBTREE_RED;
                node = parent;
                parent = node->parent;
            } else {
                if (other->left == NULL || other->left->color == RBTREE_BLACK) {
                    other->right->color = RBTREE_BLACK;
                    other->color = RBTREE_RED;
                    __rotate_left(root, other);
                    other = parent->left;
                }
                other->color = parent->color;
                parent->color = RBTREE_BLACK;
                other->right->color = RBTREE_BLACK;
                __rotate_right(root, parent);
                node = root->node;
                break;
            }
        }
    }
    if (node != NULL) node->color = RBTREE_BLACK;
}

void rbtree_root_init(struct rbtree_root *root)
{
    root->node = NULL;
}

void rbtree_node_init(struct rbtree_node *node)
{
    node->parent = node;
    node->left = node->right = NULL;
    node->color = RBTREE_RED;
}

void rbtree_insert(struct rbtree_root *root, struct rbtree_node *node)
{
    struct rbtree_node *parent, *gparent;
    while ((parent = node->parent) != NULL && parent->color == RBTREE_RED) {
        gparent = parent->parent;
        if (parent == gparent->left) {
            struct rbtree_node *uncle = gparent->right;
            if (uncle != NULL && uncle->color == RBTREE_RED) {
                uncle->color = RBTREE_BLACK;
                parent->color = RBTREE_BLACK;
                gparent->color = RBTREE_RED;
                node = gparent;
            } else {
                if (parent->right == node) {
                    struct rbtree_node *tmp;
                    __rotate_left(root, parent);
                    tmp = parent;
                    parent = node;
                    node = tmp;
                }
                parent->color = RBTREE_BLACK;
                gparent->color = RBTREE_RED;
                __rotate_right(root, gparent);
            }
        } else {
            struct rbtree_node *uncle = gparent->left;
            if (uncle != NULL && uncle->color == RBTREE_RED) {
                uncle->color = RBTREE_BLACK;
                parent->color = RBTREE_BLACK;
                gparent->color = RBTREE_RED;
                node = gparent;
            } else {
                if (parent->left == node) {
                    struct rbtree_node *tmp;
                    __rotate_right(root, parent);
                    tmp = parent;
                    parent = node;
                    node = tmp;
                }
                parent->color = RBTREE_BLACK;
                gparent->color = RBTREE_RED;
                __rotate_left(root, gparent);
            }
        }
    }
    root->node->color = RBTREE_BLACK;
}

void rbtree_delete(struct rbtree_root *root, struct rbtree_node *node)
{
    struct rbtree_node *child, *parent;
    int color;
    if (node->left == NULL) {
        child = node->right;
    } else if (node->right == NULL) {
        child = node->left;
    } else {
        struct rbtree_node *sub, *left;
        sub = node->right;
        while ((left = sub->left) != NULL) sub = left;
        if (node->parent != NULL) {
            if (node->parent->left == node) {
                node->parent->left = sub;
            } else {
                node->parent->right = sub;
            }
        } else {
            root->node = sub;
        }
        child = sub->right;
        parent = sub->parent;
        color = sub->color;
        if (parent == node) {
            parent = sub;
        } else {
            if (child != NULL) child->parent = parent;
            parent->left = child;
            sub->right = node->right;
            node->right->parent = sub;
        }
        sub->parent = node->parent;
        sub->color = node->color;
        sub->left = node->left;
        node->left->parent = sub;
        if (color == RBTREE_BLACK) __rb_delete_color(root, child, parent);
        return;
    }
    parent = node->parent;
    color = node->color;
    if (child != NULL) child->parent = parent;
    if (parent != NULL) {
        if (parent->left == node) {
            parent->left = child;
        } else {
            parent->right = child;
        }
    } else {
        root->node = child;
    }
    if (color == RBTREE_BLACK) __rb_delete_color(root, child, parent);
}

struct rbtree_node *rbtree_first(const struct rbtree_root *root)
{
    struct rbtree_node *node = root->node;
    if (node == NULL) return NULL;
    while (node->left != NULL) node = node->left;
    return node;
}

struct rbtree_node *rbtree_last(const struct rbtree_root *root)
{
    struct rbtree_node *node = root->node;
    if (node == NULL) return NULL;
    while (node->right != NULL) node = node->right;
    return node;
}

struct rbtree_node *rbtree_next(const struct rbtree_node *node)
{
    struct rbtree_node *parent;
    if (node->parent == node) return NULL;
    if (node->right != NULL) {
        node = node->right;
        while (node->left != NULL) node = node->left;
        return (struct rbtree_node *)node;
    }
    while ((parent = node->parent) != NULL && node == parent->right) node = parent;
    return parent;
}

struct rbtree_node *rbtree_prev(const struct rbtree_node *node)
{
    struct rbtree_node *parent;
    if (node->parent == node) return NULL;
    if (node->left != NULL) {
        node = node->left;
        while (node->right != NULL) node = node->right;
        return (struct rbtree_node *)node;
    }
    while ((parent = node->parent) != NULL && node == parent->left) node = parent;
    return parent;
}
