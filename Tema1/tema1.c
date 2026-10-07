//Dumitrescu Andrei 313 CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct unit_t unit_t;
//structura asociata unui nod de tip unitate
struct unit_t {
	int id;
	int availability;
	char type;
	unit_t *next, *prev;
};

typedef struct incident_t incident_t;
//structura asociata unui nod de tip incident
struct incident_t {
	int id;
	char priority[7];
	char *description;
	char status[11];
	incident_t *next, *prev;
};

typedef struct intervention_t intervention_t;
//structura asociata unui nod de tip interventie
struct intervention_t {
	incident_t *incident;
	unit_t *unit;
	intervention_t *next, *prev;
};

typedef struct list_t list_t;
//structura generica asociata unei liste dublu inlantuite cu santinela
struct list_t {
	int size;
	void *head, *tail;
};

typedef struct node_queue_t node_queue_t;
/*structura asociata unui nod dintr-o coada
avand in vedere ca este nevoie de cozi atat pentru stocarea de pointeri
catre unitati (availability queue), cat si catre incidente, un nod
generic dintr-o structura de coada va avea pointer catre ambele, urmand
sa fie folosit doar cel necesar*/
struct node_queue_t {
	incident_t *incident;
	unit_t *unit;
	node_queue_t *next, *prev;
};

typedef struct queue_t queue_t;
//structura asociata cozii
struct queue_t {
	int size;
	node_queue_t *head, *tail;
};

typedef struct node_stack_t node_stack_t;
//structura asociata unui nod din stiva
struct node_stack_t {
	intervention_t *intervention;
	node_stack_t *next;
};

typedef struct stack_t stack_t;
//structura asociata stivei
struct stack_t {
	int size;
	node_stack_t *head;
};

//functie ce creeaza o lista dublu inlantuita cu santinela pentru unitati
list_t *create_list_unit(void)
{
	list_t *list = malloc(sizeof(list_t));
	unit_t *sentinel = malloc(sizeof(unit_t));
	sentinel->next = sentinel;
	sentinel->prev = sentinel;
	list->head = sentinel;
	list->tail = sentinel;
	list->size = 0;
	return list;
}

//functie ce creeaza o lista dublu inlantuita cu santinela pentru incidente
list_t *create_list_incident(void)
{
	list_t *list = malloc(sizeof(list_t));
	incident_t *sentinel = malloc(sizeof(incident_t));
	sentinel->next = sentinel;
	sentinel->prev = sentinel;
	list->head = sentinel;
	list->tail = sentinel;
	list->size = 0;
	return list;
}

//functie ce creeaza o lista dublu inlantuita cu santinela pentru interventii
list_t *create_list_intervention(void)
{
	list_t *list = malloc(sizeof(list_t));
	intervention_t *sentinel = malloc(sizeof(intervention_t));
	sentinel->next = sentinel;
	sentinel->prev = sentinel;
	list->head = sentinel;
	list->tail = sentinel;
	list->size = 0;
	return list;
}

//functie ce creeaza o lista simplu inlantuita pentru stiva
stack_t *create_stack(void)
{
	stack_t *stack = malloc(sizeof(stack_t));
	stack->size = 0;
	stack->head = NULL;
	return stack;
}

//functie ce creeaza o lista dublu inlantuita pentru coada
queue_t *create_queue(void)
{
	queue_t *queue = malloc(sizeof(queue_t));
	queue->size = 0;
	queue->head = NULL;
	queue->tail = NULL;
	return queue;
}

//functie ce adauga un incident la finalul unei cozi
void add_incident_queue(queue_t *queue, incident_t *incident)
{
	if (queue->size == 0) {
		node_queue_t *node = malloc(sizeof(node_queue_t));
		node->incident = incident;
		node->unit = NULL;
		node->next = node;
		node->prev = node;
		queue->head = node;
		queue->tail = node;
		queue->size++;
		return;
	}
	node_queue_t *node = malloc(sizeof(node_queue_t));
	node->incident = incident;
	node->unit = NULL;
	queue->tail->next = node;
	node->prev = queue->tail;
	queue->tail = node;
	node->next = queue->head;
	queue->head->prev = node;
	queue->size++;
}

//functie ce adauga o unitate la finalul unei cozi
void add_unit_queue(queue_t *queue, unit_t *unit)
{
	if (queue->size == 0) {
		node_queue_t *node = malloc(sizeof(node_queue_t));
		node->unit = unit;
		node->incident = NULL;
		node->next = node;
		node->prev = node;
		queue->head = node;
		queue->tail = node;
		queue->size++;
		return;
	}
	node_queue_t *node = malloc(sizeof(node_queue_t));
	node->unit = unit;
	node->incident = NULL;
	queue->tail->next = node;
	node->prev = queue->tail;
	queue->tail = node;
	node->next = queue->head;
	queue->head->prev = node;
	queue->size++;
}

//functie ce adauga o unitate la finalul listei sale
void add_unit(list_t *list, int id, char type, queue_t *queue)
{
	unit_t *new = malloc(sizeof(unit_t));
	new->id = id;
	new->type = type;
	new->availability = 1;
	((unit_t *)list->tail)->next = new;
	new->prev = (unit_t *)list->tail;
	((unit_t *)list->head)->prev = new;
	new->next = (unit_t *)list->head;
	list->tail = new;
	list->size++;
	add_unit_queue(queue, new);
}

//functie ce adauga un incident la finalul listei sale
void add_incident(list_t *list, int id, char *priority,
				  char *description, queue_t *queue)
{
	incident_t *incident = malloc(sizeof(incident_t));
	incident->description = malloc(strlen(description) + 1);
	strcpy(incident->description, description);
	incident->id = id;
	strcpy(incident->priority, priority);
	strcpy(incident->status, "queued");
	((incident_t *)list->tail)->next = incident;
	incident->prev = (incident_t *)list->tail;
	list->tail = incident;
	incident->next = (incident_t *)list->head;
	((incident_t *)list->head)->prev = incident;
	list->size++;
	add_incident_queue(queue, incident);
}

//functie folosita pentru afisarea numarului de unitati disponibile
void check_units_availability(queue_t *queue, FILE *fout)
{
	fprintf(fout, "Number of available units: %d\n", queue->size);
}

/*functie folosita pentru adaugarea unui incident la inceputul cozii,
necesara in cazul unui undo dispatch*/
void add_incident_front_queue(queue_t *queue, incident_t *incident)
{
	node_queue_t *node_queue = malloc(sizeof(node_queue_t));
	node_queue->incident = incident;
	node_queue->unit = NULL;

	if (queue->size == 0) {
		node_queue->next = node_queue;
		node_queue->prev = node_queue;
		queue->head = node_queue;
		queue->tail = node_queue;
	} else {
		node_queue->next = queue->head;
		node_queue->prev = queue->tail;
		queue->head->prev = node_queue;
		queue->tail->next = node_queue;
		queue->head = node_queue;
	}
	queue->size++;
}

/*functie de dispatch, ce scoate unitatea valabila si incidentul din cozi
si creeaza o interventie, ce este adaugata atat in lista de interventii,
cat si in stiva*/
void dispatch(queue_t *queue_units, queue_t *queue_incidents, stack_t *stack,
			  list_t *list)
{
	unit_t *unit = queue_units->head->unit;
	incident_t *incident = queue_incidents->head->incident;
	strcpy(incident->status, "intervened");
	unit->availability = 0;

	node_queue_t *free_node_unit = queue_units->head;
	if (queue_units->size == 1) {
		queue_units->head = NULL;
		queue_units->tail = NULL;
	} else {
		queue_units->head = free_node_unit->next;
		queue_units->head->prev = queue_units->tail;
		queue_units->tail->next = queue_units->head;
	}
	queue_units->size--;
	free(free_node_unit);

	node_queue_t *free_node_incident = queue_incidents->head;
	if (queue_incidents->size == 1) {
		queue_incidents->head = NULL;
		queue_incidents->tail = NULL;
	} else {
		queue_incidents->head = free_node_incident->next;
		queue_incidents->head->prev = queue_incidents->tail;
		queue_incidents->tail->next = queue_incidents->head;
	}
	queue_incidents->size--;
	free(free_node_incident);

	intervention_t *intervention = malloc(sizeof(intervention_t));
	intervention->incident = incident;
	intervention->unit = unit;
	((intervention_t *)list->tail)->next = intervention;
	intervention->prev = (intervention_t *)list->tail;
	list->tail = intervention;
	intervention->next = (intervention_t *)list->head;
	((intervention_t *)list->head)->prev = intervention;
	list->size++;

	node_stack_t *node = malloc(sizeof(node_stack_t));
	node->intervention = intervention;
	node->next = stack->head;
	stack->head = node;
	stack->size++;
}

/*functie de undo dispatch, ce cauta in stiva o interventie careia i se poate
face undo, urmand sa trimita unitatea aferenta la finalul cozii sale, iar
incidentul la inceput; de asemenea este scoasa interventia din lista*/
void undo_dispatch(stack_t *stack, queue_t *queue_units, queue_t *queue_low,
				   queue_t *queue_medium, queue_t *queue_high,
				   list_t *list, FILE *fout)
{
	if (stack->size == 0) {
		fprintf(fout, "INVALID OPERATION! ERROR 404\n");
		return;
	}
	node_stack_t *node = stack->head;
	while ((node) &&
		   (strcmp(node->intervention->incident->status, "intervened") != 0)) {
		node_stack_t *curr = node;
		node = node->next;
		free(curr);
		stack->size--;
	}
	if (!node) {
		fprintf(fout, "INVALID OPERATION! ERROR 404\n");
		stack->size = 0;
		stack->head = NULL;
		return;
	}
	if (stack->size == 1) {
		stack->head = NULL;
	} else {
		stack->head = node->next;
	}
	stack->size--;

	incident_t *incident = node->intervention->incident;
	unit_t *unit = node->intervention->unit;
	strcpy(incident->status, "queued");
	unit->availability = 1;

	if (strcmp(incident->priority, "high") == 0)
		add_incident_front_queue(queue_high, incident);
	else if (strcmp(incident->priority, "medium") == 0)
		add_incident_front_queue(queue_medium, incident);
	else
		add_incident_front_queue(queue_low, incident);

	intervention_t *intervention = node->intervention;
	if (intervention == (intervention_t *)list->tail)
		list->tail = intervention->prev;

	node->intervention->prev->next = node->intervention->next;
	node->intervention->next->prev = node->intervention->prev;
	list->size--;
	free(intervention);
	free(node);
	add_unit_queue(queue_units, unit);
}

//functie ce marcheaza incidentul din interventie ca fiind rezolvat
void solved(list_t *list, queue_t *queue_units, int id, FILE *fout)
{
	if (list->size == 0) {
		fprintf(fout, "INVALID OPERATION! ERROR 404\n");
		return;
	}
	intervention_t *intervention = ((intervention_t *)list->head)->next;
	while (intervention != (intervention_t *)list->head) {
		if (intervention->incident->id == id &&
			strcmp(intervention->incident->status, "intervened") == 0)
			break;
		intervention = intervention->next;
	}
	if (intervention == (intervention_t *)list->head) {
		fprintf(fout, "INVALID OPERATION! ERROR 404\n");
		return;
	}
	strcpy(intervention->incident->status, "solved");
	intervention->unit->availability = 1;
	add_unit_queue(queue_units, intervention->unit);
}

//functie ce afiseaza informatii despre unitate
void show_unit(list_t *list, int id, FILE *fout)
{
	unit_t *unit = ((unit_t *)list->head)->next;
	while (unit != (unit_t *)list->head) {
		if (unit->id == id) {
			if (unit->availability == 1)
				fprintf(fout, "Unit %d is type %c and is available\n",
						id, unit->type);
			else
				fprintf(fout, "Unit %d is type %c and is unavailable\n",
						id, unit->type);
			return;
		}
		unit = unit->next;
	}
	fprintf(fout, "INVALID OPERATION! ERROR 404\n");
}

//functie ce afiseaza informatii despre incident
void show_incident(list_t *list, int id, FILE *fout)
{
	incident_t *incident = ((incident_t *)list->head)->next;
	while (incident != (incident_t *)list->head) {
		if (incident->id == id) {
			fprintf(fout, "Incident %d has %s priority, the following"
			" description: \"%s\" and is %s\n",
			id, incident->priority, incident->description,
			incident->status);
		return;
		}
		incident = incident->next;
	}
	fprintf(fout, "INVALID OPERATION! ERROR 404\n");
}

//functie ce afiseaza toate interventiile din lista
void show_interventions(list_t *list, FILE *fout)
{
	if (list->size == 0) {
		fprintf(fout, "No intervention has been initiated\n");
		return;
	}
	intervention_t *intervention = ((intervention_t *)list->head)->next;
	while (intervention != (intervention_t *)list->head) {
		fprintf(fout, "Incident %d was assigned to unit %d,"
		" and has the following status: \"%s\"\n",
		intervention->incident->id, intervention->unit->id,
		intervention->incident->status);
	intervention = intervention->next;
	}
}

//functie ce elibereaza o coada
void free_queue(queue_t *queue)
{
	if (queue->size == 0) {
		free(queue);
		return;
	}
	node_queue_t *curr = queue->head;
	node_queue_t *new;
	for (int i = 0; i < queue->size; i++) {
		new = curr->next;
		free(curr);
		curr = new;
	}
	free(queue);
}

//functie ce elibereaza stiva
void free_stack(stack_t *stack)
{
	if (stack->size == 0) {
		free(stack);
		return;
	}
	node_stack_t *curr = stack->head;
	node_stack_t *new;
	for (int i = 0; i < stack->size; i++) {
		new = curr->next;
		free(curr);
		curr = new;
	}
	free(stack);
}

//functie ce elibereaza lista de unitati
void free_list_unit(list_t *list)
{
	unit_t *curr = ((unit_t *)list->head)->next;
	unit_t *new;
	while (curr != (unit_t *)list->head) {
		new = curr->next;
		free(curr);
		curr = new;
	}
	free((unit_t *)list->head);
	free(list);
}

//functie ce elibereaza lista de incidente
void free_list_incident(list_t *list)
{
	incident_t *curr = ((incident_t *)list->head)->next;
	incident_t *new;
	while (curr != (incident_t *)list->head) {
		new = curr->next;
		free(curr->description);
		free(curr);
		curr = new;
	}
	free((incident_t *)list->head);
	free(list);
}

//functie ce elibereaza lista de interventii
void free_list_intervention(list_t *list)
{
	intervention_t *curr = ((intervention_t *)list->head)->next;
	intervention_t *new;
	while (curr != (intervention_t *)list->head) {
		new = curr->next;
		free(curr);
		curr = new;
	}
	free((intervention_t *)list->head);
	free(list);
}

/*functie ce citeste datele din fisier si apeleaza subprogramele necesare;
de asemenea, initializeaza toate structurile de care este nevoie in rezolvare*/
void read(void)
{
	FILE *fin = fopen("tema1.in", "r"); FILE *fout = fopen("tema1.out", "w");
	int n; fscanf(fin, "%d", &n); list_t *list_unit = create_list_unit();
	queue_t *queue_available_units = create_queue();
	while (n--) {
		int id; char type; fscanf(fin, "%d %c", &id, &type);
		add_unit(list_unit, id, type, queue_available_units);
	}
	fscanf(fin, "%d ", &n); queue_t *queue_medium = create_queue();
	list_t *list_incident = create_list_incident();
	queue_t *queue_low = create_queue(); queue_t *queue_high = create_queue();
	stack_t *stack = create_stack();
	list_t *list_intervention = create_list_intervention();
	while (n--) {
		char line[100]; fgets(line, 100, fin);
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		if (strncmp(line, "ADD", 3) == 0) {
			int id; char command[20], priority[10], description[50];
			if (sscanf(line, "%s %d %s \"%[^\"]\"", command, &id, priority,
					   description) != 4)
				return;
			if (strcmp(priority, "low") == 0)
				add_incident(list_incident, id, priority, description,
							 queue_low);
			else if (strcmp(priority, "medium") == 0)
				add_incident(list_incident, id, priority, description,
							 queue_medium);
			else if (strcmp(priority, "high") == 0)
				add_incident(list_incident, id, priority, description,
							 queue_high);
		}
		if (strncmp(line, "CHECK", 5) == 0)
			check_units_availability(queue_available_units, fout);
		if (strcmp(line, "DISPATCH") == 0) {
			if (queue_available_units->size == 0)
				fprintf(fout, "INVALID OPERATION! ERROR 404\n");
			else if (queue_high->size > 0)
				dispatch(queue_available_units, queue_high, stack,
						 list_intervention);
			else if (queue_medium->size > 0)
				dispatch(queue_available_units, queue_medium, stack,
						 list_intervention);
			else if (queue_low->size > 0)
				dispatch(queue_available_units, queue_low, stack,
						 list_intervention);
			else
				fprintf(fout, "INVALID OPERATION! ERROR 404\n");
		}
		if (strncmp(line, "UNDO", 4) == 0) {
			undo_dispatch(stack, queue_available_units, queue_low, queue_medium,
						  queue_high, list_intervention, fout);
		}
		if (strncmp(line, "SOLVED", 6) == 0) {
			int id; char command[20];
			if (sscanf(line, "%s %d", command, &id) != 2)
				return;
			solved(list_intervention, queue_available_units, id, fout);
		}
		if (strncmp(line, "SHOW_UNIT", 9) == 0) {
			int id; char command[20];
			if (sscanf(line, "%s %d", command, &id) != 2)
				return;
			show_unit(list_unit, id, fout);
		}
		if (strncmp(line, "SHOW_INCIDENT", 13) == 0) {
			int id; char command[20];
			if (sscanf(line, "%s %d", command, &id) != 2)
				return;
			show_incident(list_incident, id, fout);
		}
		if (strcmp(line, "SHOW_INTERVENTIONS") == 0)
			show_interventions(list_intervention, fout);
	}
	fclose(fin); fclose(fout);
	free_queue(queue_low); free_queue(queue_medium);
	free_queue(queue_high); free_queue(queue_available_units);
	free_stack(stack); free_list_intervention(list_intervention);
	free_list_unit(list_unit); free_list_incident(list_incident);
}

int main(void)
{
	read();
	return 0;
}