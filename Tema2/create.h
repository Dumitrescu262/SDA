#ifndef CREATE_H
#define CREATE_H

#include "structs.h"

//functie ce creeaza o lista generica
list_t* create_list();

//functie ce creeaza un nod pentru fisier
node_file_t* create_file(char *id, int score);

//functie ce creeaza un arbore si radacina sa
tree_t* create_tree();

#endif