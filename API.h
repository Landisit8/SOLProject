#if !defined(API_H_)
#define API_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>

#include <sys/socket.h>
#include <sys/un.h>

/**
 *	\ Viene aperta una connessione AF_UNIX al socket file "sockname" attraverso il file di config
      se il server non accetta la connessione, si cerca di ripetere la richiesta dopo "msec" fino allo scadere del tempo "abstime"
 *	\ ritorna 0 in caso di collegamento o -1 in caso di fallimento
*/
int openConnection(const char* sockname, int msec, const struct timespec abstime);

/**
 *	\ Chiude la connessione AF_UNIX associata al socket "sockname"
      qualsiasi sia il risultato di una operazione, il collegamento viene chiuso
 *	\ ritorna 0 in caso di successo della chiusura, -1 altrimenti
*/
int closeConnection(const char* sockname);

/**
 *	\ Richiesta di apertura o di creazione di un file.
      attraverso un "flags" viene deciso il tipo di operazione da effetuare che può essere
      4 tipi di operazioni descrite da un valore numero da 0-3
      La differenza è tra un tipo di creazione e un tipo di settaggio.
 *	\ Ritorna 0 in caso di successo della operazione, -1 altrimenti
*/
int openFile(const char* pathname, int flags);

/**
 *	\ Legge il contenuto di un file esistente ritornando il puntatore ad un'area allocata nel parametro "buf"
      mentre size è la dimensione del file appena letto.
 *	\ in caso di errore buf e size non saranno validi e ritorna -1. altrimenti in caso di successo 0.
*/
int readFile(const char* pathname, void** buf, size_t* size);

/**
 *	\ Richiede al server la lettura di N file da memorizzare nella directory "dirname" lato client.
      nel caso di meno di N file disponibli di inviati tutti (anche nel caso che non venga specificato)
 *	\ ritorna valore uguale o maggiore di 0 in caso di successo, -1 in caso di fallimento
*/
int readNFiles(int N, const char* dirname);

/**
 *	\
     \
*/
char* readBytes(const char* name, long* filelen);

/**
 *	\
     \
*/
int writeBytes(const char* name, char* text, long size, const char* dirname);

/**
 *	\ Scrive tutti il file da un "pathname" nel file server.
 *	\ ritorna 0 in caso di successo, -1 altrimenti
*/
int writeFile(const char* pathname, const char* dirname);

/**
 *	\ Richiesta di scrivere al file "pathname" i "size" contenuti nel buffer "buf"
 *	\ Ritorna 0 in caso di successo -1 altrimenti
*/
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);

/**
 *	\ si setta il flag "lucchetto" al file, settando in lock il file.
      Se il file era stato aperto/creato con il flag "lucchetto" e la richiesta viene dallo stesso processo.
 *	\ Ritorna 0 in caso di successo, -1 altrimenti
*/
int lockFile(const char* pathname);

/**
 *	\ si setta il flag "lucchetto" al file, settando in lock il file.
      Se il file era stato aperto/creato con il flag "lucchetto" e la richiesta viene dallo stesso processo.
 *	\ Ritorna 0 in caso di successo, -1 altrimenti
*/
int unlockFile(const char* pathname);

/**
 *	\ Richiesta di chiusura del file puntato da "pathname".
      eventuali operazioni sul file falliscono.
 *	\ Ritorna 0 in caso di successo, -1 altrimenti
*/
int closeFile(const char* pathname);

/**
 *	\ rimuove il file cancellando dalla cache del server principale.
 *	\ l'operazione falisce se il file e lock o chiuso da parte di un client diverso da chi effettua la remove
*/
int removeFile(const char* pathname);

#endif //