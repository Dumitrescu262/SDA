# 112 Emergency Dispatch Simulator

A C simulation of an emergency-call dispatch system (inspired by Romania's 112 service), built to practice **circular doubly linked lists with a sentinel, queues and stacks**. Incidents are reported with a priority level, queued, and automatically assigned to a limited pool of intervention units (police, firefighters, ambulances).

The whole implementation lives in a single file, `tema1.c`. It reads commands from `tema1.in` and writes results to `tema1.out`.

---

## Build and run

```bash
gcc -Wall -Wextra -std=c99 -o tema1 tema1.c
./tema1          # reads ./tema1.in, writes ./tema1.out
```

Check for memory problems with Valgrind:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./tema1
```

No global or static variables are used; all state is created in `read()` and passed to the functions as parameters.

---

## Input / output format

**`tema1.in`**

```
<N_units>
<unit_id> <type>          (N_units lines, type is A / B / C)
<N_operations>
<operation>               (N_operations lines)
```

**Supported operations**

| Operation | Effect |
|---|---|
| `ADD_INCIDENT <id> <low\|medium\|high> "<description>"` | Report a new incident (status `queued`) |
| `CHECK_UNITS_AVAILABILITY` | Print the number of available units |
| `DISPATCH` | Assign the most urgent queued incident to the first available unit |
| `UNDO_LAST_DISPATCH` | Cancel the most recent unfinished intervention |
| `SOLVED_INCIDENT <id>` | Mark an ongoing incident as solved and free its unit |
| `SHOW_UNIT <id>` | Print a unit's type and availability |
| `SHOW_INCIDENT <id>` | Print an incident's priority, description and status |
| `SHOW_INTERVENTIONS` | Print every intervention in creation order |

Invalid operations print `INVALID OPERATION! ERROR 404`.

**Example**

`tema1.in`
```
2
1 A
2 B
8
ADD_INCIDENT 1 high "fire in building"
ADD_INCIDENT 2 medium "a car accident"
DISPATCH
DISPATCH
UNDO_LAST_DISPATCH
CHECK_UNITS_AVAILABILITY
SHOW_INCIDENT 2
UNDO_LAST_DISPATCH
```

`tema1.out`
```
Number of available units: 1
Incident 2 has medium priority, the following description: "a car accident" and is queued
```

---

## Data structures

### Main entities

| Struct | Fields | Notes |
|---|---|---|
| `unit_t` | `int id`, `int availability`, `char type`, `next`, `prev` | `availability`: `1` = available, `0` = unavailable |
| `incident_t` | `int id`, `char priority[7]`, `char *description`, `char status[11]`, `next`, `prev` | `description` is heap-allocated with exactly `strlen + 1` bytes |
| `intervention_t` | `incident_t *incident`, `unit_t *unit`, `next`, `prev` | Stores pointers only, never copies of units or incidents |

### Containers

| Struct | Used for | Implementation |
|---|---|---|
| `list_t` | Units, incidents, interventions | Generic header: `size`, `void *head` (sentinel), `void *tail`. Nodes form a **circular doubly linked list with a sentinel** |
| `queue_t` / `node_queue_t` | `queue_high`, `queue_medium`, `queue_low`, `queue_available_units` | Circular doubly linked list with `head`, `tail`, `size`. Each node holds a pointer to an `incident_t` **or** a `unit_t` (the unused one is `NULL`) |
| `stack_t` / `node_stack_t` | Dispatch history | Singly linked list, push/pop at the head. Each node holds a pointer to an `intervention_t` |

### How they fit together

```
list_unit          units in input order, sentinel-based, circular, doubly linked
list_incident      every incident ever reported, appended at the tail
list_intervention  every started intervention, appended at the tail

queue_available_units ──► pointers into list_unit
queue_high/medium/low ──► pointers into list_incident
stack                 ──► pointers into list_intervention
```

The queues and the stack **never copy structures**: they only store pointers to nodes that live in the lists. The queue nodes themselves are small wrappers that are allocated on enqueue and freed on dequeue.

`list_t` is deliberately generic (`void *head, *tail`). Three separate `create_list_*` functions allocate the right sentinel type, and the code casts `head`/`tail` back to the concrete node type when it manipulates them.

---

## Function overview

### Creation

- `create_list_unit`, `create_list_incident`, `create_list_intervention`: allocate a `list_t` plus a self-linked sentinel node.
- `create_queue`, `create_stack`: allocate empty containers.

### Insertion helpers

- `add_unit(list, id, type, queue)`: appends a unit (availability `1`) to the unit list and enqueues it in `queue_available_units`.
- `add_incident(list, id, priority, description, queue)`: allocates the incident, copies the description into an exactly-sized buffer, sets status `"queued"`, appends it to the incident list and enqueues it in the matching priority queue.
- `add_incident_queue`, `add_unit_queue`: enqueue at the **tail** of a queue.
- `add_incident_front_queue`: enqueue at the **head** of a queue (used by undo).

### Operations

- **`dispatch`**: takes the head of the availability queue and the head of the chosen incident queue, sets the incident to `"intervened"` and the unit to unavailable, frees both queue nodes, appends a new `intervention_t` to the intervention list and pushes it on the stack. The decision *which* queue to use is made in `read()`: the call fails with an error if no unit is available; otherwise it takes the first non-empty queue in the order `high → medium → low`, and fails with an error if all three are empty.
- **`undo_dispatch`**: pops the stack, discarding (and freeing the stack node of) every intervention whose incident is no longer `"intervened"` (i.e. already solved), until it finds an ongoing one. If the stack runs out, it prints the error. Otherwise it:
  1. sets the incident back to `"queued"` and pushes it to the **front** of its priority queue,
  2. sets the unit to available and enqueues it at the **end** of `queue_available_units`,
  3. unlinks the intervention from the intervention list in O(1) (possible thanks to the doubly linked structure) and frees it.
- **`solved`**: scans the intervention list for an intervention on the given incident ID whose status is `"intervened"`. If found, the incident becomes `"solved"`, the unit becomes available and is enqueued at the end of the availability queue. The intervention stays in the list (and on the stack). Prints the error if the ID is unknown or the incident is not currently being handled.
- **`check_units_availability`**: prints the size of `queue_available_units`.
- **`show_unit`**, **`show_incident`**: linear search through the corresponding list; print the formatted details or the error.
- **`show_interventions`**: prints every intervention in insertion order, or `No intervention has been initiated` if the list is empty.

### Input handling and cleanup

- **`read`**: opens `tema1.in` / `tema1.out`, builds the units, creates all containers, then reads the operations line by line with `fgets` and dispatches on the command name. `ADD_INCIDENT` lines are parsed with `sscanf("%s %d %s \"%[^\"]\"")` so descriptions may contain spaces.
- **`free_queue`, `free_stack`, `free_list_unit`, `free_list_incident`, `free_list_intervention`**: release every node (including each incident's `description` and each list's sentinel) and the container itself. `read()` calls all of them before returning.

---

## Complexity

| Operation | Time |
|---|---|
| `ADD_INCIDENT` | O(1) |
| `CHECK_UNITS_AVAILABILITY` | O(1) |
| `DISPATCH` | O(1) |
| `UNDO_LAST_DISPATCH` | O(k), where k is the number of already-solved interventions popped from the stack |
| `SOLVED_INCIDENT` | O(number of interventions) |
| `SHOW_UNIT` | O(number of units) |
| `SHOW_INCIDENT` | O(number of incidents) |
| `SHOW_INTERVENTIONS` | O(number of interventions) |

---

## Memory management

- Incident descriptions are allocated with exactly `strlen(description) + 1` bytes.
- Queue nodes are freed as soon as they are dequeued; stack nodes are freed when popped or when the stack is destroyed.
- Undone interventions are freed immediately when removed from the intervention list.
- At the end of `read()`, all queues, the stack and the three lists (with their sentinels) are freed, so the program is designed to exit without leaks. You can confirm this with the Valgrind command above.

---

## Limitations

- Each input line is read into a 100-character buffer, and an incident description into a 50-character buffer, so descriptions are limited to 49 characters and lines to 99.
- Priorities other than `low`, `medium` and `high` are ignored when adding an incident.
- `read()` stops processing if an `ADD_INCIDENT`, `SOLVED_INCIDENT`, `SHOW_UNIT` or `SHOW_INCIDENT` line cannot be parsed.
- Lookups by ID (`SHOW_*`, `SOLVED_INCIDENT`) are linear scans, which is fine for the problem size (at most 50 units) but would not scale to very large inputs.
