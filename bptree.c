// File: bptree.c
#include <stdlib.h>
#include <string.h>
#include "bptree.h"
#include "university_list.h"

BPTNode *create_node(int leaf) {
    BPTNode *node = malloc(sizeof(BPTNode));
    node->is_leaf = leaf;
    node->num_keys = 0;
    node->parent = NULL;
    node->next = NULL;
    for (int i = 0; i < ORDER + 1; i++) node->children[i] = NULL;
    for (int i = 0; i < ORDER; i++) { node->keys[i] = NULL; node->lists[i] = NULL; }
    return node;
}

BPTNode *find_leaf(BPTNode *root, char *dept) {
    if (!root) return NULL;
    BPTNode *current = root;
    while (!current->is_leaf) {
        int i = 0;
        while (i < current->num_keys && strcmp(dept, current->keys[i]) >= 0) i++;
        current = current->children[i];
    }
    return current;
}

void split_leaf(BPTNode **root, BPTNode *leaf, int *splits, size_t *mem) {
    int mid = ORDER / 2;
    BPTNode *new_leaf = create_node(1);
    (*splits)++;
    (*mem) += sizeof(BPTNode);
    new_leaf->num_keys = leaf->num_keys - mid;
    for (int i = 0; i < new_leaf->num_keys; i++) {
        new_leaf->keys[i] = leaf->keys[mid + i];
        new_leaf->lists[i] = leaf->lists[mid + i];
        leaf->keys[mid + i] = NULL;
        leaf->lists[mid + i] = NULL;
    }
    leaf->num_keys = mid;
    new_leaf->next = leaf->next;
    leaf->next = new_leaf;
    char *up_key = strdup(new_leaf->keys[0]);
    if (!leaf->parent) {
        BPTNode *new_root = create_node(0);
        (*mem) += sizeof(BPTNode);
        new_root->keys[0] = up_key;
        new_root->children[0] = leaf;
        new_root->children[1] = new_leaf;
        new_root->num_keys = 1;
        leaf->parent = new_root;
        new_leaf->parent = new_root;
        *root = new_root;
    } else {
        BPTNode *parent = leaf->parent;
        int i = 0;
        while (i < parent->num_keys && strcmp(up_key, parent->keys[i]) > 0) i++;
        for (int j = parent->num_keys; j > i; j--) {
            parent->keys[j] = parent->keys[j - 1];
            parent->children[j + 1] = parent->children[j];
        }
        parent->keys[i] = up_key;
        parent->children[i + 1] = new_leaf;
        parent->num_keys++;
        new_leaf->parent = parent;
        if (parent->num_keys == ORDER) split_internal(root, parent, splits, mem);
    }
}

void split_internal(BPTNode **root, BPTNode *node, int *splits, size_t *mem) {
    int mid = ORDER / 2;
    BPTNode *new_node = create_node(0);
    (*splits)++;
    (*mem) += sizeof(BPTNode);
    char *up_key = strdup(node->keys[mid]);
    for (int i = mid + 1; i < ORDER; i++) {
        new_node->keys[i - mid - 1] = node->keys[i];
        new_node->children[i - mid - 1] = node->children[i];
        if (new_node->children[i - mid - 1])
            new_node->children[i - mid - 1]->parent = new_node;
        node->keys[i] = NULL;
        node->children[i] = NULL;
    }
    new_node->children[ORDER - mid - 1] = node->children[ORDER];
    if (new_node->children[ORDER - mid - 1])
        new_node->children[ORDER - mid - 1]->parent = new_node;
    new_node->num_keys = ORDER - mid - 1;
    node->num_keys = mid;
    if (!node->parent) {
        BPTNode *new_root = create_node(0);
        (*mem) += sizeof(BPTNode);
        new_root->keys[0] = up_key;
        new_root->children[0] = node;
        new_root->children[1] = new_node;
        new_root->num_keys = 1;
        node->parent = new_root;
        new_node->parent = new_root;
        *root = new_root;
    } else {
        BPTNode *parent = node->parent;
        int i = 0;
        while (i < parent->num_keys && strcmp(up_key, parent->keys[i]) > 0) i++;
        for (int j = parent->num_keys; j > i; j--) {
            parent->keys[j] = parent->keys[j - 1];
            parent->children[j + 1] = parent->children[j];
        }
        parent->keys[i] = up_key;
        parent->children[i + 1] = new_node;
        parent->num_keys++;
        new_node->parent = parent;
        if (parent->num_keys == ORDER) split_internal(root, parent, splits, mem);
    }
}

void insert_seq(BPTNode **root, char *dept, char *uni, double score, int *splits, size_t *mem) {
    struct UnivList *node = create_list_node(uni, score);
    (*mem) += sizeof(*(node)) + strlen(uni) + 1;
    if (!*root) {
        BPTNode *leaf = create_node(1);
        (*mem) += sizeof(*leaf);
        leaf->keys[0] = strdup(dept);
        (*mem) += strlen(dept) + 1;
        leaf->lists[0] = node;
        leaf->num_keys = 1;
        *root = leaf;
        return;
    }
    BPTNode *leaf = find_leaf(*root, dept);
    int i = 0;
    while (i < leaf->num_keys && strcmp(dept, leaf->keys[i]) > 0) i++;
    if (i < leaf->num_keys && strcmp(dept, leaf->keys[i]) == 0) {
        leaf->lists[i] = insert_sorted(leaf->lists[i], uni, score);
        return;
    }
    for (int j = leaf->num_keys; j > i; j--) {
        leaf->keys[j] = leaf->keys[j - 1];
        leaf->lists[j] = leaf->lists[j - 1];
    }
    leaf->keys[i] = strdup(dept);
    (*mem) += strlen(dept) + 1;
    leaf->lists[i] = node;
    leaf->num_keys++;
    if (leaf->num_keys == ORDER) split_leaf(root, leaf, splits, mem);
}

int tree_height(BPTNode *root) {
    int height = 0;
    BPTNode *current = root;
    while (current) {
        height++;
        if (current->is_leaf) break;
        current = current->children[0];
    }
    return height;
}

char *search_bpt(BPTNode *root, char *dept, int rank, double *score) {
    BPTNode *leaf = find_leaf(root, dept);
    if (!leaf) return NULL;
    for (int i = 0; i < leaf->num_keys; i++) {
        if (strcmp(dept, leaf->keys[i]) == 0) {
            struct UnivList *u = leaf->lists[i];
            for (int r = 1; u && r < rank; r++) u = u->next;
            if (u) {
                *score = u->score;
                return u->uni;
            }
        }
    }
    return NULL;
}