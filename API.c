#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int sockfd;

/**
 *	\
 *	\
*/
int openConnection(const char* sockname, int msec, const struct timespec abstime)
{
	errno = 0;
	int ris = 0;

	// controllo se i paramentri sono giusti
	if (sockname == NULL || msec < 0) {errno = EINVAL; return -1;}

	struct sockaddr_un serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path,SOCKNAME, strlen(SOCKNAME)+1);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {errno = -1; return -1;}

	struct timespec current_time;
	if ((clock_gettime(CLOCK_REALTIME, &current_time)) == -1) return -1;

	//	se la connessione falisce, ritenta un collegamento
	while ((result = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) != 0 && abstime.tv_sec > current_time.tv_sec) {
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
int closeConnection(const char* sockname)
{

}

/**
 *	\
 *	\
*/
int openFile(const char* pathname, int flags)
{

}


/**
 *	\Legge tutto il contenuto del file dal server, ritornando un puntatore.
 *	\Ritorna 0 in caso di successo, -1 in caso di fallimento, errno.
*/
int readFile(const char* pathname, void** buf, size_t* size)
{
	
}
      
/**
 *	\
 *	\
*/
int writeFile(const char* pathname, const char* dirname)
{

}


/**
 *	\
 *	\
*/
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname)
{

}

/**
 *	\
 *	\
*/
int closeFile(const char* pathname)
{

}

/**
 *	\
 *	\
*/
int removeFile(const char* pathname)
{

}