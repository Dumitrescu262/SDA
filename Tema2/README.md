# Simplified File Indexing System

A C implementation of a keyword-based file index. Files are stored in a doubly linked list, while their keywords are stored in a **26-way trie** (multiway retrieval tree). Each terminal trie node keeps references to the files that contain that keyword. Queries such as `TOPK` use a **max-heap**, and `PREFIX` works by traversing a subtree of the trie.

The program reads commands from `indexare.in` and writes results to `indexare.out`.

---

## Project layout

| File | Purpose |
|---|---|
| `structs.h` | All data-structure definitions |
| `create.c` / `create.h` | Constructors: `create_list`, `create_file`, `create_tree` |
| `free.c` / `free.h` | Destructors: `free_list`, `free_ref_list`, `free_tree` |
| `functions.c` / `functions.h` | Trie, list and heap logic, plus one function per command |
| `main.c` | Reads the number of commands and dispatches each one by name |

---

## Build and run

```bash
gcc -Wall -Wextra -o search_index main.c create.c free.c functions.c
./search_index        # reads ./indexare.in, writes ./indexare.out
```

Check memory usage with Valgrind:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./search_index
```

---

## Input and output

**Commands** (first line of `indexare.in` is the number of commands):

| Command | Output |
|---|---|
| `ADD id score t kw1 ... kwt` | `OK`, or `EXISTS` if the id is already present |
| `DEL id` | `OK`, or `NOT FOUND` |
| `ADDKW id kw` | `OK`, or `NOT FOUND` if the file does not exist |
| `DELKW id kw` | `OK`, or `NOT FOUND` if the file does not exist |
| `FIND kw` | `<count> <ids...>` in lexicographic order, or `EMPTY` |
| `TOPK kw k` | `<count> <ids...>` by descending score, or `EMPTY` |
| `PRINT` | One line per keyword: `<keyword> <count> <ids...>` in lexicographic order, or `EMPTY` |
| `PREFIX p` | `<count> <ids...>` of files having a keyword starting with `p`, or `EMPTY` |

---

## Data structures

### Generic doubly linked list

```c
struct node_t { void *data; node_t *next, *prev; };
struct list_t { node_t *head, *tail; };
```

A non-circular list terminated by `NULL` at both ends, with no sentinel. `data` is a `void *`, so the same list type is reused for two purposes:

1. **The master file list**, which *owns* its `node_file_t` records.
2. **The per-keyword reference lists** in the trie and the temporary lists built by `PREFIX`, which only hold *pointers* to files owned by the master list.

### File record

```c
struct node_file_t { char id[200]; int score; };
```

Files are created once in `ADD` (`create_file`) and appended to the master list in insertion order. Everywhere else the program passes around pointers to this same record, so a file is never duplicated.

### Trie

```c
struct node_tree_t {
    int end;                    // 1 if a keyword ends in this node
    list_t *files;              // references to files containing that keyword
    node_tree_t *children[26];  // one slot per letter 'a'..'z'
};
struct tree_t { void *root; };
```

- The root is an empty node created by `create_tree`; each edge corresponds to one lowercase letter (`word[i] - 'a'`).
- A node with `end == 1` is a terminal node and has a non-`NULL` `files` list. The list is created lazily when the first file is attached.
- **Each reference list is kept sorted lexicographically by file id** (sorted insertion). This means `FIND`, `PRINT` and `PREFIX` can print ids in the required order without sorting afterwards, and it also prevents duplicate references.

### Heap (for `TOPK`)

A 1-indexed array `node_file_t **heap` of pointers to files, built per query and freed at the end of it. The comparison function `cmp(a, b)` defines the priority:

1. higher `score` wins;
2. on equal scores, the lexicographically smaller `id` wins (`strcmp(b->id, a->id)`).

Because of this, the max-heap yields files in exactly the order the problem requires.

---

## How each operation works

### `ADD`
1. Scans the master list for the id. If found, it reads and discards the `t` keywords from the input (to keep the input stream aligned), prints `EXISTS` and returns.
2. Otherwise it creates the file, appends it to the master list and, for each keyword, calls `insert_word_in_tree`.
3. `insert_word_in_tree` walks down the trie, creating missing nodes, marks the last node as terminal and inserts the file reference into that node's list at its sorted position. If the file is already referenced there, nothing is added, so duplicate keywords in one `ADD` count once.

### `DEL`
1. Looks the file up in the master list (`NOT FOUND` if missing).
2. `delete_file_from_tree` performs a DFS over the whole trie, rebuilding the current word in a buffer. In every terminal node it removes the reference to the file, if present.
3. When a terminal node's list becomes empty, the list is freed, the node is un-marked (`end = 0`) and `remove_word_recursive` is called for that word from the root.
4. Finally `delete_file` unlinks the file from the master list and frees the record.

### `ADDKW`
Finds the file (`NOT FOUND` otherwise) and calls `insert_word_in_tree`. If the (file, keyword) pair already exists, the sorted insertion detects it and leaves the structure unchanged, still printing `OK`.

### `DELKW`
Finds the file (`NOT FOUND` otherwise), then `delete_file_from_word` walks the trie along the keyword. If the keyword does not exist or the file is not attached to it, nothing changes and `OK` is printed. Otherwise it removes the reference and, if the list became empty, cleans the word out of the trie the same way `DEL` does.

### Trie cleanup: `remove_word_recursive`
Removes nodes that became useless after a keyword lost its last file. It recurses down the word and, on the way back up, frees each child that is **not terminal and has no children** (`is_leaf(node) && end == 0`). It stops propagating at the first node that is still terminal for another keyword or still has a child, so the trie never keeps dead branches. The root itself is never freed.

### `FIND`
Walks the trie along the word. If the path is missing or the final node is not terminal, it prints `EMPTY`; otherwise it counts and prints the already-sorted reference list.

### `TOPK`
1. Walks the trie to the terminal node (`EMPTY` if there is none or its list is empty).
2. Copies the file pointers of that node into a heap array, calling `heapify_up` after each insertion.
3. Clamps `k` to the number of files, then performs `k` extractions: print the root, move the last element to the root, and `heapify_down`.

### `PRINT`
Prints `EMPTY` if the root has no children. Otherwise it does a DFS (`print_tree`) visiting children in the order `a`..`z`. The current word is built in a buffer, and whenever a terminal node is reached the program prints the word, the number of files and their ids. Visiting children alphabetically while appending letters yields keywords in lexicographic order, with prefixes printed before their extensions.

### `PREFIX` (bonus)
1. Walks the trie along the prefix (`EMPTY` if the path does not exist).
2. `find_files_prefix` traverses the entire subtree below that node. For each terminal node it inserts every referenced file into a temporary list, keeping it sorted by id and skipping files already present, so each file appears once even if several matching keywords point to it.
3. Prints the count and ids, then frees the temporary list with `free_ref_list` (nodes only, because the files belong to the master list).

---

## Memory management

- `free_list` (master list) frees each `node_file_t` and every list node.
- `free_ref_list` frees only list nodes, never the files, and is used for the per-keyword lists and the temporary `PREFIX` list.
- `free_tree` recursively frees every trie node and its reference list.
- The `TOPK` heap array is freed at the end of each query.
- Allocations are checked with a `DIE` macro that prints the location and `perror` message, then exits.
- At the end of `main`, the tree and then the file list are released.

---

## Complexity

Notation: `L` = word length, `F` = files in the system, `m` = files attached to one keyword, `N` = trie nodes.

| Operation | Time |
|---|---|
| `ADD` | O(F) duplicate check, plus O(L + m) per keyword |
| `DEL` | O(F + N · m): full trie traversal |
| `ADDKW` | O(F + L + m) |
| `DELKW` | O(F + L + m) |
| `FIND` | O(L + m) |
| `TOPK` | O(L + m log m) |
| `PRINT` | O(N + total references) |
| `PREFIX` | O(L + subtree size + c²), where `c` is the number of matching references |

---

## Known limitations

- Individual files do not store their own keyword list; membership is recorded only in the trie. Because of this, `DEL` has to traverse the whole trie to find all of a file's keywords.
- A file whose last keyword is removed with `DELKW` stays in the master list.
- `FIND` and `PREFIX` read their argument into 20-character buffers, and keywords are read into 200-character buffers elsewhere, so very long keywords in `FIND`/`PREFIX` queries are not supported.
- Keywords are assumed to contain only lowercase letters `a`–`z`, as stated in the assignment.
