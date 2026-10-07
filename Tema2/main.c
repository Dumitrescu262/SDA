//Dumitrescu Andrei 313 CA
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "structs.h"
#include "create.h"
#include "functions.h"
#include "free.h"

int main(void)
{
    freopen("indexare.in", "r", stdin);
    freopen("indexare.out", "w", stdout);

    int n; scanf("%d", &n);
    list_t *file_list = create_list();
    tree_t *tree = create_tree();
    while (n--){
        char command[200];
        scanf("%s", command);
        if (strcmp(command, "ADD") == 0)
            ADD(file_list, tree);
        if (strcmp(command, "DEL") == 0)
            DEL(file_list, tree);
        if (strcmp(command, "ADDKW") == 0)
            ADDKW(tree, file_list);
        if (strcmp(command, "DELKW") == 0)
            DELKW(tree, file_list);
        if (strcmp(command, "FIND") == 0)
            FIND(tree);
        if (strcmp(command, "TOPK") == 0)
            TOPK(tree);
        if (strcmp(command, "PRINT") == 0)
            PRINT(tree);
        if (strcmp(command, "PREFIX") == 0)
            PREFIX(tree);
    }
    free_tree(tree->root);
    free(tree);
    free_list(file_list);
    return 0;
}
