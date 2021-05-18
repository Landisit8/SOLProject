#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 *	\
 *	\
*/
int openConnection(const char* sockname, int msec, const struct timespec abstime)
{

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

int main(int argc, char* argv)
{

	return 0;
}