// File: bptree.h
#ifndef BPTREE_H
#define BPTREE_H

#define ORDER 4

/* forward declaration for linked list nodes */
struct UnivList;

typedef struct BPTNode {
    int is_leaf;
    int num_keys;
    char *keys[ORDER];
    struct BPTNode *children[ORDER + 1];
    struct BPTNode *parent;
    struct BPTNode *next;
    struct UnivList *lists[ORDER];
} BPTNode;

BPTNode *create_node(int leaf);
void insert_seq(BPTNode **root, char *dept, char *uni, double score, int *splits, size_t *mem);
BPTNode *find_leaf(BPTNode *root, char *dept);
void split_leaf(BPTNode **root, BPTNode *leaf, int *splits, size_t *mem);
void split_internal(BPTNode **root, BPTNode *node, int *splits, size_t *mem);
void bulk_load(BPTNode **root, const char *filename, int *splits, size_t *mem);
char *search_bpt(BPTNode *root, char *dept, int rank, double *score);
int tree_height(BPTNode *root);

#endif  // BPTREE_H