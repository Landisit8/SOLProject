#include <lfucache.h>

int ordine = 0;				//	ordine di ingresso nell'albero
pthread_mutex_t setOrdine;

extern long memMax;
extern long numMax;
extern pthread_mutex_t setMemMax;
extern pthread_mutex_t setNumMax;

//	creazione di un nuovo nodo
nodo* newNode (int freq, char* nome, char* testo, int stato, int lock)
{
	nodo* new;
	new = alloca(sizeof(nodo));
	new->freq = freq;
	new->ord = ordine;
	new->nome = alloca(strlen(nome) + 1);
	strncpy(new->nome, nome, (strlen(nome)) + 1);
	new->testo = alloca(strlen(testo) + 1);
	strncpy(new->testo, testo, (strlen(testo)) + 1);
	new->stato = stato;
	new->lucchetto = lock;
	new->left = NULL;
	new->right = NULL;

	return new;
}

//	aggiungo un valore nell'albero.
nodo* addTree(nodo* n, int freq, char* nome, char* testo, int stato, int lock)
{
	if (n == NULL)	return newNode(freq,nome,testo,stato,lock);
	if (strlen(n->nome) >= strlen(nome))	n->left = addTree(n->left,freq,nome,testo,stato,lock);
	else	n->right = addTree(n->right,freq,nome,testo,stato,lock);
	return n;
}

//	Ricerco il minimo valore nell'intero albero e salvo l'informazione in min e name.
nodo* findTreeMin(nodo* n, int* min, char** name)
{	
	if (n == NULL)	return NULL;
	if (strcmp(n->nome, "pRoot") != 0) {
		if (*min > (n->freq))	
		{
			*min = n->freq;
			strncpy(*name,n->nome, strlen(n->nome) + 1); 
		}
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

	else {
		if (strlen(str) <= strlen(n->nome))	tmp = findTreeFromName(n->left,str);
		if (strlen(str) > strlen(n->nome))	tmp = findTreeFromName(n->right,str);
	}
	return tmp;
}

//	scambio i valori di 2 nodi
int swapTree (nodo* a, nodo* b) 
{
	if (a == NULL || b == NULL)	return -1;
	nodo* tmp = newNode(a->freq, a->nome, a->testo, a->stato, a->lucchetto);

	a->freq = b->freq;
	free(a->nome);
	a->nome = alloca(strlen(b->nome) + 1);
	strncpy(a->nome, b->nome, strlen(b->nome) + 1);
	free(a->testo);
	a->testo = alloca(strlen(b->testo) + 1);
	strncpy(a->testo, b->testo, strlen(b->testo) + 1);
	a->stato = b->stato;

	b->freq = tmp->freq;
	free(b->nome);
	b->nome = alloca(strlen(tmp->nome) + 1);
	strncpy(b->nome, tmp->nome, strlen(tmp->nome));
	free(b->testo);
	b->testo = alloca(strlen(tmp->testo) + 1);
	strncpy(b->testo, tmp->testo, strlen(tmp->testo) + 1);
	b->stato = tmp->stato;

	free(tmp->nome);
	free(tmp->testo);
	free(tmp);
	return 0;
}	
 
//	creo un nodo temporaneo, controllo effetivamente se nel nodo parent non ci sono ulteriori foglie
//	se non ci sono foglie, scambio la foglia con il nodo temporaneo
nodo* isLeaf (nodo* parent)
{
	if (parent->left == NULL && parent->right == NULL)
	{
		nodo* tmp = newNode(parent->freq,parent->nome,parent->testo,parent->stato, parent->lucchetto);
		return tmp;
	}
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
			free(n->left->nome);
			free(n->left->testo);
			free(n->left);
			n->left = NULL;
			return leaf;
		}
		if ((leaf = isLeaf(n->right)) != NULL)
		{
			free(n->right->nome);
			free(n->right->testo);
			free(n->right);
			n->right = NULL;
			return leaf;
		}
		leaf = searchLeaf(n->left);
	}
	if (n->left == NULL && n->right != NULL)
	{
		if ((leaf = isLeaf(n->right)) != NULL)
		{
			free(n->right->nome);
			free(n->right->testo);
			free(n->right);
			n->right = NULL;
			return leaf;
		}
		leaf = searchLeaf(n->right);
	}
	if (n->left != NULL && n->right == NULL)
	{
		if ((leaf = isLeaf(n->left)) != NULL)
		{
			free(n->left->nome);
			free(n->left->testo);
			free(n->left);
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
int lfuRemove(nodo* n, msg_t** text)
{
	if (n == NULL)	return -3;
	if (n->left == NULL && n->right == NULL)	return -1;
	int min;
	char* name;

	//	Prendo la prima informazione disponibile
	if (n->left == NULL){	min = n->right->freq;	name = n->right->nome; }
	else { min = n->left->freq;	name = n->left->nome; }
	
	nodo* find = NULL;
	nodo* leaf = NULL;
	
	//	cerco il nodo minimo e trovo il nome
	if (!findTreeMin(n,&min,&name))	return -3;

	// cerco il nodo dal nome del nodo minimo e ottengo un riferimento
	if (!(find = findTreeFromName(n,name)))	return -3;

	//carico gli elementi in "text" per caricarlo nella cartella apposita
	*text = alloca(sizeof(msg_t));
	strncpy((*text)->str,find->testo, strlen(find->testo));
	(*text)->lStr = strlen(find->testo);
	strncpy((*text)->nome,find->nome, strlen(find->nome));
	(*text)->lNome = strlen(find->nome);

	//	cerco una foglia generica
	if (!(leaf = searchLeaf(n)))	return -3;

	if (swapTree(leaf, find) != 0)	return -1;
	free(leaf->nome);
	free(leaf->testo);
	free(leaf);
	return 0;
}

nodo* findTreeOrd(nodo* root, int* min, char** name){
	if (root == NULL)	return NULL;
	if (strcmp(root->nome, "pRoot") != 0) {
		if (*min > (root->ord))	
		{
			*min = root->ord;
			strncpy(*name,root->nome, strlen(root->nome) + 1); 
		}
	}
	findTreeOrd(root->left,min,name);
	findTreeOrd(root->right,min,name);

	return root;
}

int fifoRemove(nodo* root, msg_t** text)
{
	if (root == NULL)	return -3;
	if (root->left == NULL && root->right == NULL)	return -1;
	int min;
	char* name;

	if (root->left == NULL){	min = root->right->ord;	name = root->right->nome; }
	else { min = root->left->ord;	name = root->left->nome; }
	
	nodo* find = NULL;
	nodo* leaf = NULL;
	
	//	cerco il nodo con l'ordine minimo e trovo il nome
	if (!findTreeOrd(root,&min,&name))	return -3;

	// cerco il nodo dal nome del nodo minimo e ottengo un riferimento
	if (!(find = findTreeFromName(root,name)))	return -3;

	//carico gli elementi in "text" per caricarlo nella cartella apposita
	*text = alloca(sizeof(msg_t));
	strncpy((*text)->str,find->testo, strlen(find->testo));
	(*text)->lStr = strlen(find->testo);
	strncpy((*text)->nome,find->nome, strlen(find->nome));
	(*text)->lNome = strlen(find->nome);

	//	cerco una foglia generica
	if (!(leaf = searchLeaf(root)))	return -3;

	if (swapTree(leaf, find) != 0)	return -1;
	free(leaf->nome);
	free(leaf->testo);
	free(leaf);
	return 0;
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

int openFile(nodo* root, char* name, int flags, pid_t cLock)
{
	LOCK(&setNumMax);
	if (numMax == 0){	UNLOCK(&setNumMax); return -5;}
	UNLOCK(&setNumMax);
	if (strcmp(name, "pRoot") == 0)	return -1;
	switch(flags){
		case 0:
			if (findTreeFromName(root,name) == NULL){
					root = addTree(root, 0, name, "", 0, 1);
					LOCK(&setNumMax);
					numMax--;
					UNLOCK(&setNumMax);
					LOCK(&setOrdine);
					ordine++;
					UNLOCK(&setOrdine);
					}
			else return -1;
		break;
		case 1:
			changeLock(root, name, 1, cLock);
			changeStatus(root, name, 0, cLock);
		break;
		case 2:
			changeStatus(root, name, 0,cLock);
		break;
		case 3:
			if (findTreeFromName(root,name) == NULL){
				root = addTree(root, 0, name, "", 0, 0);
				changeLock(root, name, 1, cLock);
				changeStatus(root, name, 0, cLock);
				LOCK(&setNumMax);
				numMax--;
				UNLOCK(&setNumMax);
				LOCK(&setOrdine);
				ordine++;
				root->ord = ordine;
				UNLOCK(&setOrdine);
				}
			else return -1;
		break;
		default:
			return -1;
		break;
	}
	return 0;
}

//	leggo un file dall'albero
int readFile(nodo* root, char* name, msg_t** text, pid_t cLock)
{
	nodo* tmp;
	*text = alloca(sizeof(msg_t));
	if (strcmp(name, "pRoot") == 0)	return -1;
	if ((tmp = findTreeFromName(root,name)) == NULL)	return -3;
	else if (tmp->stato != 0 || (tmp->lucchetto == 0 && tmp->sLock != cLock))	return -2;
	else addFrequenza(tmp->freq);
	strncpy((*text)->str,tmp->testo, (strlen(tmp->testo)) + 1);
	(*text)->lStr= strlen(tmp->testo);
	strncpy((*text)->nome,tmp->nome, (strlen(tmp->nome)) + 1);
	(*text)->lNome = strlen(tmp->nome);
	return 0;
}


//	leggo n file dall'albero
void readsFile(nodo* root, int n, pid_t cLock, msg_l* buffer)
{
	if (root == NULL)	return;
	if(strcmp((root->nome), "pRoot")){
	if (root->stato != 1){
	if ((root->lucchetto != 1 && root->sLock != cLock) || root->lucchetto != 0){
		if (n >= 0){
			msg_t* tmp = alloca(sizeof(msg_t));
			strncpy(tmp->str, root->testo, strlen(root->testo));
			tmp->lStr = strlen(root->testo);
			strncpy(tmp->nome, root->nome, strlen(root->nome));
			tmp->lNome = strlen(root->nome);
			msgHead(tmp, buffer);
			n--;
		} else if (n == -1){
			msg_t* tmp = alloca(sizeof(msg_t));
			strncpy(tmp->str, root->testo, strlen(root->testo));
			tmp->lStr = strlen(root->testo);
			strncpy(tmp->nome, root->nome, strlen(root->nome));
			tmp->lNome = strlen(root->nome);
			msgHead(tmp, buffer);
		}
	}
	}
	}
	readsFile(root->left, n, cLock, buffer);
	readsFile(root->right, n, cLock, buffer);

	return;
}

//	scrivo sull'albero
int appendToFile(nodo* root, char* name, char* text, pid_t cLock)
{
	nodo* find = NULL;
	if ((find = findTreeFromName(root,name)) == NULL)	return -3;
	else if (find->stato != 0 || (find->lucchetto == 0 && find->sLock != cLock))	return -2;
	else
	{
		LOCK(&setMemMax);
		memMax = memMax - strlen(find->testo);
		UNLOCK(&setMemMax);
		if (memMax <= 0)	{fprintf(stderr,"sto cancellando....\n"); return -5;}
		int somma = strlen(find->testo) + strlen(text);
		CHECK_EQ_EXIT(find->testo = realloc(find->testo, somma + 2), NULL, ERROR: malloc);
		strncat(find->testo, text, strlen(text));
		addFrequenza(find->freq);
	} 
	return 0;
}

int writeFile(nodo* root, char* name, char* text, pid_t cLock)
{
	int tmp;
	if ((findTreeFromName(root,name)) == NULL) {	
			if ((tmp = openFile(root,name,0,cLock)) != 0){
				return tmp;
			}
			else 
				return appendToFile(root,name,text,cLock);
		}
	else	return -1;
}

//	cerca il nome nel nodo poi si cerca una foglia qualsiasi e con le considerazioni della funzione searchLeaf
//	si scambia con la foglia e cancello la foglia 
int fileRemove(nodo* root, char* nome, pid_t cLock)
{
	if (root == NULL)	return -1;

	if (strcmp(nome, "pRoot") == 0)	return -1;

	nodo* find = NULL;
	nodo* leaf = NULL;

	if (!(find = findTreeFromName(root,nome)))	return -3;
	if (find->stato != 0)	return -2;
	if (find->lucchetto == 0 && find->sLock != cLock)	return -2;

	if (!(leaf = searchLeaf(root)))	return -3;

	if ((strlen(find->nome) == strlen(leaf->nome)) && (strlen(find->testo) == strlen(leaf->testo))){
		free(leaf->nome);
		free(leaf->testo);
		free(leaf);
		return 0;
	} else if (swapTree(leaf, find) == 0){
		free(leaf->nome);
		free(leaf->testo);
		free(leaf);
		addFrequenza(find->freq);
		return 0;
	}	
	return -1;
}

//	cambia lo stato del nodo (0 aperto - 1 chiuso)
int changeStatus(nodo* root, char* name, int lb, pid_t cLock)
{
	nodo* find = NULL;
	if (strcmp(name, "pRoot") == 0)	return -1;
	if ((find = findTreeFromName(root,name)) == NULL)	return -3;
	if (find->lucchetto == 0 && find->sLock != cLock)	return -2;
	else {
		if (lb == find->stato)	return -2;
		//find->stato = lb;
		if (find->stato == 0)	find->stato = 1;
		else if (find->stato == 1)	find->stato = 0;
		}

	addFrequenza(find->freq);
	return 0;
}

//	cambia lo stato del nodo (1 Unlock - 0 Lock)
int changeLock(nodo* root, char* name, int lb, pid_t cLock)
{
	nodo* find = NULL;
	if (strcmp(name, "pRoot") == 0)	return -1;
	if ((find = findTreeFromName(root,name)) == NULL)	return -3;
	else {
			//	UNLOCK
			if (lb == 1){
				if (find->sLock == cLock)
					find->lucchetto = 1;
			}
			//	LOCK
			if (lb == 0){
				if (find->lucchetto == 0)
					return -2;
				else{
					find->lucchetto = 0;
					find->sLock = cLock;
				}
			}
		}

	addFrequenza(find->freq);
	return 0;
}

void cleanTree (nodo* root)
{
	if (root == NULL)	return;

	//	ciclo nell'albero
	cleanTree(root->right);
	cleanTree(root->left);

	// cancello
	free(root->nome);
	free(root->testo);
	free(root);
}