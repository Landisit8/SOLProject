#define _XOPEN_SOURCE   600
#define _POSIX_C_SOURCE 200112L
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include <utils.h>
#include <ops.h>
#include <conn.h>

int sockfd;

/**
 *	\
 *	\
*/
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
	sockClose->op = 7;

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

/**
 *	\
 *	\
*/
int openFile(const char* pathname, int flags)
{
	return 0;
}


/**
 *	\Legge tutto il contenuto del file dal server, ritornando un puntatore.
 *	\Ritorna 0 in caso di successo, -1 in caso di fallimento, errno.
*/
int readFile(const char* pathname, void** buf, size_t* size)
{
	return 0;
}
      
/**
 *	\
 *	\
*/
int writeFile(const char* pathname, const char* dirname)
{
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
	return 0;
}

/**
 *	\
 *	\
*/
int unlockFile(const char* pathname)
{
	return 0;
}

/**
 *	\
 *	\
*/
int closeFile(const char* pathname)
{
	return 0;
}

/**
 *	\
 *	\
*/
int removeFile(const char* pathname)
{
	return 0;
}