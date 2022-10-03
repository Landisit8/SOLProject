#if !defined(_UTILSLIST_H)
#define _UTILSLIST_H

typedef struct node
{
    int lettera;
    struct node* next;
}nodo;

typedef struct list
{
    nodo *testa;
    nodo *coda;
    size_t lung;
}lista;

static inline void list_Start(lista* list)
{
  list->testa = NULL;
  list->coda = NULL;
  list->lung = 0;
}

/**
 * Inserimento in testa
 */
static inline void listHead(nodo *head, lista *list)
{
  // lista vuota
  if (list->lung == 0)
  {
    // la testa è l'unico elemento
    list->testa = head;
    list->coda = head;
  }
  else
  {
    // lista contiene almeno un elemento
    // allegare al nuovo nodo la vecchia lista
    head->next = list->testa;
    // ora il nuovo nodo è in testa alla lista
    list->testa = head;
  }

  // aumentare la dimensione dell'elenco
  list->lung++;
  return;
}

//  controllo se nella lista esiste l'elemento 'w' o 'W'
static inline int doubleV(lista* list)
{
  nodo* current = list->testa;
  nodo* next;

  while (current != NULL){
    if ((current->lettera == 'w')  || (current->lettera == 'W')) return 0;
    next = current->next;
    current = next;
  }
  return -1;
}

//  cancello la lista
static inline void listClean(lista* list)
{
    nodo* current = list->testa;
    nodo* next;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
    list->testa = NULL;
    list->coda = NULL;
    free(list);
    return;
}


#endif /* _UTILSLIST_H */