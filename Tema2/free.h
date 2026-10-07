#ifndef FREE_H
#define FREE_H
#include "structs.h"

//functie folosita pentru lista cu fisiere
void free_list(list_t *list);

//functie folosita pentru lista cu referinte catre fisiere
void free_ref_list(list_t *list);

//functie folosita pentru arbore
void free_tree(node_tree_t *current);
#endif