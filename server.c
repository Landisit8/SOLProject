#define _POSIX_C_SOURCE  200112L
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/select.h>
#include <pthread.h>

#include <utils.h>
#include <conn.h>
#include <ops.h>
#include <lfucache.h>

#define config "./setup/config.txt"

char* sktname = NULL;

int fdmax;
long memMax;
long numMax;
fd_set set;

nodo pRoot = {0,"pRoot","abcd",1,NULL,NULL};

void cleanup() {
	unlink(sktname);
}

// ritorno l'indice massimo tra i descrittori attivi
int updatemax(fd_set set, int fdmax) {
    for(int i=(fdmax-1);i>=0;--i)
	if (FD_ISSET(i, &set)) return i;
    assert(1==0);
    return -1;
}

//	attraverso il file config che contiene i 4 parametri, estrabolo le informaizoni
//	e le carico in 4 variabili.
int parsing (long* thrw, long* memMax, char** sktname, long* numMax)
{
	//	descrittori
	FILE *fd = NULL;
	char *buffer = NULL;
	//	variabile di controllo
	short c = 0;
	//	variabile per la isNumber
	long tmp = 0;
	//	apro in lettura
	
	if ((fd = fopen(config, "r")) == NULL)
	{
		perror("Nessun file rilevato");
		fclose(fd);
		return -1;
	}
	//	alloco un buffer in memoria con un MAX piu' i controlli dedicati
	if ((buffer=malloc(MAXS*sizeof(char))) == NULL)
	{
		perror("Error malloc");
		free(buffer);
		return -1;
	}
	
	while(fgets(buffer,MAXS,fd) != NULL)
	{

		//	controllo di aver letto tutta la riga
		char* nline;
		if ((nline=strchr(buffer, ';')) == NULL)
		{
			fprintf(stderr, "buffer troppo piccolo, aumentare il MAX (%d) ricompilando con '-DMAX=<n>' \n", MAXS);
			return -1;
		}
		*nline = '\0';	//	elimino lo \n
		//	controllo se e' presente '='
		char* colonna;
		if ((colonna=strchr(buffer, '=')) == NULL)
		{
			fprintf(stderr, "Errore nel formato");
			return -1;
		}
		char *string;
		if ((string=malloc(MAXS*sizeof(char))) == NULL)
		{
			perror("Error malloc");
			free(string);
			return -1;
		}
		char *token = strtok(buffer,"=");
		token = strtok(NULL, "=");
		strncpy(string, token, MAXS);
		switch(isNumber(string, &tmp))
		{
			case 0:
				c++;
				if (c == 1)	*thrw = tmp;
				else if (c == 2) *memMax = tmp;
				else if (c == 3) *numMax = tmp;
			break;
			case 1:
				if (c < 3) fprintf(stderr, "non hai inserito un numero \n");
				else strncpy(*sktname, string, MAXS);
			break;
			case 2:
				fprintf(stderr, "overflow/underflow \n");
				free(string);
			break;
			default:
				fprintf(stderr, "Error in tokenizer");
			break;
		}
	}
	*memMax = *memMax * 1048576;	//	conversione da megabyte in byte
	return 0;
}

void message(int back, msg_t* msg)
{
	switch(back)
	{
		//	la funzione non ritorna errori
		case 0:
			msg->op = OP_OK;	
		break;
		//	la funzione non riesce a fare l'operazione
		case -1:
			msg->op = OP_FOK;
		break;
		//	la funzione e' bloccata dallo stato
		case -2:
			msg->op = OP_BLOCK;
		break;
		//	la funzione non trova il file richiesto
		case -3:
			msg->op = OP_FFL_SUCH;
		break;
		//	Chiusurà colleggamento immediata
		case -4:
			msg->op = OP_END;
		break;
	}
}

int operation(int fd_io, msg_t msg) {
	printf("sono dentro operation\n");
	int tmp;
    switch(msg.op)
    {
    	case OPEN_OP:
    		printf("sto eseguendo la open\n");
    		tmp = openFile(&pRoot,msg.nome);
    		message(tmp,&msg);
    		printf("%d\n", msg.op);
    		operation(fd_io,msg);
    	break;
    	case READ_OP:
    		printf("sto eseguendo la read\n");
    		tmp = readFile(&pRoot,msg.nome);
    		message(tmp,&msg);
    		operation(fd_io,msg);
    	break;
    	case WRITE_OP:
    		printf("sto eseguendo la write\n");
    		tmp = writeFile(&pRoot,msg.nome, msg.str);
    		message(tmp,&msg);
    		operation(fd_io,msg);
    	break;
    	case APPEND_OP:
    		printf("sto eseguendo la append\n");
    		tmp = appendToFile(&pRoot,msg.nome, msg.str);
    		message(tmp,&msg);
    		operation(fd_io,msg);
    		message(tmp,&msg);
    		operation(fd_io,msg);
    	break;
    	case CLOSE_OP:
    		printf("sto eseguendo la close\n");
    		tmp = changeStatus(&pRoot,msg.nome,1);
    		message(tmp,&msg);
    		operation(fd_io,msg);
    	break;
    	case REMOVE_OP:
    		printf("sto eseguendo la lfuremove\n");
    		//tmp = fileRemove(&pRoot,msg.nome);
    		tmp = lfuRemove(&pRoot);
    		message(tmp,&msg);
    		operation(fd_io,msg);
    	break;
    	case LOCK_OP:
    		printf("sto eseguendo la stato :)\n");
    		tmp = changeStatus(&pRoot,msg.nome,0);
    		message(tmp,&msg);
    		operation(fd_io,msg);
    	break;
		case END_OP:
			printf("sto eseguendo la chiusura del server\n");
			message(-4,&msg);
			operation(fd_io,msg);
		break;
    	case OP_OK:
			printf("Sto mandando ok\n");
    		if (writen(fd_io, &msg.op, sizeof(ops))<=0) {
				errno = -1;
				perror("ERRORE10: NON STO MANDANDO LA RISPOSTA AL CLIENT");
				return -1;
				}
    	break;
    	case OP_FOK:
    		printf("sto mandando no ok\n");
    		if (writen(fd_io, &msg, sizeof(int))<=0) {
				errno = -1;
				perror("ERRORE11: NON STO MANDANDO LA RISPOSTA AL CLIENT");
				return -1;
				}
    	break;
    	case OP_BLOCK:
    		if (writen(fd_io, &msg, sizeof(int))<=0) {
				errno = -1;
				perror("ERRORE12: NON STO MANDANDO LA RISPOSTA AL CLIENT");
				return -1;
				}

    	break;
    	case OP_FFL_SUCH:
    		if (writen(fd_io, &msg, sizeof(int))<=0) {
				errno = -1;
				perror("ERRORE13: NON STO MANDANDO LA RISPOSTA AL CLIENT");
				return -1;
				}
    	break;
    	case OP_MSG_SIZE:
    		if (writen(fd_io, &msg, sizeof(int))<=0) {
				errno = -1;
				perror("ERRORE14: NON STO MANDANDO LA RISPOSTA AL CLIENT");
				return -1;
				}
    	break;
    	case OP_END:
			fprintf(stderr, "sono dentro OP_END");
			fflush(stderr);
    		if (writen(fd_io, &msg.op, sizeof(ops))<=0) {
				errno = -1;
				perror("ERRORE15: NON STO MANDANDO LA RISPOSTA AL CLIENT");
				return -1;
				}
				close(fd_io);
				if (fd_io == fdmax)
				fdmax = updatemax(set,fdmax);
    	break;
    	default:
    		return -1;
    	break;
    }
    return 0;
}

int readValue(int fd_io)
{
	msg_t* msg = alloca(sizeof(msg_t));
	if (readn(fd_io, msg, sizeof(msg_t))<=0){
    	perror("ERRORE: lettura messaggio dal client");
        return -1;
    }
 	operation(fd_io,*msg);
    return 0;
}

int main (int argc, char* argv[])
{
	long thrw;
	addTree(&pRoot,0,"ema","gay",0);
	addTree(&pRoot,5,"amelia","hola",0);
	addTree(&pRoot,7,"fede","bello",0);

	//	controllo se file config non esiste
	if ((sktname = malloc(MAXS*sizeof(char))) == NULL)
	{

		perror("malloc");
		free(sktname);
		return EXIT_FAILURE;
	}
	
	parsing(&thrw,&memMax,&sktname,&numMax);

	printf("nthread:%ld - memoria:%ld - socketname:%s - nfile massimi:%ld\n", thrw, memMax, sktname,numMax);

	cleanup();
	atexit(cleanup);
	/*
  	*********************************************
 	Creazione del socket e generazione del pool di thread
 	*********************************************
	*/

	//	socket per la comunicazione
	int fd;
	//	socket per I/O
	int fd_c;
	//set per descriptor attivi, tmpset copia del set per la select
	fd_set tmpset;
	//	serve solo per la macro exit  
	int notused;

	//	Creazione del socket
	SYSCALL_EXIT("socket", fd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
    //	genero la struttura del socket
	struct sockaddr_un serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;    
	strncpy(serv_addr.sun_path, sktname, strlen(sktname)+1);
	//	genero il collegamento con il socket
	SYSCALL_EXIT("bind", notused, bind(fd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)), "bind", "");
	//	mi metto in ascolto
	SYSCALL_EXIT("listen", notused, listen(fd, MAXBACKLOG), "listen", "");


	// Creazione del parametro da passare ai thread worker. Ho un array di param e in ogni posizione dell'array
 	// faccio in modo di passare un numero che mi identifica il thread e l'fd_set


    // 0 tmpset e aggiungo fd all'insieme
	FD_ZERO(&set);
	FD_ZERO(&tmpset);
	FD_SET(fd, &set);

	//	il massimo dei descrittori
	fdmax = fd;

	//loop infinito
    for(;;) 
    {      
		// preparo la maschera per la select
		tmpset = set;
		if (select(fdmax+1, &tmpset, NULL, NULL, NULL) == -1) 
		{ 
			// attenzione al +1
	    	perror("select");
	    	return -1;
		}
		// cerchiamo di capire da quale fd abbiamo ricevuto una richiesta
		for(int i = 0; i <= fdmax; i++) 
		{
			//se fa parte del set...
	   		if (FD_ISSET(i, &tmpset)) 
	   		{
				if (i == fd) 
				{ 
					//...ed se e' una nuova richiesta di connessione...
					//...acetto la connessione 
		  			SYSCALL_EXIT("accept", fd_c, accept(fd, (struct sockaddr*)NULL ,NULL), "accept", "");
		  			FD_SET(fd_c, &set);  // aggiungo il descrittore al master set
		  			if(fd_c > fdmax) fdmax = fd_c;  // ricalcolo il massimo
		  			printf("Nuovo max: %d\n", fdmax);
		  			continue;
				} 

				fd_c = i;  // e' una nuova richiesta da un client già connesso
				// eseguo il comando e se c'e' un errore lo tolgo dal master set
				if (readValue(fd_c) < 0) 
				{ 
		  			close(fd_c); 
		  			FD_CLR(fd_c, &set); 
		  			// controllo se deve aggiornare il massimo
					//senza questo gli fd risultano sempre in richiesta di attenzione non so perchè
		 			if (fd_c == fdmax) fdmax = updatemax(set, fdmax);
				}
	    	}
		}
	}
	close(fd);
	free(sktname);
	return 0;
}