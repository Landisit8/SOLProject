#if !defined(LFUCACHE_H_)
#define LFUCACHE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

//	struttura dati per l'albero
typedef struct tree{
	int freq;				//	frequenza, il numero di volte che un file viene fatta una qualsiasi operazione
	char* nome;		
	char* testo;
	int stato;				//	stato, indica se un file e' chiuso o aperto
	struct tree *left;
	struct tree *right;
} nodo;

/** 
 * \crea un nuovo nodo   
*/
nodo* newNode(int freq, char* nome, char* testo, int stato);

/** 
 * \aggiungo un valore nell'albero.
 * \return n ok  NULL non esiste l'albero   
*/
nodo* addTree(nodo* n, int freq, char* nome, char* testo, int stato);

/** 
 * \Ricerco il minimo valore nell'intero albero e salvo l'informazione in min e name.
 * \return n ok  NULL non esiste l'albero   
*/
nodo* findTreeMin(nodo* n, int* min, char** name);

/** 
 * \Ricerco il nome nell'albero
 * \return tmp ok  NULL non esiste l'albero   
*/
nodo* findTreeFromName(nodo* n, char* str);

/** 
 * \scambio i valori di 2 nodi
*/
int swapTree (nodo* a, nodo* b);

/** 
 * \creo un nodo temporaneo, controllo effetivamente se nel nodo parent non ci sono ulteriori foglie
 * \se non ci sono foglie, scambio la foglia con il nodo temporaneo
 * \return tmp ok  NULL non esiste l'albero oppure parent non e' una foglia 
*/
nodo* isLeaf (nodo* parent);

/** 
 * \ricerca verso una qualsisasi foglia
 * \da notare che se trovo la foglia, restituisco un nodo distaccato dall'albero.
 * \return leaf ok  NULL non esiste l'albero   
*/
nodo* searchLeaf (nodo* n);

/** 
 * \cerca il valore minimo nell'albero, un'altra ricerca per trovare il nome del nodo
 * \cosi restituisce il nodo effettevivo, infine si cerca una foglia qualsisi e con le considerazioni della funzione searchLeaf
 * \si scambia con il nodo con la frequenza minima trovata con la foglia e si cancella la foglia 
 * \return n ok  NULL non esiste l'albero   
*/
int lfuRemove(nodo* n);

/** 
 * \cerca il nome nel nodo poi si cerca una foglia qualsisi e con le considerazioni della funzione searchLeaf
 * \si scambia con la foglia e cancello la foglia 
 * \return n ok  NULL non esiste l'albero   
*/
int fileRemove(nodo* root, char* nome);

/** 
 * \se viene fatta una operazione di richiesta del cient, aumento la frequenza  
 * \return fre++  
*/
int addFreqquenza(int fre);

/** 
 * \cambia lo stato del nodo
 * \return il valore cambiato dello stato //lb operazione
*/
int changeStatus(nodo* root, char* name, int lb);

/** 
 * \
 * \ 
*/
int openFile(nodo* root, char* name);

/** 
 * \
 * \ 
*/
int readFile(nodo* root, char* name);

/** 
 * \
 * \ 
*/
int appendToFile(nodo* root, char* name, char* text);

/** 
 * \
 * \ 
*/
int writeFile(nodo* root, char* name, char* text);

void print (nodo* n);

#endif // 