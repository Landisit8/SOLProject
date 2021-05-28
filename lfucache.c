#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

#include <lfucache.h>

//	creazione di un nuovo nodo
nodo* newNode (int freq, char* nome, char* testo, int stato)
{
	nodo* new = (nodo*) malloc(sizeof(nodo));	

	new->freq = freq;
	strcpy(new->nome, nome);
	strcpy(new->testo, testo);
	new->stato = stato;
	new->left = NULL;
	new->right = NULL;

	return new;
}

//	aggiungo un valore nell'albero.
nodo* addTree(nodo* n, int freq, char* nome, char* testo, int stato)
{
	if (n == NULL)
	{
		return newNode(freq,nome,testo,stato);
	}

	if (strlen(n->nome) >= strlen(nome))	n->left = addTree(n->left,freq,nome,testo,stato);
	
	else	n->right = addTree(n->right,freq,nome,testo,stato);

	return n;
}

//	Ricerco il minimo valore nell'intero albero e salvo l'informazione in min e name.
nodo* findTreeMin(nodo* n, int* min, char** name)
{	
	if (n == NULL)	return NULL;
	if (*min > (n->freq))	
	{
		*min = n->freq;
		strcpy(*name,n->nome); 
	}

	findTreeMin(n->left,min,name);
	findTreeMin(n->right,min,name);

	return n;
}


//	Ricerco il nome nell'albero
nodo* findTreeFromName(nodo* n, char* str)
{
	nodo* tmp = NULL;
	if (n == NULL)	return NULL;
	if ((strcmp(n->nome, str)) == 0)	return n;

	else 
		if (strlen(str) <= strlen(n->nome))	tmp = findTreeFromName(n->left,str);
		if (strlen(str) > strlen(n->nome))	tmp = findTreeFromName(n->right,str);
	
	return tmp;
}

//	scambio i valori di 2 nodi
int swapTree (nodo* a, nodo* b) 
{
	if (a == NULL || b == NULL)	return -1;

	nodo* tmp = newNode(a->freq, a->nome, a->testo, a->stato);

	a->freq = b->freq;
	strcpy(a->nome, b->nome);
	strcpy(a->testo, b->testo);
	a->stato = b->stato;

	b->freq = tmp->freq;
	strcpy(b->nome, tmp->nome);
	strcpy(b->testo, tmp->testo);
	b->stato = tmp->stato;

	free(tmp);
	return 0;
}	
 
//	creo un nodo temporaneo, controllo effetivamente se nel nodo parent non ci sono ulteriori foglie
//	se non ci sono foglie, scambio la foglia con il nodo temporaneo
nodo* isLeaf (nodo* parent)
{
	nodo* tmp = (nodo*) malloc(sizeof(nodo));
	if (parent->left == NULL && parent->right == NULL)
	{
		swapTree(tmp,parent);
		return tmp;
	}
	free(tmp);
	return NULL;
}

 
//	ricerca verso una qualsisasi foglia
//	da notare che se trovo la foglia, restituisco un nodo distaccato dall'albero.
nodo* searchLeaf (nodo* n)
{
	if (n == NULL)
		return NULL;

	nodo* leaf = NULL;

	if (n->left != NULL && n->right != NULL)
	{
		if ((leaf = isLeaf(n->left)) != NULL)
		{
			n->left = NULL;
			return leaf;
		}
		if ((leaf = isLeaf(n->right)) != NULL)
		{
			n->right = NULL;
			return leaf;
		}

		leaf = searchLeaf(n->left);
	}
	if (n->left == NULL && n->right != NULL)
	{
		if ((leaf = isLeaf(n->right)) != NULL)
		{
			n->right = NULL;
			return leaf;
		}
		leaf = searchLeaf(n->right);
	}

	if (n->left != NULL && n->right == NULL)
	{
		if ((leaf = isLeaf(n->left)) != NULL)
		{
			n->left = NULL;
			return leaf;
		}
		leaf = searchLeaf(n->left);
	}
	return leaf;
}

//	cerca il valore minimo nell'albero, un'altra ricerca per trovare il nome del nodo
//	cosi restituisce il nodo effettevivo, infine si cerca una foglia qualsisi e con le considerazioni della funzione searchLeaf
//	si scambia con il nodo con la frequenza minima trovata con la foglia e si cancella la foglia 
nodo* lfuRemove(nodo* n)
{
	if (n == NULL)	return NULL;

	if (n->left == NULL && n->right == NULL)	free(n);

	int min = n->freq;
	char* name = malloc(MAX_CACHE*sizeof(char));
	nodo* find = NULL;
	nodo* leaf = NULL;
	
	if (!findTreeMin(n,&min,&name))	return NULL;
	printf("Minima frequenza: %d\n", min);
	printf("nome utente: %s\n", name);

	if (!(find = findTreeFromName(n,name)))	return NULL;
	printf("convertito in nodo, nome: %s\n", find->nome);

	if (!(leaf = searchLeaf(n)))	return NULL;
	printf("foglia:%s\n", leaf->nome);

	if (swapTree(leaf, find) != 0)	return NULL;
	printf("find:%s <-> leaf:%s\n", find->nome, leaf->nome);
	free(leaf);

	return n;
}

//	se viene fatta una operazione di richiesta del cient, aumento la frequenza  
int addFrequenza(int fre)
{
	return fre++;
}

void print (nodo* n){

  if(n == NULL) return;
  printf("stampa elemento:\n");
  
  printf("Frequenza: %d\n", n->freq);
  printf("Nome: %s\n", n->nome);
  printf("Testo: %s\n", n->testo);
  printf("Stato: %d\n", n->stato);
  printf("\n");
  print(n->left);
  print(n->right);
}

int openFile(nodo* root, char* name)
{
	print(root);
	if (strcmp(name, "pRoot") == 0)	return -1;
	if (findTreeFromName(root,name) == NULL)	addTree(root, 0, name, text, 0);
	else	return -1;
	print(root);
	return 0;
}

int readFile(nodo* root, char* name)
{
	if (root->stato != 0)	return -2;
	if (findTreeFromName(root,name) == NULL)	return -3;
	else addFrequenza(root->freq);	
	return 0;
}

//	cerca il nome nel nodo poi si cerca una foglia qualsisi e con le considerazioni della funzione searchLeaf
//	si scambia con la foglia e cancello la foglia 
int fileRemove(nodo* root, char* nome)
{
	if (root == NULL)	return -1;

	if (root->stato != 0)	return -2;

	if (strcmp(name, "pRoot") == 0)	return -1;

	int min = root->freq;
	char* name = malloc(MAX_CACHE*sizeof(char));
	nodo* find = NULL;
	nodo* leaf = NULL;

	if (!findTreeMin(n,&min,&name))	return -3;
	printf("Minima frequenza: %d\n", min);
	printf("nome utente: %s\n", name);

	if (!(find = findTreeFromName(root,name)))	return -3;
	printf("convertito in nodo, nome: %s\n", find->nome);

	if (!(leaf = searchLeaf(root)))	return -3;
	printf("foglia:%s\n", leaf->nome);

	if (swapTree(leaf, find) != 0)	return -1;
	printf("find:%s <-> leaf:%s\n", find->nome, leaf->nome);
	free(leaf);

	addFrequenza(root->freq);
	return 0;
}

//	cambia lo stato del nodo
int changeStatus(nodo* root, char* name)
{
	if (strcmp(name, "pRoot") == 0)	return -1;
	if (findTreeFromName(root,name) == NULL)	return -3;
	else
		if (root->stato == 0)	root->stato = 1;
		else	if (root->stato == 1)	root->stato = 0;
	addFrequenza(root->freq);
	return 0;
}
