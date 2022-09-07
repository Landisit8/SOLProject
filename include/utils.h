#if !defined(_UTIL_H)
#define _UTIL_H

#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>

#include <ops.h>

#define MAXS 1024
#define MAXL 80000

#if !defined(BUFSIZE)
#define BUFSIZE 256
#endif

#if !defined(EXTRA_LEN_PRINT_ERROR)
#define EXTRA_LEN_PRINT_ERROR 512
#endif

#define SYSCALL_EXIT(name, r, sc, str, ...) \
  if ((r = sc) == -1)                       \
  {                                         \
    perror(#name);                          \
    int errno_copy = errno;                 \
    print_error(str, __VA_ARGS__);          \
    exit(errno_copy);                       \
  }

#define SYSCALL_PRINT(name, r, sc, str, ...) \
  if ((r = sc) == -1)                        \
  {                                          \
    perror(#name);                           \
    int errno_copy = errno;                  \
    print_error(str, __VA_ARGS__);           \
    errno = errno_copy;                      \
  }

#define SYSCALL_RETURN(name, r, sc, str, ...) \
  if ((r = sc) == -1)                         \
  {                                           \
    perror(#name);                            \
    int errno_copy = errno;                   \
    print_error(str, __VA_ARGS__);            \
    errno = errno_copy;                       \
    return r;                                 \
  }

#define CHECK_EQ_EXIT(X, val, str) \
  if ((X) == val)                  \
  {                                \
    perror(#str);                  \
    free(X);                       \
    exit(EXIT_FAILURE);            \
  }

#define CHECK_NEQ_EXIT(X, val, str) \
  if ((X) != val)                   \
  {                                 \
    perror(#str);                   \
    free(X);                        \
    exit(EXIT_FAILURE);             \
  }

/**
 * \brief Procedura di utilita' per la stampa degli errori
 */
static inline void print_error(const char *str, ...)
{
  const char err[] = "ERROR: ";
  va_list argp;
  char *p = (char *)malloc(strlen(str) + strlen(err) + EXTRA_LEN_PRINT_ERROR);
  if (!p)
  {
    perror("malloc");
    fprintf(stderr, "FATAL ERROR nella funzione 'print_error'\n");
    return;
  }
  strcpy(p, err);
  strcpy(p + strlen(err), str);
  va_start(argp, str);
  vfprintf(stderr, p, argp);
  va_end(argp);
  free(p);
}

/**
 * \brief Controlla se la stringa passata come primo argomento e' un numero.
 * \return  0 ok  1 non e' un numbero   2 overflow/underflow
 */
static inline int isNumber(const char *s, long *n)
{
  if (s == NULL)
    return 1;
  if (strlen(s) == 0)
    return 1;
  char *e = NULL;
  errno = 0;
  long val = strtol(s, &e, 10);
  if (errno == ERANGE)
    return 2; // overflow/underflow
  if (e != NULL && *e == (char)0)
  {
    *n = val;
    return 0; // successo
  }
  return 1; // non e' un numero
}

#define LOCK(l)                              \
  if (pthread_mutex_lock(l) != 0)            \
  {                                          \
    fprintf(stderr, "ERRORE FATALE lock\n"); \
    pthread_exit((void *)EXIT_FAILURE);      \
  }
#define UNLOCK(l)                              \
  if (pthread_mutex_unlock(l) != 0)            \
  {                                            \
    fprintf(stderr, "ERRORE FATALE unlock\n"); \
    pthread_exit((void *)EXIT_FAILURE);        \
  }
#define WAIT(c, l)                           \
  if (pthread_cond_wait(c, l) != 0)          \
  {                                          \
    fprintf(stderr, "ERRORE FATALE wait\n"); \
    pthread_exit((void *)EXIT_FAILURE);      \
  }
/* ATTENZIONE: t e' un tempo assoluto! */
#define TWAIT(c, l, t)                                                \
  {                                                                   \
    int r = 0;                                                        \
    if ((r = pthread_cond_timedwait(c, l, t)) != 0 && r != ETIMEDOUT) \
    {                                                                 \
      fprintf(stderr, "ERRORE FATALE timed wait\n");                  \
      pthread_exit((void *)EXIT_FAILURE);                             \
    }                                                                 \
  }
#define SIGNAL(c)                              \
  if (pthread_cond_signal(c) != 0)             \
  {                                            \
    fprintf(stderr, "ERRORE FATALE signal\n"); \
    pthread_exit((void *)EXIT_FAILURE);        \
  }
#define BCAST(c)                                  \
  if (pthread_cond_broadcast(c) != 0)             \
  {                                               \
    fprintf(stderr, "ERRORE FATALE broadcast\n"); \
    pthread_exit((void *)EXIT_FAILURE);           \
  }
static inline int TRYLOCK(pthread_mutex_t *l)
{
  int r = 0;
  if ((r = pthread_mutex_trylock(l)) != 0 && r != EBUSY)
  {
    fprintf(stderr, "ERRORE FATALE unlock\n");
    pthread_exit((void *)EXIT_FAILURE);
  }
  return r;
}

/**
 * tipo del messaggio
 */
typedef struct msg
{
  char nome[MAXS];
  ops op;
  char str[MAXL];
  int lNome;
  int lStr;
  long fd_c;
  int flags;
  pid_t cLock;
  struct msg *next;
} msg_t;

/**
 * struct della lista di messaggi
 */
typedef struct msgList
{
  msg_t *testa;
  msg_t *coda;
  size_t lung;
} msg_l;

/**
 * setto la lista
 */
static inline void msg_lStart(msg_l *list)
{
  list->testa = NULL;
  list->coda = NULL;
  list->lung = 0;
}
/**
 * Funzione che fa la malloc e controlla
 */
static inline void *alloca(size_t size)
{
  void *alloca = malloc(size);
  CHECK_EQ_EXIT(alloca, NULL, ERROR
                : malloc);
  memset(alloca, 0, size);
  return alloca;
}
/**
 * Inserimento in testa
 */
static inline void msgHead(msg_t *head, msg_l *list)
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
/**
 * copia 2 messaggi
 */
static inline void msgcpy(msg_t *destination, msg_t *source)
{
  destination->lNome = source->lNome;
  destination->lStr = source->lStr;
  destination->op = source->op;
  destination->flags = source->flags;
  destination->cLock = source->cLock;

  memcpy(destination->nome, source->nome, strlen(source->nome));
  memcpy(destination->str, source->str, source->lStr);

  // destination->pid = source->pid;
  destination->fd_c = source->fd_c;

  destination->next = NULL;
}

static inline void msgPopReturn(msg_l *list, msg_t **toReturn)
{
  // se l'elenco è null return
  if (list->testa == NULL)
    return;

  msg_t *current = list->testa;

  // l'elenco ha un solo elemento
  if (current->next == NULL)
  {

    // copia l'elemento nel nodo di ritorno
    msgcpy(*toReturn, current);
    // libera l'unico elemento della lista
    free(current);

    // reimpostare l'elenco su null
    list->testa = NULL;
    list->coda = NULL;
    list->lung = list->lung - 1;
    return;
  }

  // scorrere l'elenco fino alla penultima
  while (current->next->next != NULL)
  {
    current = current->next;
  }

  // copiando l'ultimo elemento
  msgcpy(*toReturn, current->next);
  // liberandolo
  free(current->next);
  // aggiornando l'elenco
  current->next = NULL;
  list->coda = current;
  list->lung = list->lung - 1;

  return;
}
/**
 * Stampa della risposta dal server
 */
static inline void stampaOp(ops op)
{
  switch (op)
  {
  case OP_OK:
    printf("OPERAZIONE ESEGUITA CON SUCCESSO\n\n");
    break;
  case OP_FOK:
    printf("OPERAZIONE ESEGUITA CON NON SUCCESSO\n\n");
    break;
  case OP_BLOCK:
    printf("FILE BLOCCATO\n\n");
    break;
  case OP_FFL_SUCH:
    printf("FILE RICHIESTO NON ESISTE\n\n");
    break;
  case OP_MSG_SIZE:
    printf("MESSAGGIO TROPPO LUNGO\n\n");
    break;
  case OP_LFU:
    printf("RIMOZIONE DI UN FILE IN CASO DI ECCESSO DI DATI\n\n");
    break;
  case OP_END:
    printf("CHIUSURA' DELLA CONNESSIONE\n\n");
    break;
  default:
    fprintf(stderr,"COMMAND NOT FOUND\n\n");
    break;
  }
  fflush(stdout);
}
#endif /* _UTIL_H */
