#define _POSIX_C_SOURCE  200112L
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/select.h>

#include <utils.h>
#include <conn.h>
#include <ops.h>
#include <lfucache.h>
#include <parsing.h>

/** 
 * tipo del messaggio
 */
typedef struct msg { 
	char *nome;
	ops op;
    char *str;
} msg_t;

char* sktname = NULL;

nodo pRoot = {0,"pRoot","abcd",1,NULL,NULL};

void cleanup() {
	unlink(sktname);
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
    		printf("sto cambiando il messaggio\n");
    		message(tmp,&msg);
    		printf("richiamo ricorsivamente\n");
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

    	break;
    	case APPEND_OP:
    		printf("sto eseguendo la append\n");

    	break;
    	case CLOSE_OP:
    		printf("sto eseguendo la close\n");
    		tmp = changeStatus(&pRoot,msg.nome);
    		message(tmp,&msg);
    		operation(fd_io,msg);
    	break;
    	case REMOVE_OP:
    		printf("sto eseguendo la remove\n");

    	break;
    	case STATUS_OP:
    		printf("sto eseguendo la stato :)\n");
    		tmp = changeStatus(&pRoot,msg.nome);
    		message(tmp,&msg);
    		operation(fd_io,msg);
    	break;
    	case OP_OK:
    		printf("sto mandando ok\n");
    		if (writen(fd_io, msg.nome, MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    		if (writen(fd_io, "Operazione eseguita con successo", MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    	break;
    	case OP_FOK:
    	printf("sto mandando no ok\n");
    		if (writen(fd_io, msg.nome, MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    		if (writen(fd_io, "Operazione non eseguita con successo", MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    	break;
    	case OP_BLOCK:
    		if (writen(fd_io, msg.nome, MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    		if (writen(fd_io, "File bloccato", MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    	break;
    	case OP_FFL_SUCH:
    		if (writen(fd_io, msg.nome, MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    		if (writen(fd_io, "File richiesto non esiste",MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    	break;
    	case OP_MSG_SIZE:
    		if (writen(fd_io, msg.nome, MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    		if (writen(fd_io, "Mesaggio troppo lungo", MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    	break;
    	case OP_END:
    		if (writen(fd_io, msg.nome, MAXS*sizeof(char))<=0) { free (msg.nome); return -1;}
    		if (writen(fd_io, "Raggiunto il massimo degli operazioni, attendere", MAXS*sizeof(char))<=0) { free(msg.nome); return -1;}
    	break;
    	default:
    		return -1;
    	break;
    }
    return 0;
}

int readValue (int fd_io)
{
	msg_t msg;

    msg.nome = calloc(MAXS * sizeof(char), sizeof(char));
    printf("1\n");
    if (readn(fd_io, msg.nome, MAXS*sizeof(char))<=0) return -1;
    printf("%s\n", msg.nome);

    msg.str = calloc(MAXS * sizeof(char), sizeof(char));
    printf("2\n");
    if (readn(fd_io, msg.str, sizeof(char)*MAXS)<=0) return -1;
    printf("%s\n", msg.str);

    printf("3\n");
    if (readn(fd_io, &msg.op, sizeof(ops))>0) {printf("%d\n", msg.op); operation(fd_io,msg);}
    else	return -1;

    return 0;
}



// ritorno l'indice massimo tra i descrittori attivi
int updatemax(fd_set set, int fdmax) {
    for(int i=(fdmax-1);i>=0;--i)
	if (FD_ISSET(i, &set)) return i;
    assert(1==0);
    return -1;
}

int main (int argc, char* argv[])
{
	cleanup();
	atexit(cleanup);

	//	controllo se file config non esiste
	long thrw;
	long memMax;
	if ((sktname = malloc(MAXS*sizeof(char))) == NULL)
	{

		perror("malloc");
		free(sktname);
		return EXIT_FAILURE;
	}
	
	parsing(&thrw,&memMax,&sktname);

	printf("nthread:%ld - memoria:%ld - socketname:%s \n", thrw, memMax, sktname);
	
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
	fd_set set, tmpset;
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
	int fdmax = fd;

	//loop infinito
	printf("Entro nel loop\n");
    for(;;) 
    {      
		// preparo la maschera per la select
		tmpset = set;
		printf("%d\n", fdmax);
		if (select(fdmax+1, &tmpset, NULL, NULL, NULL) == -1) 
		{ 
			// attenzione al +1
	    	perror("select");
	    	return -1;
		}
		printf("sono bello\n");
		// cerchiamo di capire da quale fd abbiamo ricevuto una richiesta
		for(int i = 0; i <= fdmax; i++) 
		{
			//se fa parte del set...
	   		if (FD_ISSET(i, &tmpset)) 
	   		{
	   			printf("sono bellissimo\n");
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
				printf("eseguo la readvalue\n");
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