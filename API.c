#define _XOPEN_SOURCE   600
#define _POSIX_C_SOURCE 200112L

#include <API.h>
#include <utils.h>
#include <ops.h>
#include <conn.h>

int sockfd;

int openConnection(const char* sockname, int msec, const struct timespec abstime){
	errno = 0;
	int ris = 0;

	// controllo se i paramentri sono giusti
	if (sockname == NULL || msec < 0) {errno = EINVAL; return -1;}

	struct sockaddr_un serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path,sockname, strlen(sockname)+1);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {errno = -1; return -1;}

	struct timespec current_time;
	if ((clock_gettime(CLOCK_REALTIME, &current_time)) == -1) return -1;

	//	se la connessione falisce, ritenta un collegamento
	while ((ris = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) != 0 && abstime.tv_sec > current_time.tv_sec) {
		if ((ris = usleep(msec*1000)) != 0) return ris;
		if ((clock_gettime(CLOCK_REALTIME, &current_time)) == -1) return -1;
	}

	if (ris != -1) return 0;
	return ris;
}

/**
 *	\
 *	\
*/
int closeConnection(const char* sockname){
	errno = 0;

	msg_t* sockClose = alloca(sizeof(msg_t));
	sockClose->op = 8;

	//	invio il messaggio al server
	if (writen(sockfd,sockClose,sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERROR: Messaggio non inviato al server");
		return -1;
	}

	int ris = 0;

	// ricevo risposta dal server
	if (readn(sockfd, &ris, sizeof(int)) <= 0){
		errno = -1;
		perror("ERROR: Messaggio non ricevuto dal server");
		return -1;
	}

	// chiudo la connessione
	close(sockfd);
	// se il server risponde con "ok", chiusura' con successo
	if(ris == OP_END){
		printf("Chiusura' eseguita con successo\n");
		return 0;
	}
	// errore di tipo risposta dal server
	return -1;
}


int openFile(const char* pathname, int flags)
{
	msg_t* open = alloca(sizeof(msg_t));
	open->op = 0;
	open->flags = flags;	// 0 creazione, 1 lock, 2 stato, 3 crei, lock e stato
	errno = 0;

	//	lunghezza del pathname
	int nameLen = strlen(pathname)+1;

	strncpy(open->nome, pathname, nameLen);

	open->nome[nameLen] = '\0';
	open->lNome = nameLen; 

	open->cLock = getpid();

	//	mando il messaggio al server
	if (writen(sockfd, open, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: scrittura openFile");
	}

	//	ricevo il messaggio dal server
	msg_t tmp;
	if (readn(sockfd, &tmp, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: lettura risposta readFile");
	}

	if (tmp.op == OP_LFU){
		writeBytes(tmp.nome,tmp.str,tmp.lStr,"./LFU");
		openFile(pathname,flags);
	}
	if (tmp.op != OP_OK)
		return -1;
	return 0;
}

char* readBytes(const char* name, long* filelen)
{
	FILE *file = NULL;
	if ((file = fopen(name, "rb")) == NULL){
		perror("ERRORE: APERTURA FILE");
		fclose(file);
		return NULL;
	}

	if (fseek(file, 0, SEEK_END) == -1){
		perror("ERRORE: FSEEK");
		fclose(file);
		return NULL;
	}

	long lun = ftell(file);
	*filelen = lun;

	char* ret;
	ret = alloca(lun);

	if (fseek(file, 0, SEEK_SET) != 0) {
		perror("ERRORE: FSEEK");
		fclose(file);
		free(ret);
		return NULL;
	}

	int err;

	if ((err = fread(ret, 1, lun, file)) != lun){
		perror("ERRORE: FREAD");
		fclose(file);
		free(ret);
		return NULL;
	}

	fclose(file);
	return ret;
}

int writeBytes(const char* name, char* text, long size, const char* dirname)
{
	FILE *file;
	char* path = alloca(strlen(dirname) + strlen(name) + 1);

	sprintf(path, "%s/%s", dirname, name);
	path[strlen (dirname) + strlen(name) + 1] = '\0';
	errno = 0;
	if ((file = fopen(path, "wb")) == NULL){
		fprintf(stderr, "errno:%d\n", errno);
		return -1;
	}

	if ((fwrite(text, sizeof(char), size, file)) != size)	return -1;

	fclose(file);
	return 0;
}

/**
 *	\Legge tutto il contenuto del file dal server, ritornando un puntatore.
 *	\Ritorna 0 in caso di successo, -1 in caso di fallimento, errno.
*/
int readFile(const char* pathname, void** buf, size_t* size)
{
	msg_t* read = alloca(sizeof(msg_t));
	read->op = 1;

	errno = 0;

	int nameLen = strlen(pathname) + 1;

	strncpy(read->nome, pathname, nameLen);

	read->nome[nameLen] = '\0';
	read->lNome = nameLen;

	read->cLock = getpid();

	//	mando il messaggio al server
	if (writen(sockfd, read, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: scrittura readFile");
		return -1;
	}

	//	ricevo il messaggio dal server
	msg_t tmp;
	if (readn(sockfd, &tmp, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: lettura risposta readFile");
	}
	
	if (tmp.op != OP_OK){printf("E' sbagliato\n");	return -1;}

	*size = tmp.lStr;
	char *tmp_buf = alloca((*size));
	strncpy(tmp_buf, tmp.str, tmp.lStr);
	*buf = (void*)tmp_buf;
	return 0;
}

int readNFiles(int N, const char* dirname){
	msg_t* reads = alloca(sizeof(msg_t));
	reads->op = 9;
	reads->flags = N;
	int tmpo = 0;
	int ret = 0;

	errno = 0;

	reads->cLock = getpid();

	//	mando il messaggio al server
	if (writen(sockfd, reads, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: scrittura readFile");
		return -1;
	}
	msg_t tmp;
	do{
		//	ricevo il messaggio dal server
		if (readn(sockfd, &tmp, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: lettura risposta readFile");
		}
		if (tmp.flags == 0)	return -1;
		if (tmpo == 0){
			N = tmp.flags;
			ret = tmp.flags;
			tmpo = 1;
		}
		writeBytes(tmp.nome,tmp.str,tmp.lStr,dirname);
		N--;
	}while(N>0);

	return ret;
}

      
/**
 *	\
 *	\
*/
int writeFile(const char* pathname, const char* dirname)
{
	msg_t* write = alloca(sizeof(msg_t));
	write->op = 2;
	errno = 0;

	int nameLen = strlen(pathname) + 1;

	strncpy(write->nome, pathname, nameLen);

	write->nome[nameLen] = '\0';
	write->lNome = nameLen;

	long fileLen;
	char* buf;

	if ((buf = readBytes(pathname, &fileLen)) == NULL){
		errno = -1;
		perror("ERRORE: Lettura di readFile");
		return -1;
	}

	memcpy(write->str, buf, fileLen);
	write->lStr = fileLen;
	write->cLock = getpid();
	
	//	mando il messaggio al server
	if (writen(sockfd, write, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: scrittura readFile");
		return -1;
	}

	//	ricevo il messaggio dal server
	msg_t tmp;
	if (readn(sockfd, &tmp, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: lettura risposta readFile");
	}

	if (tmp.op != OP_OK){printf("E' sbagliato\n");	return -1;}

	printf("ok\n");
	return 0;
}


/**
 *	\
 *	\
*/
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname)
{
	
	return 0;
}

/**
 *	\
 *	\
*/
int lockFile(const char* pathname)
{
	msg_t* lock = alloca(sizeof(msg_t));
	lock->op = 6;
	errno = 0;

	int nameLen = strlen(pathname) + 1;

	strncpy(lock->nome, pathname, nameLen);

	lock->nome[nameLen] = '\0';
	lock->lNome = nameLen;

	lock->cLock = getpid();
	
	//	mando il messaggio al server
	if (writen(sockfd, lock, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: scrittura lockFile");
		return -1;
	}

	//	ricevo il messaggio dal server
	msg_t tmp;
	if (readn(sockfd, &tmp, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: lettura risposta lockFile");
	}

	if (tmp.op != OP_OK){printf("E' sbagliato\n");	return -1;}

	printf("ok\n");
	return 0;
}

/**
 *	\
 *	\
*/
int unlockFile(const char* pathname)
{
	msg_t* unlock = alloca(sizeof(msg_t));
	unlock->op = 7;
	errno = 0;

	int nameLen = strlen(pathname) + 1;

	strncpy(unlock->nome, pathname, nameLen);

	unlock->nome[nameLen] = '\0';
	unlock->lNome = nameLen;

	unlock->cLock = getpid();

	//	mando il messaggio al server
	if (writen(sockfd, unlock, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: scrittura lockFile");
		return -1;
	}

	//	ricevo il messaggio dal server
	msg_t tmp;
	if (readn(sockfd, &tmp, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: lettura risposta lockFile");
	}	

	if (tmp.op != OP_OK){printf("E' sbagliato\n");	return -1;}

	printf("ok\n");
	return 0;
}

/**
 *	\
 *	\
*/
int closeFile(const char* pathname)
{
	msg_t* close = alloca(sizeof(msg_t));
	close->op = 4;
	errno = 0;

	int nameLen = strlen(pathname) + 1;

	strncpy(close->nome, pathname, nameLen);

	close->nome[nameLen] = '\0';
	close->lNome = nameLen;

	close->cLock = getpid();

	//	mando il messaggio al server
	if (writen(sockfd, close, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: scrittura lockFile");
		return -1;
	}

	//	ricevo il messaggio dal server
	msg_t tmp;
	if (readn(sockfd, &tmp, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: lettura risposta lockFile");
	}	

	if (tmp.op != OP_OK){printf("E' sbagliato\n");	return -1;}
	printf("ok\n");
	return 0;
}

/**
 *	\
 *	\
*/
int removeFile(const char* pathname)
{
	msg_t* remove = alloca(sizeof(msg_t));
	remove->op = 5;
	errno = 0;

	int nameLen = strlen(pathname) + 1;

	strncpy(remove->nome, pathname, nameLen);
	remove->nome[nameLen] = '\0';

	remove->cLock = getpid();

	//	mando il messaggio al server
	if (writen(sockfd, remove, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: scrittura lockFile");
		return -1;
	}

	//	ricevo il messaggio dal server
	msg_t tmp;
	if (readn(sockfd, &tmp, sizeof(msg_t)) <= 0){
		errno = -1;
		perror("ERRORE: lettura risposta lockFile");
	}	

	if (tmp.op != OP_OK){printf("E' sbagliato\n");	return -1;}
	printf("ok\n");
	return 0;
}