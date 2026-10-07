#include <stdlib.h>
#include <string.h>
#include "structs.h"

void free_list(list_t *list)
{
    node_t *current = list->head;
    while (current != NULL) {
        node_t *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    free(list);
}

void free_ref_list(list_t *list)
{
    node_t *current = list->head;
    while (current != NULL) {
        node_t *next = current->next;
        free(current);          
        current = next;
    }
    free(list);
}

void free_tree(node_tree_t *current)
{
    if (current == NULL)
        return;
    // elibereaza lista de fisiere
    if (current->files != NULL) {
        free_ref_list(current->files);
        current->files = NULL;
    }
    // recursiv pentru toti copiii
    for (int i = 0; i < 26; i++) {
        free_tree(current->children[i]);
        current->children[i] = NULL;
    }
    free(current);
}