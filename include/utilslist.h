#if !defined(_UTIL_H)
#define _UTILSLIST_H

typedef struct node
{
    int lettera;
    char* dati;
    struct node* next;
}nodo;

typedef struct list
{
    nodo *testa;
    nodo *coda;
    size_t lung;
}lista;

static inline void msg_lStart(lista* list)
{
  list->testa = NULL;
  list->coda = NULL;
  list->lung = 0;
}

/**
 * Inserimento in testa
 */
static inline void listHead(nodo *head, list *list)
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

//  cancello la lista
static inline void listClean(list* list)
{
    msg_t* current = list->testa;
    msg_t* next;

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