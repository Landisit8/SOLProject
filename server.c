
#define _POSIX_C_SOURCE 200112L

#include <utils.h>
#include <conn.h>
#include <ops.h>
#include <lfucache.h>

#define config "./setup/config.txt"

//	variabili globali
char *sktname = NULL;
int fdmax;
// variabili per LOCK & UNLOCK
long memMax;
pthread_mutex_t setMemMax = PTHREAD_MUTEX_INITIALIZER;
long numMax;
pthread_mutex_t setNumMax = PTHREAD_MUTEX_INITIALIZER;
long thrw;
fd_set set;
pthread_mutex_t setLock = PTHREAD_MUTEX_INITIALIZER;
int cont = 0;
pthread_mutex_t setCont = PTHREAD_MUTEX_INITIALIZER;
FILE* log_file = NULL;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;
char *fileLogName;
// Pull di thread vuoto
pthread_t *thr = NULL;
// Messaggio di messaggi tmp 
msg_l *attesa;

pthread_mutex_t richiesta = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wait_attesa = PTHREAD_COND_INITIALIZER;

//	gestione dei segnali
volatile sig_atomic_t sig_interruzione = 0;
volatile sig_atomic_t sig_chiusura = 0;
volatile sig_atomic_t interrompi_segnali = 0;

pthread_t signal_handler;

sigset_t signal_mask;

nodo pRoot = {0, "pRoot", "abcd", 1, 1, 0, NULL, NULL,};
pthread_mutex_t setTree = PTHREAD_MUTEX_INITIALIZER;

void cleanup()
{
	unlink(sktname);
}

// ritorno l'indice massimo tra i descrittori attivi
int updatemax(fd_set set, int fdmax)
{
	for (int i = (fdmax - 1); i >= 0; --i)
		if (FD_ISSET(i, &set))
			return i;
	assert(1 == 0);
	return -1;
}

//	attraverso il file config che contiene i 5 parametri, estrabolo le informaizoni
//	e le carico in 5 variabili.
int parsing(long *thrw, long *memMax, char **sktname, long *numMax, char **fileLogName)
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
	if ((buffer = malloc(MAXS * sizeof(char))) == NULL)
	{
		perror("Error malloc");
		free(buffer);
		return -1;
	}

	while (fgets(buffer, MAXS, fd) != NULL)
	{

		//	controllo di aver letto tutta la riga
		char *nline;
		if ((nline = strchr(buffer, ';')) == NULL)
		{
			fprintf(stderr, "buffer troppo piccolo, aumentare il MAX (%d) ricompilando con '-DMAX=<n>' \n", MAXS);
			return -1;
		}
		*nline = '\0'; //	elimino lo \n
		//	controllo se e' presente '='
		char *colonna;
		if ((colonna = strchr(buffer, '=')) == NULL)
		{
			fprintf(stderr, "Errore nel formato");
			return -1;
		}
		char *string;
		if ((string = malloc(MAXS * sizeof(char))) == NULL)
		{
			perror("Error malloc");
			free(string);
			return -1;
		}
		char *token = strtok(buffer, "=");
		token = strtok(NULL, "=");
		strncpy(string, token, MAXS);
		switch (isNumber(string, &tmp))
		{
		case 0:
			c++;
			if (c == 1)
				*thrw = tmp;
			else if (c == 2)
				*memMax = tmp;
			else if (c == 3)
				*numMax = tmp;
			break;
		case 1:
			if (c < 3)
				fprintf(stderr, "non hai inserito un numero \n");
			else {
				c++;
				if (c == 4)	strncpy(*sktname, string, MAXS);
				else if (c == 5) strncpy(*fileLogName, string, MAXS);
			}
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
	*memMax = *memMax * 1048576; //	conversione da megabyte in byte
	return 0;
}

// Funzione di creazione del logFile
void fileLogCreate(){
	char *path = alloca(7 + strlen(fileLogName));
	sprintf(path, "./%s.txt", fileLogName);

	log_file = fopen(path, "w");

	if (log_file != NULL){
		fprintf(log_file,"LOG FILE\n");
		fflush(log_file);
	}

	free(path);

	return;
}

//	Funzione che cambia la op rendendo il messaggio da mandare
void message(int back, msg_t *msg)
{
	switch (back)
	{
	//	la funzione non ritorna errori
	case 0:
		msg->op = OP_OK;
		break;
	//	la funzione non riesce a fare l'operazione
	case -1:
		msg->op = OP_FOK;
		break;
	//	la funzione e' bloccata dallo stato o dalla lock
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
	case -5:
		msg->op = OP_LFU;
		break;
	}
}

//	Corpo del server, divide ogni operazione che può effettuare il server
int operation(int fd_io, msg_t msg)
{
	int tmp;
	msg_t re;	//ATTENZIONE
	msg_t* res;
	msg_l* buffer = NULL;
	switch (msg.op)
	{
	case OPEN_OP:
		LOCK(&log_lock);
		fprintf(log_file, "Open del file %s\n", msg.nome);	// LogFile
		fflush(log_file);
		UNLOCK(&log_lock);
		LOCK(&setTree);
		tmp = openFile(&pRoot, msg.nome, msg.flags, msg.cLock);
		UNLOCK(&setTree);
		message(tmp, &msg);
		operation(fd_io, msg);
		break;
	case READ_OP:
	//	caso particolare per mandare anche il testo insieme alla risposta
		LOCK(&log_lock);
		fprintf(log_file, "Read del file %s\n", msg.nome);	// LogFile
		UNLOCK(&log_lock);
		fflush(log_file);
		LOCK(&setTree);
		tmp = readFile(&pRoot, msg.nome, &re, msg.cLock);
		UNLOCK(&setTree);
		message(tmp, &re);
		if (writen(fd_io, &re, sizeof(msg_t)) <= 0)
		{
			errno = -1;
			perror("ERRORE10: NON STO MANDANDO LA RISPOSTA AL CLIENT");
			return -1;
		}
		break;
	case READS_OP:
	//	caso particolare per mandare anche il testo insieme alla risposta
		LOCK(&log_lock);
		fprintf(log_file, "Reads del file %s\n", msg.nome);	// LogFile
		fflush(log_file);
		UNLOCK(&log_lock);
		if (msg.flags == 0)	msg.flags = -1;
		buffer = alloca(sizeof(msg_l));
		msg_lStart(buffer);
		LOCK(&setTree);
		readsFile(&pRoot, msg.flags, msg.cLock, buffer);
		UNLOCK(&setTree);
		if (buffer->lung == 0){
			res->op = 0;
			if (writen(fd_io, res, sizeof(msg_t)) <= 0)
			{
				errno = -1;
				perror("ERRORE10: NON STO MANDANDO LA RISPOSTA AL CLIENT");
				return -1;
			}		
		}
		while ((buffer->lung) > 0){
			res = alloca(sizeof(msg_t));
			msgPopReturn(buffer, &res);
			res->flags = buffer->lung + 1;
			if (writen(fd_io, res, sizeof(msg_t)) <= 0)
			{
				errno = -1;
				perror("ERRORE10: NON STO MANDANDO LA RISPOSTA AL CLIENT");
				return -1;
			}
			free(res);
		}
		break;
	case WRITE_OP:
		LOCK(&log_lock);
		fprintf(log_file, "Write del file %s\n", msg.nome);	// LogFile
		fflush(log_file);
		UNLOCK(&log_lock);
		LOCK(&setTree);
		tmp = writeFile(&pRoot, msg.nome, msg.str, msg.cLock);
		UNLOCK(&setTree);
		message(tmp, &msg);
		operation(fd_io, msg);
		break;
	case APPEND_OP:
		LOCK(&log_lock);
		fprintf(log_file, "Append del file %s\n", msg.nome);	// LogFile
		fflush(log_file);
		UNLOCK(&log_lock);
		LOCK(&setTree);
		tmp = appendToFile(&pRoot, msg.nome, msg.str, msg.cLock);
		UNLOCK(&setTree);
		message(tmp, &msg);
		operation(fd_io, msg);
		break;
	case CLOSE_OP:
		LOCK(&log_lock);
		fprintf(log_file, "Close del file %s\n", msg.nome);	// LogFile
		fflush(log_file);
		UNLOCK(&log_lock);
		LOCK(&setTree);
		tmp = changeStatus(&pRoot, msg.nome, 1, msg.cLock);
		UNLOCK(&setTree);
		message(tmp, &msg);
		operation(fd_io, msg);
		break;
	case REMOVE_OP:
		LOCK(&log_lock);
		fprintf(log_file, "Remove del file %s\n", msg.nome);	// LogFile
		fflush(log_file);
		UNLOCK(&log_lock);
		LOCK(&setTree);
		tmp = fileRemove(&pRoot,msg.nome, msg.cLock);
		UNLOCK(&setTree);
		message(tmp, &msg);
		operation(fd_io, msg);
		break;
	case LOCK_OP:
		LOCK(&log_lock);
		fprintf(log_file, "Lock del file %s\n", msg.nome);	// LogFile
		fflush(log_file);
		UNLOCK(&log_lock);
		LOCK(&setTree);
		tmp = changeLock(&pRoot, msg.nome, 0, msg.cLock);
		UNLOCK(&setTree);
		message(tmp, &msg);
		operation(fd_io, msg);
		break;
	case UNLOCK_OP:
		LOCK(&log_lock);
		fprintf(log_file, "Unlock del file %s\n", msg.nome);	// LogFile
		fflush(log_file);
		UNLOCK(&log_lock);
		LOCK(&setTree);
		tmp = changeLock(&pRoot, msg.nome, 1, msg.cLock);
		UNLOCK(&setTree);
		message(tmp, &msg);
		operation(fd_io, msg);
		break;
	case END_OP:
		message(-4, &msg);
		operation(fd_io, msg);
		break;
	// caso della politica di cancellamento
	case OP_LFU:
		LOCK(&log_lock);
		fprintf(log_file, "LFU per il file %s\n", msg.nome);	// LogFile
		fflush(log_file);
		UNLOCK(&log_lock);
		LOCK(&setTree);
		tmp = lfuRemove(&pRoot, &re);
		UNLOCK(&setTree);
		LOCK(&setCont);
		cont++;
		UNLOCK(&setCont);
		LOCK(&setNumMax);
		numMax++;
		UNLOCK(&setNumMax);
		re.op = OP_LFU;
		if (tmp != 0){
			message(tmp,&msg);
			operation(fd_io, msg);
		}
		else{
			if (writen(fd_io, &re, sizeof(msg_t)) <= 0)
			{
				errno = -1;
				perror("ERRORE10: NON STO MANDANDO LA RISPOSTA AL CLIENT");
				return -1;
			}
		}
		break;
	//	Le risposte che si possono mandare
	case OP_OK:
		//	da cambiare il ritorno del messaggio, da op a msg x tutti
		if (writen(fd_io, &msg, sizeof(msg_t)) <= 0)
		{
			errno = -1;
			perror("ERRORE10: NON STO MANDANDO LA RISPOSTA AL CLIENT");
			return -1;
		}
		break;
	case OP_FOK:
		if (writen(fd_io, &msg, sizeof(msg_t)) <= 0)
		{
			errno = -1;
			perror("ERRORE11: NON STO MANDANDO LA RISPOSTA AL CLIENT");
			return -1;
		}
		break;
	case OP_BLOCK:
		if (writen(fd_io, &msg, sizeof(msg_t)) <= 0)
		{
			errno = -1;
			perror("ERRORE12: NON STO MANDANDO LA RISPOSTA AL CLIENT");
			return -1;
		}

		break;
	case OP_FFL_SUCH:
		if (writen(fd_io, &msg, sizeof(msg_t)) <= 0)
		{
			errno = -1;
			perror("ERRORE13: NON STO MANDANDO LA RISPOSTA AL CLIENT");
			return -1;
		}
		break;
	case OP_MSG_SIZE:
		if (writen(fd_io, &msg, sizeof(msg_t)) <= 0)
		{
			errno = -1;
			perror("ERRORE14: NON STO MANDANDO LA RISPOSTA AL CLIENT");
			return -1;
		}
		break;
	case OP_END:
		fprintf(log_file, "Chiusurà al client %d\n", fd_io);	// LogFile
		fflush(log_file);
		fflush(stderr);
		if (writen(fd_io, &msg.op, sizeof(ops)) <= 0)
		{
			errno = -1;
			perror("ERRORE15: NON STO MANDANDO LA RISPOSTA AL CLIENT");
			return -1;
		}
		close(fd_io);
		if (fd_io == fdmax)
			fdmax = updatemax(set, fdmax);
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

// Funzione che legge i valori dal client
void *readValue(void *arg)
{
	//è stato ricevuto un SIGINT o un SIGQUIT
	while (!sig_interruzione)
	{

		//SIGHUP ricevuto e ho terminato di servire tutte le richieste rimanenti
		if (sig_chiusura && attesa->testa == NULL){
			fprintf(stderr, "SIGHUP: terminato\n");
			break;
		}

		msg_t *msg = alloca(sizeof(msg_t));

		LOCK(&richiesta);
		WAIT(&wait_attesa, &richiesta);

		//	sse non sono stati mandati segnali di interruzione forzata controllo se ci sono richieste da servire
		if (!sig_interruzione && attesa->testa != NULL){
			msgPopReturn(attesa, &msg);

			UNLOCK(&richiesta);

			operation(msg->fd_c, *msg);

		if (msg->op != 8)
			{
				//fprintf(stderr, "reinserisco client\n");
				LOCK(&setLock); //faccio lock sul set prima di reinserire la richiesta
				FD_SET(msg->fd_c, &set);
				UNLOCK(&setLock);
			}
		}
		else	UNLOCK(&richiesta);	
		free(msg);
	}
	fflush(stderr);
	pthread_exit(NULL);
}

// Funzione per la gestione dei segnali
void* signalhandler(void*arg)
{
	fprintf(stderr,"\nSignal handler attivato\n");
	//variabile per ricevere il segnale
	int segnale = -1;

	//quando ricevo un segnale il thread si ferma
	while (!interrompi_segnali)
	{
		//aspetto il segnale
		sigwait(&signal_mask, &segnale);

		//segnale ricevuto
		interrompi_segnali = 1;

		//ricevo un segnale per fermare immediatamente il server
		if (segnale == SIGINT || segnale == SIGQUIT){
			fprintf(stderr, "SIGINT o SIGQUIT\n");
			//	segnalo ai worker
			sig_interruzione = 1;
			//sblocco la list
			pthread_cond_broadcast(&wait_attesa);

			//chiudo tutti i client connessi e annullo le le richieste
			msg_t* msg = attesa->testa;
			msg_t* msg_n;

			while (msg != NULL){
				close(msg->fd_c);
				msg_n = msg->next;
				free(msg);
				msg = msg_n;
			}

			//chiudo tutti i thread
			for (int i=0;i<thrw;i++)
				pthread_join(thr[i], NULL);
		}

		//chiudo il server dopo aver sentito tutte le richieste
		if (segnale == SIGHUP){
			fprintf(stderr, " SIGHUP\n");
			//segnalo ai worker
			sig_chiusura =1;
			//sblocco la lista
			pthread_cond_broadcast(&wait_attesa);
			//aspetto i thread
			for(int i=0;i<thrw;i++)
				pthread_join(thr[i], NULL);
		}
	}
	fprintf(stderr, "Uscita signal");
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	long thrw;

	fprintf(stdout, "Numero di processo del server: %d\n", getpid());
	//	controllo se file config non esiste
	if ((sktname = malloc(MAXS * sizeof(char))) == NULL)
	{

		perror("malloc");
		free(sktname);
		return -1;
	}

	fileLogName = alloca(MAXS);

	parsing(&thrw, &memMax, &sktname, &numMax, &fileLogName);

	fprintf(stdout,"nthread:%ld - memoria:%ld - socketname:%s - nfile massimi:%ld\n", thrw, memMax, sktname, numMax);

	cleanup();
	atexit(cleanup);
	
	fileLogCreate();

	int r = 0;

	sigset_t oldmask;

	SYSCALL_EXIT("sigemptyset", r, sigemptyset(&signal_mask), "ERRORE: sigemptyset", "");
	SYSCALL_EXIT("sigaddset", r, sigaddset(&signal_mask, SIGHUP), "ERRORE: sigaddset", "");
	SYSCALL_EXIT("sigaddset", r, sigaddset(&signal_mask, SIGINT), "ERRORE: sigaddset", "");
	SYSCALL_EXIT("sigaddset", r, sigaddset(&signal_mask, SIGQUIT), "ERRORE: sigaddset", "");
	
	//applico la maschera
	SYSCALL_EXIT("pthread_sigmask", r, pthread_sigmask(SIG_SETMASK, &signal_mask, &oldmask), "ERRORE: pthread_sigmask", "");

	//attivo il thread per la gestione dei segnali
    SYSCALL_EXIT("pthread_create", r, pthread_create(&signal_handler, NULL, &signalhandler, NULL), "ERROR: pthread_create", "");  

	/*
	Creazione del socket e generazione del pool di thread
	*/

	attesa = alloca(sizeof(msg_l));
	//	socket per la comunicazione
	int fd;
	//	socket per I/O
	int fd_c;
	//	per vericare il risultato della select
	int fd_s;
	// set per descriptor attivi, tmpset copia del set per la select
	fd_set tmpset;
	//	serve solo per la macro exit
	int notused;

	//	inizializzo gli elementi della lista
	msg_lStart(attesa);
	//	Creazione del socket
	SYSCALL_EXIT("socket", fd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
	//	genero la struttura del socket
	struct sockaddr_un serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, sktname, strlen(sktname) + 1);
	//	genero il collegamento con il socket
	SYSCALL_EXIT("bind", notused, bind(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)), "bind", "");
	//	mi metto in ascolto
	SYSCALL_EXIT("listen", notused, listen(fd, MAXBACKLOG), "listen", "");

	// Creazione del parametro da passare ai thread worker. Ho un array di param e in ogni posizione dell'array
	// faccio in modo di passare un numero che mi identifica il thread e l'fd_set

	//	Creazione e attivazione del pool di thread
	CHECK_EQ_EXIT((thr = (pthread_t *)calloc(thrw, sizeof(pthread_t))), NULL, "ERRORE: calloc threads");

	int res = 0;

	for (int i = 0; i < thrw; i++)
	{
		if ((res = pthread_create(&(thr[i]), NULL, &readValue, NULL) != 0))
		{
			perror("ERRORE: threads init");
			return -1;
		}
	}

	// 0 tmpset e aggiungo fd all'insieme
	FD_ZERO(&set);
	FD_ZERO(&tmpset);
	FD_SET(fd, &set);

	//	il massimo dei descrittori
	fdmax = fd;

	struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;

	// loop infinito
	while(!sig_chiusura && !sig_interruzione)
	{
		// preparo la maschera per la select
		tmpset = set;
		if (select(fdmax + 1, &tmpset, NULL, NULL, &timeout) == -1)
		{
			// attenzione al +1
			perror("select");
			return -1;
		}
		// cerchiamo di capire da quale fd abbiamo ricevuto una richiesta
		for (fd_s = 0; fd_s <= fdmax; fd_s++)
		{
			// se fa parte del set...
			if (FD_ISSET(fd_s, &tmpset))
			{
				if (fd_s == fd)
				{
					//...ed se e' una nuova richiesta di connessione...
					//...accetto la connessione
					SYSCALL_EXIT("accept", fd_c, accept(fd, (struct sockaddr *)NULL, NULL), "accept", "");
					FD_SET(fd_c, &set); // aggiungo il descrittore al master set
					fprintf(log_file, "Connessione al client %d\n", fd_c);	// LogFile
					fflush(log_file);
					if (fd_c > fdmax)
						fdmax = fd_c; // ricalcolo il massimo
					continue;
				}
				fd_c = fd_s;

				msg_t *rqs = alloca(sizeof(msg_t));

				if (readn(fd_c, rqs, sizeof(msg_t)) <= 0)
				{
					perror("ERRORE: lettura del messaggio");
					return -1;
				}

				//	tolgo il client dal set per evitare messaggi doppi
				FD_CLR(fd_c, &set);

				rqs->fd_c = fd_c;

				LOCK(&richiesta);
				msgHead(rqs, attesa);
				pthread_cond_signal(&wait_attesa);
				UNLOCK(&richiesta);
			}
		}
	}
	close(fd);
	unlink(sktname);
	free(sktname);
	unlink(fileLogName);
	free(fileLogName);
	return 0;
}