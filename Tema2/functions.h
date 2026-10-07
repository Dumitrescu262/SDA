#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "structs.h"
#include "create.h"

//functie ce adauga un nod la finalul unei liste
void add_to_list(void *data, list_t *list);

//adauga un cuvant in arbore si fisierul sau in lista de fisiere
//din arbore, in ordine lexicografica
void insert_word_in_tree(tree_t *tree, char* word);

//functia apelata cand se citeste cuvantul "ADD"
void ADD(list_t *file_list,tree_t *tree);

//verifica daca un nod este sau nu frunza
int is_leaf(node_tree_t *node);

//functia ce sterge recursiv un cuvant din arbore
int remove_word_recursive(node_tree_t *current, char *word, int depth);

//elimina fisierul din toate listele cuvintelor care il contineau
//daca un cuvant ramane fara fisiere se incearca stergerea sa recursiva
void delete_file_from_tree(node_file_t *file, node_tree_t *current,
                           node_tree_t *root, char *word, int depth);

//sterge fisierul din lista de fisiere
void delete_file(list_t *file_list, tree_t *tree, char *id);

//sterge un fisier
void DEL(list_t *list, tree_t *tree);

//adauga fisierul in lista cuvantului din arbore
void ADDKW(tree_t *tree, list_t *list);

//sterge fisierul asociat unui cuvant din arbore
void delete_file_from_word(tree_t *tree, node_file_t *file, char *word);

//apelata pentru stergerea scoaterea unui cuvant din fisier
void DELKW(tree_t *tree, list_t *list);

//afiseaza toate fisierele care contin cuvantul
void FIND(tree_t *tree);

//functie recursiva ce parcurge arborele lexicografic si afiseaza fisierele
void print_tree(node_tree_t *current, int depth, char *s);

//functie apelata pentru afisare
void PRINT(tree_t *tree);

//functia de comparatie necesara pentru heap
int cmp(node_file_t *file1, node_file_t *file2);

//pentru mutarea unui element la pozitia potrivita dupa inserare
void heapify_up(node_file_t **heap, int i);

//folosit dupa stergere pentru a aduce elementul pe pozitia corecta
void heapify_down(node_file_t **heap, int size, int i);

//creeaza heap-ul si extrage primele k valori
void TOPK(tree_t *tree);

//cauta fisiere in tot subarborele
void find_files_prefix(node_tree_t *current, list_t *file_list);

//parcurge arborele pana la finalul prefixului apoi cauta fisierele
void PREFIX(tree_t *tree);

#endif