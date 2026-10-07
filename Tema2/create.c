#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "structs.h"

#define DIE(assertion, call_description)                    \
    do {                                                    \
        if (assertion) {                                    \
            fprintf(stderr, "(%s, %d): ",                  \
                    __FILE__, __LINE__);                    \
            perror(call_description);                       \
            exit(errno);                                    \
        }                                                   \
    } while (0)

list_t *create_list()
{
    list_t *list = malloc(sizeof(list_t));
    DIE(list == NULL, "malloc list");

    list->head = list->tail = NULL;
    return list;
}

node_file_t *create_file(char *id, int score)
{
    node_file_t *node = malloc(sizeof(node_file_t));
    DIE(node == NULL, "malloc node_file");

    strcpy(node->id, id);
    node->score = score;
    return node;
}

tree_t *create_tree()
{
    tree_t *tree = malloc(sizeof(tree_t));
    DIE(tree == NULL, "malloc tree");

    node_tree_t *root = malloc(sizeof(node_tree_t));
    DIE(root == NULL, "malloc root");

    tree->root = root;
    root->end = 0;
    root->files = NULL;

    for (int i = 0; i < 26; i++)
        root->children[i] = NULL;

    return tree;
}