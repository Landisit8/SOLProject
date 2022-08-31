#if !defined(API_H_)
#define API_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/**
 *	\Apre una connessione 
 *	\
*/
int openConnection(const char* sockname, int msec, const struct timespec abstime);
/**
 *	\Chiude la connessione
 *	\
*/
int closeConnection(const char* sockname);
/**
 *	\ 
 *	\
*/
int openFile(const char* pathname, int flags);
/**
 *	\
 *	\
*/
int readFile(const char* pathname, void** buf, size_t* size);
/**
 *	\
 *	\
*/
int writeFile(const char* pathname, const char* dirname);
/**
 *	\
 *	\
*/
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);
/**
 *	\
 *	\
*/
int lockFile(const char* pathname);
/**
 *	\
 *	\
*/
int unlockFile(const char* pathname);
/**
 *	\
 *	\
*/
int closeFile(const char* pathname);
/**
 *	\
 *	\
*/
int removeFile(const char* pathname);

#endif //