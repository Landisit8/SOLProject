#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX 2048

typedef struct tree{
	int freq;
	char nome[2048];
	char testo[2048];
	int stato;
	struct tree *left;
	struct tree *right;
} nodo;

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

nodo* findTreeMin(nodo* n, int* min, char** name)
{
	nodo* tmp = NULL;
	if (n == NULL)	return NULL;

	if (*min > (n->freq))	
	{
		*min = n->freq;
		strcpy(*name,n->nome);
	}

	tmp = findTreeMin(n->left,min,name);
	tmp = findTreeMin(n->right,min,name);

	return tmp;
}

nodo* searchLeaf (nodo* n)
{
	unsigned short a;

	nodo* tmp = NULL;
	if (n->left == NULL)	a++;
	else	tmp = searchLeaf(n->left);
	if (n->right == NULL)	a++;
	else	tmp = searchLeaf(n->right);
	printf("pre:%d\n", a);
	if (a == 2)
		return n;
	printf("post:%d\n", a);
	return tmp;
}

void swapTree (nodo* a, nodo* b) 
{
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
}

//	sostitusco con una foglia
//	elimino
nodo* minTree(nodo* n)
{
	if (n == NULL)	return NULL;
	int min = n->freq;
	char* name = malloc(MAX*sizeof(char));
	nodo* find = NULL;
	nodo* leaf = NULL;
	
	findTreeMin(n,&min,&name);
	printf("Minima frequenza: %d\n", min);
	printf("nome utente: %s\n", name);
	find = findTreeFromName(n,name);
	printf("convertito in nodo, nome: %s\n", find->nome);
	leaf = searchLeaf(n);
	printf("foglia:%s\n", leaf->nome);
	swapTree(leaf, find);

	printf("find:%s <-> leaf:%s\n", find->nome, leaf->nome);
	//cancellazione

	return NULL;
}

int addFreqquenza(int fre)
{
	return fre++;
}

int changeStatus(int stato)
{
	if (stato == 0)	stato = 1;
	else	if (stato == 1)	stato = 0;
	return stato;
}


int main() {
	nodo* root = NULL;
	root = addTree(root, 4, "abcd", "radice", 0);
	addTree(root, 2, "ema", "stringa1", 0);
	addTree(root, 3, "amel", "stringa2", 0);
	addTree(root, 4, "lorenzo", "stringa3", 0);
	addTree(root, 5, "federico", "stringa4", 0);
	addTree(root, 6, "alessa", "stringa5", 0);

/*	ok	
	printf("%s\n", root->nome);
	printf("cambio verso, sinistra:\n");
	printf("foglia della radice: %s\n", root->left->nome);
	printf("nipote della radice: %s\n", root->left->right->nome);
	printf("cambio verso, destra:\n");
	printf("%s\n", root->right->nome);
	printf("%s\n", root->right->left->nome);
	printf("%s\n", root->right->right->nome);
	printf("---------\n");
	print(root);
	
	printf("Ricerca:\n");
	nodo* find = NULL;
	printf("%s\n", root->left->right->nome);
	printf("%s\n", root->left->right->testo);
	find = findTreeFromName(root, "amel");
	printf("Trovato '%s': %d\n", find->nome, find->freq);
	*/
	printf("---------\n");
	minTree(root);
/*
	printf("min:\n");
	minTree(root, root->left, root->right);
	print(root);

	printf("---------\n");

	printf("swap:\n");
	swapTree(root, root->left);
	print(root);
*/
	return 0;
}