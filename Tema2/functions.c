#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "create.h"
#include "structs.h"
#include "free.h"

#define DIE(assertion, call_description)                \
    do {                                                \
        if (assertion) {                                \
            fprintf(stderr, "(%s, %d): ",              \
                    __FILE__, __LINE__);                \
            perror(call_description);                   \
            exit(errno);                                \
        }                                               \
    } while (0)

void add_to_list(void *data, list_t *list)
{
    //creeaza un nod, in care campul "data" este informatia
    node_t *node = malloc(sizeof(node_t));
    DIE(node == NULL, "malloc node");
    node->data = data;
    node->next = node->prev = NULL;
    //adauga nodul la finalul listei
    if (list->head == NULL) {
        list->head = list->tail = node;
        return;
    }
    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
}

void insert_word_in_tree(tree_t *tree, char *word, node_file_t *file)
{
    //parcurgere pana la capatul cuvantului
    node_tree_t *current = (node_tree_t *)tree->root;
    int len = strlen(word);
    for (int i = 0; i < len; i++){
        int idx = word[i] - 'a';
        if (current->children[idx] == NULL){
        //se creeaza nodurile arborelui
            node_tree_t *new = malloc(sizeof(node_tree_t));
            DIE(new == NULL, "malloc node_tree");
            new->end = 0;
            new->files = NULL;
            for (int j = 0; j < 26; j++)
                new->children[j] = NULL;
            current->children[idx] = new;
        }
        current = current->children[idx];
    }
    current->end = 1;
    //se creeaza lista pentru nodul respectiv daca nu exista deja
    if (current->files == NULL)
        current->files = create_list();

    //nodul ce va fi adaugat in lista
    node_t *new_node = malloc(sizeof(node_t));
    DIE(new_node == NULL, "malloc new_node");
    new_node->data = file;
    new_node->prev = NULL;
    new_node->next = NULL;

    if (current->files->head == NULL) {
        current->files->head = current->files->tail = new_node;
        return;
    }

    node_t *cur = current->files->head;
    while (cur != NULL) {
        node_file_t *f = (node_file_t *)cur->data;
        //fisierul exista deja, nu se adauga duplicate
        if (strcmp(file->id, f->id) == 0) {
            free(new_node);
            return;
        }
        //se cauta pozitia potrivita, lexicografic
        if (strcmp(file->id, f->id) < 0){
            new_node->next = cur;
            new_node->prev = cur->prev;
            if (cur->prev != NULL)
                cur->prev->next = new_node;
            else
                current->files->head = new_node;
            cur->prev = new_node;
            return;
        }
        cur = cur->next;
    }
    //cazul in care trebuie adaugat pe ultima pozitie
    new_node->prev = current->files->tail;
    current->files->tail->next = new_node;
    current->files->tail = new_node;
}

void ADD(list_t *list, tree_t *tree)
{
    char id[200]; int score; int nr_words;
    scanf("%s %d %d", id, &score, &nr_words);
    node_t *current = list->head;

    //verifica daca exista deja fisierul in lista
    while (current != NULL) {
        node_file_t *file = (node_file_t *)current->data;
        if (strcmp(file->id, id) == 0) {
            char garbage[200];
            for (int i = 0; i < nr_words; i++)
                scanf("%s", garbage);
            printf("EXISTS\n");
            return;
        }
        current = current->next;
    }
    //adauga fisierul in lista
    node_file_t *file = create_file(id, score);
    add_to_list((void *)file, list);

    //citirea si inserarea cuvintelor
    for (int i = 0; i < nr_words; i++){
        char word[200]; scanf("%s", word);
        insert_word_in_tree(tree, word, file);
    }
    printf("OK\n");
}

int is_leaf(node_tree_t *node)
{
    //verifica daca un nod din arbore este frunza
    for (int i = 0; i < 26; i++)
        if (node->children[i] != NULL)
            return 0;
    return 1;
}

int remove_word_recursive(node_tree_t *current, char *word, int depth)
{
    int a = strlen(word);
    //s-a ajuns la ultima litera din cuvant
    if (depth == a){
        //nodul e frunza, poate fi sters
        if (is_leaf(current))
            return 1;
        return 0;
    }
    //pentru fiecare litera se verifica daca succesoarea ei poate fi stearsa
    int idx = word[depth] - 'a';
    int delete = remove_word_recursive(current->children[idx], word, depth + 1);
    if (delete == 1){
        free(current->children[idx]);
        current->children[idx] = NULL;
    }
    //verificare daca litera curenta este frunza si nu reprezinta sfarsitul
    //altui cuvant
    return is_leaf(current) && current->end == 0;
}

void delete_file_from_tree(node_file_t *file, node_tree_t *current,
                           node_tree_t *root, char *word, int depth)
{

    if (current == NULL)
        return;

    //cuvantul are o lista de fisiere asociata
    if (current->end == 1 && current->files != NULL) {
        node_t *cur = current->files->head;
        while (cur != NULL) {
            node_file_t *f = (node_file_t *)cur->data;
            //a fost gasit fisierul ce trebuie sters
            if (strcmp(f->id, file->id) == 0) {
                if (cur->prev != NULL)
                    cur->prev->next = cur->next;
                else
                    current->files->head = cur->next;

                if (cur->next != NULL)
                    cur->next->prev = cur->prev;
                else
                    current->files->tail = cur->prev;

                free(cur);
                break;
            }
            cur = cur->next;
        }
        //daca lista a ramas goala
        if (current->files->head == NULL) {
            free(current->files);
            current->files = NULL;
            current->end = 0;
            word[depth] = '\0';
            //se incearca stergerea recursiva a cuvantului
            remove_word_recursive(root, word, 0);
        }
      
    }
    //parcurgerea cuvantului
    for (int i = 0; i < 26; i++) {
        if (current->children[i] != NULL) {
            word[depth] = 'a' + i;
            delete_file_from_tree(file, current->children[i], root, word, depth + 1);
        }
    }
}

void delete_file(list_t *file_list, char *id)
{
    node_t *current = file_list->head;
    //se cauta fisierul in lista
    while (current != NULL) {
        node_file_t *file = (node_file_t *)current->data;
        if (strcmp(file->id, id) == 0)
            break;
        current = current->next;
    }
    //nu a fost gasit
    if (current == NULL)
        return;

    //stergerea fisierului
    node_file_t *file = (node_file_t *)current->data;
    if (current->prev != NULL)
        current->prev->next = current->next;
    else
        file_list->head = current->next;

    if (current->next != NULL)
        current->next->prev = current->prev;
    else
        file_list->tail = current->prev;

    //eliberarea memoriei
    free(file);
    free(current);
}
void DEL(list_t *list, tree_t *tree)
{
    char id[200]; scanf("%s", id);
    node_t *current = list->head;
    //se cauta fisierul in lista
    while (current != NULL){
        node_file_t *file = (node_file_t *)current->data;
        if (strcmp(file->id, id) == 0)
            break;
        current = current->next;
    }
    //nu exista, operatia esueaza
    if (current == NULL){
        printf("NOT FOUND\n");
        return;
    }
    node_file_t *file = (node_file_t *)current->data;
    char word[200];
    //este sters din toate cuvintele din arbore
    delete_file_from_tree(file, tree->root, tree->root, word, 0);
    //este sters din lista
    delete_file(list, id);
    printf("OK\n");
}

void ADDKW(tree_t *tree, list_t *list)
{
    char id[200]; char word[200];
    scanf("%s %s", id, word);
    node_t *current = list->head;
    node_file_t *file = NULL;
    //se cauta fisierul in lista
    while (current != NULL) {
        node_file_t *f = (node_file_t *)current->data;
        if (strcmp(f->id, id) == 0) {
            file = f;
            break;
        }
        current = current->next;
    }
    //nu exista, operatia esueaza
    if (file == NULL) {
        printf("NOT FOUND\n");
        return;
    }
    //se insereaza cuvantul in arbore
    insert_word_in_tree(tree, word, file);
    printf("OK\n");
}

void delete_file_from_word(tree_t *tree, node_file_t *file, char *word)
{
    node_tree_t *curr = (node_tree_t *)tree->root;
    int len = strlen(word);
    //parcurgerea cuvantului
    for (int i = 0; i < len; i++) {
        int idx = word[i] - 'a';
        if (curr->children[idx] == NULL)
            return;
        curr = curr->children[idx];
    }
    //nu exista fisiere, nu are ce sa fie sters
    if (curr->end == 0 || curr->files == NULL)
        return;

    // eliminarea referintei catre fisier din nodul terminal
    node_t *cur = curr->files->head;
    while (cur != NULL) {
        node_file_t *f = (node_file_t *)cur->data;
        //a fost gasit fisierul
        if (strcmp(f->id, file->id) == 0) {
            if (cur->prev != NULL)
                cur->prev->next = cur->next;
            else
                curr->files->head = cur->next;

            if (cur->next != NULL)
                cur->next->prev = cur->prev;
            else
                curr->files->tail = cur->prev;

            free(cur);
            break;
        }
        cur = cur->next;
    }

    // daca lista a ramas goala, sterge cuvantul din arbore recursiv
    if (curr->files->head == NULL) {
        free(curr->files);
        curr->files = NULL;
        curr->end = 0;
        remove_word_recursive((node_tree_t *)tree->root, word, 0);
    }
}

void DELKW(tree_t *tree, list_t *list)
{
    char id[200]; char word[200];
    scanf("%s %s", id, word);

    // cautarea fisierului in lista
    node_t *current = list->head;
    node_file_t *file = NULL;
    while (current != NULL) {
        node_file_t *f = (node_file_t *)current->data;
        if (strcmp(f->id, id) == 0) {
            file = f;
            break;
        }
        current = current->next;
    }
    //nu exista fisierul, operatia esueaza
    if (file == NULL) {
        printf("NOT FOUND\n");
        return;
    }
    //functia de stergere a cuvantului din fisier
    delete_file_from_word(tree, file, word);
    printf("OK\n");
    
}

void FIND(tree_t *tree)
{
    char word[20];
    scanf("%s", word);

    node_tree_t *current = (node_tree_t *)tree->root;
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        int idx = word[i] - 'a';
        //daca nu exista cuvantul in arbore, niciun fisier nu-l contine
        if (current->children[idx] == NULL) {
            printf("EMPTY\n");
            return;
        }
        current = current->children[idx];
    }
    //nu reprezinta sfarsitul vreunui cuvant
    if (current->end == 0){
        printf("EMPTY\n");
        return;
    }

    //numara fisierele care il contin
    int cnt = 0;
    node_t *file = current->files->head;
    while (file != NULL) {
        cnt++;
        file = file->next;
    }

    printf("%d ", cnt);
    file = current->files->head;
    //le afiseaza
    while (file != NULL) {
        printf("%s ", ((node_file_t *)file->data)->id);
        file = file->next;
    }
    printf("\n");
}

void print_tree(node_tree_t *current, int depth, char *s)
{
    if (current == NULL)
        return;

    //a fost gasit un cuvant
    if (current->end == 1) {
        //in s este salvat cuvantul
        s[depth] = '\0'; int cnt = 0;
        node_t *file = current->files->head;
        //contorizarea fisierelor
        while (file != NULL) {
            file = file->next; cnt++;
        }
    printf("%s %d ", s, cnt);
    node_t *files = current->files->head;
    //afisarea fisierelor
    while (files != NULL){
        printf("%s ", ((node_file_t *)files->data)->id);
    files = files->next;
    } printf("\n");
    }
    
    //parcurge in ordine a tuturor copiilor
    for (int i = 0; i < 26; i++) {
        if (current->children[i] != NULL){
            //formarea cuvantului
            s[depth] = 'a' + i;
            print_tree(current->children[i], depth + 1, s);
        }
    }
}

void PRINT(tree_t *tree)
{
    char file[200];
    node_tree_t *root = (node_tree_t*)tree->root;
    // verificare daca exista elemente
    int empty = 1;
    for (int i = 0; i < 26; i++){
        if (root->children[i] != NULL)
            empty = 0;
    }
    if (empty) {
        printf("EMPTY\n");
        return;
    }
    //functia de afisare
    print_tree(root, 0, file);
}

int cmp(node_file_t *file1, node_file_t *file2)
{
    //comparatie dupa scor
    if (file1->score != file2->score)
        return file1->score - file2->score;
    //comparatie lexicografica
    return strcmp(file2->id, file1->id);
}

void heapify_up(node_file_t **heap, int i)
{
    while (i > 1) {
        int parent = i / 2;
        //nodul este mai mare decat parintele, se realizeaza interschimbarea
        if (cmp(heap[i], heap[parent]) > 0) {
            node_file_t *tmp = heap[i];
            heap[i] = heap[parent];
            heap[parent] = tmp;
            i = parent;
        }
        else break;
    }
}

void heapify_down(node_file_t **heap, int size, int i)
{
    int ok = 1;
    while (ok){
        int left = 2 * i;
        int right = 2 * i + 1;
        //tmp va fi cel mai mare nod dintre cele 3
        int tmp = i;
        //exista ambii copii
        if (left <= size && right <= size) {
            if (cmp(heap[left], heap[right]) > 0)
                tmp = left;
            else
                tmp = right;
            if (cmp(heap[tmp], heap[i]) < 0)
                tmp = i;
        //exista doar fiul stang
        } else if (left <= size) {
            if (cmp(heap[left], heap[i]) > 0)
                tmp = left;
        }
        //nodul curent este deja cel mai mare deci nu trebuie modificat heapul
        if (tmp == i)
            ok = 0;

        //altfel se face interschimbarea dintre parinte si fiul mai mare
        else{
            node_file_t *aux = heap[i];
            heap[i] = heap[tmp];
            heap[tmp] = aux;
            i = tmp;
        }
    }
}

void TOPK(tree_t *tree)
{
    char word[200]; int k;
    scanf("%s %d", word, &k);

    node_tree_t *current = (node_tree_t *)tree->root;
    int len = strlen(word);
    //parcurgerea cuvantului in arbore
    for (int i = 0; i < len; i++){
        int idx = word[i] - 'a';
        if (current->children[idx] == NULL) {
            printf("EMPTY\n");
            return;
        }
        current = current->children[idx];
    }
    //niciun fisier nu contine cuvantul
    if (current->end == 0 || current->files == NULL || current->files->head == NULL){
        printf("EMPTY\n");
        return;
    }

    int cnt = 0;
    node_t *files = current->files->head;
    //contorizarea fisierelor
    while (files != NULL){
        cnt++; files = files->next;
    }

    //indexarea este de la 1, se aloca un spatiu in plus pentru heap
    node_file_t **heap = malloc((cnt + 1) * sizeof(node_file_t *));
    DIE(heap == NULL, "malloc heap");
    files = current->files->head;
    //introduc elementele in heap
    for (int i = 1; i <= cnt; i++) {
        heap[i] = (node_file_t *)files->data;
        heapify_up(heap, i);
        files = files->next;
    }

    //nu exista k fisiere asa ca vor fi afisate toate
    if (k > cnt)
        k = cnt;
    printf("%d ", k);
    //afisez elementul prioritar si il extrag din heap, de k ori
    for (int i = 0; i < k; i++) {
        printf("%s ", heap[1]->id);
        heap[1] = heap[cnt - i];
        int size = cnt - i - 1;
        heapify_down(heap, size, 1);
    }
    printf("\n");
    free(heap);

}

void find_files_prefix(node_tree_t *current, list_t *file_list)
{
    if (current == NULL)
        return;

    if (current->end != 0 && current->files != NULL) {
        node_t *tmp = current->files->head;
        while (tmp) {
            node_file_t *f = (node_file_t *)tmp->data;
            //lista pentru prefix este goala deci nodul va fi adaugat direct
            if (file_list->head == NULL){
                node_t *new_node = malloc(sizeof(node_t));
                DIE(new_node == NULL, "malloc new_node prefix");
                new_node->data = f;
                new_node->prev = NULL;
                new_node->next = NULL;
                file_list->head = file_list->tail = new_node;
            } else{
                node_t *cmp = file_list->head;
                int ok = 0;
                while (cmp) {
                    node_file_t *id = (node_file_t *)cmp->data;
                    //fisierul exista deja, nu va mai fi adaugat
                    if (strcmp(f->id, id->id) == 0) {
                        ok = 1;
                        break;
                    }
                    //am gasit locul fisierului, lexicografic
                    if (strcmp(f->id, id->id) < 0) {
                        node_t *new_node = malloc(sizeof(node_t));
                        DIE(new_node == NULL, "malloc new_node prefix insert");
                        new_node->data = f;
                        new_node->next = cmp;
                        new_node->prev = cmp->prev;
                        if (cmp->prev != NULL)
                            cmp->prev->next = new_node;
                        else
                            file_list->head = new_node;
                        cmp->prev = new_node;
                        ok = 1;
                        break;
                    }
                    cmp = cmp->next;
                }
                //fisierul trebuie adaugat la final
                if (ok == 0) {
                    node_t *new_node = malloc(sizeof(node_t));
                    DIE(new_node == NULL, "malloc new_node prefix tail");
                    new_node->data = f;
                    new_node->prev = file_list->tail;
                    new_node->next = NULL;
                    file_list->tail->next = new_node;
                    file_list->tail = new_node;
                }
            }
            tmp = tmp->next;
        }
    }
    //parcurgere in toate directiile, in ordine
    for (int i = 0; i < 26; i++){
        find_files_prefix(current->children[i], file_list);
    }
}

void PREFIX(tree_t *tree)
{
    char prefix[20];
    scanf("%s ", prefix);
    list_t *file_list = create_list();
    node_tree_t *node = tree->root;
    //parcurgerea prefixului in arbore
    for (int i = 0; i < (int)strlen(prefix); i++){
        int idx = prefix[i] - 'a';
        if (node->children[idx] == NULL) {
            free(file_list);
            printf("EMPTY\n");
            return;
        }
        node = node->children[idx];
    }
    //cauta toate fisierele incepand cu un anumit prefix
    find_files_prefix(node, file_list);
    int cnt = 0;
    node_t *cur = file_list->head;
    //contorizarea numarului de fisiere
    while (cur != NULL) {
        cnt++;
        cur = cur->next;
    }
    if (cnt == 0){
        printf("EMPTY\n");
        return;
    }

    printf("%d ", cnt);
    cur = file_list->head;
    //afisarea lor
    while (cur != NULL) {
        printf("%s ", ((node_file_t *)cur->data)->id);
        cur = cur->next;
    }
    printf("\n");
    free_ref_list(file_list);
}