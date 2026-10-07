# Data Structures Assignments (C)

Two assignments from a university Data Structures course, written in C. Both implement their data structures from scratch using raw pointers and dynamic memory, with no global or static variables.

| Project | Description | Data structures |
|---|---|---|
| [`tema1/`](./tema1) | 112 emergency dispatch simulator | Circular doubly linked lists with sentinel, queues, stack |
| [`tema2/`](./tema2) | Keyword-based file indexing system | Doubly linked list, trie, max-heap |

Each folder contains its own README with implementation details.

---

## Project 1: Emergency dispatch

The program simulates a 112 call center. Incidents are reported with a priority (`low`, `medium` or `high`) and are automatically assigned to a limited number of intervention units. Higher-priority incidents are served first, units become available again once an incident is solved, and the most recent dispatch can be undone.

### Data structures

**Circular doubly linked list with sentinel.** Units, incidents and interventions are each stored in their own list of this type. The sentinel node means insertion and removal never need special cases for an empty list or for the first and last elements, and any node can be unlinked in constant time. All three lists share one generic header (`size`, `void *head`, `void *tail`), and the code casts the pointers to the right node type when it uses them.

**Queues.** There are four: one for each priority level (`queue_high`, `queue_medium`, `queue_low`) and one for the available units. They are implemented as linked lists and hold pointers to nodes of the main lists rather than copies, so a status change on an incident or unit is visible everywhere. `DISPATCH` checks the priority queues in order from high to low. Besides the usual insertion at the tail, queues also support insertion at the head, which is how an incident returns to the front of its queue when a dispatch is undone.

**Stack.** Every started intervention is pushed onto a singly linked stack that serves as the dispatch history. `UNDO_LAST_DISPATCH` pops entries until it finds an intervention that is still in progress, skipping those already solved, then restores the incident and the unit to their previous state.

### How they work together

A single incident can be referenced from the incident list, a priority queue and an intervention at the same time, and an intervention can also sit on the stack. Operations such as `DISPATCH` and `UNDO_LAST_DISPATCH` therefore update several structures together: statuses, queues, the intervention list and the stack.

---

## Project 2: File indexing

The program indexes files by keyword. Each file has an id, a relevance score and a set of keywords. It supports adding and deleting files, adding and removing keywords, finding files by keyword, ranking results, printing the index, and searching by prefix.

### Data structures

**Doubly linked list.** The master list holds every file record (id and score). The same generic list type (`void *data`) is also used for the per-keyword reference lists and for temporary result lists. Only the master list owns the records, while the others hold pointers to them, which is why the code has separate free functions for the two cases.

**Trie.** Keywords are stored one letter per node, with 26 children per node. A node marked as terminal marks the end of a keyword and holds the list of files that contain it. Lookup time depends on the length of the word, not on the number of keywords. Traversing children from `a` to `z` gives lexicographic order, which `PRINT` and `PREFIX` rely on. Removing a keyword also prunes every node that is no longer terminal and has no children.

**Sorted lists inside the trie.** Each terminal node keeps its file references ordered by id. This means `FIND`, `PRINT` and `PREFIX` produce ordered output directly, and duplicate references are detected during insertion.

**Max-heap.** `TOPK` copies the files of a keyword into an array-based heap (1-indexed) and extracts the best `k`. The comparison function orders by score first and by id second, so the heap itself needs no special handling for ties.

### How they work together

Adding or removing a keyword has to update both the trie and the file list consistently. `PREFIX` walks down the trie to the prefix node, traverses the subtree under it, and merges the referenced files into a sorted list without repetitions.

