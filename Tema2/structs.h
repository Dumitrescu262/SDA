#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct node_t node_t;
struct node_t{
    void *data;
    node_t *next, *prev;
};

typedef struct list_t list_t;
struct list_t{
    node_t *head, *tail;
};

typedef struct tree_t tree_t;
struct tree_t{
    void *root;
};

typedef struct node_file_t node_file_t;
struct node_file_t{
    char id[200];
    int score;
};

typedef struct node_tree_t node_tree_t;
struct node_tree_t{
    int end;
    list_t *files;
    node_tree_t *children[26];
};


#endif