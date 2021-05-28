#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <utils.h>
#include <conn.h>
#include <ops.h>

#define MAX 1024

int main(int argc, char* argv[])
{
	char* nome = "lorenzo";
	char* testo = "gay";
	ops ope = 0;

	struct sockaddr_un serv_addr;
	int sockfd;
	
	SYSCALL_EXIT("socket", sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
	 
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sun_family = AF_UNIX;    

	strncpy(serv_addr.sun_path,SOCKNAME, strlen(SOCKNAME)+1);
	int notused;
	
	SYSCALL_EXIT("connect", notused, connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "connect", "");
    
	char *buffer=NULL;

	int n=strlen(nome)+1;

	SYSCALL_EXIT("writen", notused, writen(sockfd, nome, MAX*sizeof(char)), "write", "");
	SYSCALL_EXIT("writen", notused, writen(sockfd, testo, MAX*sizeof(char)), "write", "");
	SYSCALL_EXIT("writen", notused, writen(sockfd, &ope, sizeof(ops)), "write", "");

	buffer = realloc(buffer, n*sizeof(char));
	if (!buffer) 
	{
   		perror("realloc");
   		fprintf(stderr, "Memoria esaurita....\n");
	}
	char* bello = NULL;
	bello = realloc(bello, n*sizeof(char));
	if (!bello) 
	{
   		perror("realloc");
   		fprintf(stderr, "Memoria esaurita....\n");
	};
		////////da problemi la read n
		SYSCALL_EXIT("readn", notused, readn(sockfd, buffer, MAX*sizeof(char)), "read","");
		printf("%s\n", buffer);
		SYSCALL_EXIT("readn", notused, readn(sockfd, bello, MAX*sizeof(char)), "read","");
		printf("%s\n", bello);

    close(sockfd);
    if (buffer) free(buffer);
    if (bello) free(bello);
	return 0;
}