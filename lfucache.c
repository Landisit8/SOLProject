#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

#define MAX 2048

typedef struct tree{
	int freq;
	char nome[2048];
	char testo[2048];
	int stato;
	struct tree *left;
	struct tree *right;
} nodo;

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

nodo* searchLeaf (nodo* n)
{
	if (n == NULL)
		return NULL;

	nodo* leaf = NULL;

	if (n->left != NULL && n->right != NULL)
	{
		if ((leaf = isLeaf(n->right)) != NULL)
		{
			n->right = NULL;
			return leaf;
		}
		if ((leaf = isLeaf(n->left)) != NULL)
		{
			n->left = NULL;
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

nodo* lfuRemove(nodo* n)
{
	if (n == NULL)	return NULL;

	if (n->left == NULL && n->right == NULL)	free(n);

	int min = n->freq;
	char* name = malloc(MAX*sizeof(char));
	nodo* find = NULL;
	nodo* leaf = NULL;
	
	CHECK_EQ_EXIT(findTreeMin(n,&min,&name),NULL,"Non esiste ancora/piu' una memoria cache");
	printf("Minima frequenza: %d\n", min);
	printf("nome utente: %s\n", name);

	CHECK_EQ_EXIT(find = findTreeFromName(n,name),NULL,"Non esiste ancora/piu' una memoria cache oppure non e' stato trovato il nome");
	printf("convertito in nodo, nome: %s\n", find->nome);

	CHECK_EQ_EXIT(leaf = searchLeaf(n),NULL,"Non esiste ancora/piu' una memoria cache");
	printf("foglia:%s\n", leaf->nome);

	swapTree(leaf, find);
	printf("find:%s <-> leaf:%s\n", find->nome, leaf->nome);
	free(leaf);

	return n;
}

nodo* fileRemove(nodo* n, char* nome)
{
	if (n == NULL)	return NULL;

	if (n->left == NULL && n->right == NULL)	free(n);

	nodo* find = NULL;
	nodo* leaf = NULL;

	CHECK_EQ_EXIT(find = findTreeFromName(n,nome),NULL,"Non esiste ancora/piu' una memoria cache");
	printf("trovato nome: %s\n", find->nome);

	CHECK_EQ_EXIT(leaf = searchLeaf(n),NULL,"Non esiste ancora/piu' una memoria cache");
	printf("foglia:%s\n", leaf->nome);

	swapTree(leaf, find);
	printf("find:%s <-> leaf:%s\n", find->nome, leaf->nome);
	free(leaf);

	return n;
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
/*	root = addTree(root, 4, "abcd", "radice", 0);
	addTree(root, 2, "ema", "stringa1", 0);
	addTree(root, 3, "amel", "stringa2", 0);
	addTree(root, 4, "lorenzo", "stringa3", 0);
	addTree(root, 5, "federico", "stringa4", 0);
	addTree(root, 6, "alessa", "stringa5", 0);
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
	printf("---------\n");
	print(root);
	lfuRemove(root);
	printf("---------\n");
	CHECK_EQ_EXIT(fileRemove(root, "lorenzo"), NULL,"Non esiste nessun albero");
	print(root);
	printf("min:\n");
	minTree(root, root->left, root->right);
	print(root);
	printf("---------\n");
	printf("swap:\n");
	swapTree(root, root->left);
	print(root);

	printf("---------\n");
	lfuRemove(root);
*/	return 0;
}