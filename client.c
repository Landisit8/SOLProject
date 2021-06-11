#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <utils.h>
#include <conn.h>
#include <ops.h>

#define MAX 2048
/*
int parsing (int n, char* valori)
{
	int c;
	while ((c = getopt(n,valori,"hfwWrEdtlucp:")) != -1)
		switch(c)
	{
		case 'h':

		break;
		case 'f':

		break;
		case 'w':

		break;
		case 'W':

		break;
		case 'r':

		break;
		case 'R':

		break;
		case 'd':

		break;
		case 't':

		break;
		case 'l':

		break;
		case 'u':

		break;
		case 'c':

		break;
		case 'p':

		break;
		default:

		break;
	}
}
*/
int main(int argc, char* argv[])
{
	char* nome = "ema";
	char* testo = "gay";
	ops ope = 5;

	//parsing(argc,argv);

	struct sockaddr_un serv_addr;
	int sockfd;
	
	SYSCALL_EXIT("socket", sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
	 
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sun_family = AF_UNIX;    

	strncpy(serv_addr.sun_path,SOCKNAME, strlen(SOCKNAME)+1);
	int notused;
	
	SYSCALL_EXIT("connect", notused, connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "connect", "");
    
	char *buffer=NULL;

	int lNome=strlen(nome)+1;
	int lText=strlen(testo)+1;

	SYSCALL_EXIT("writen", notused, writen(sockfd, &lNome, sizeof(int)), "write", "");
	SYSCALL_EXIT("writen", notused, writen(sockfd, nome, lNome*sizeof(char)), "write", "");

	SYSCALL_EXIT("writen", notused, writen(sockfd, &lText, sizeof(int)), "write", "");
	SYSCALL_EXIT("writen", notused, writen(sockfd, testo, lText*sizeof(char)), "write", "");

	SYSCALL_EXIT("writen", notused, writen(sockfd, &ope, sizeof(ops)), "write", "");


	int l;
	SYSCALL_EXIT("readn", notused, readn(sockfd, &l, sizeof(int)), "read","");
	buffer = realloc(buffer, l*sizeof(char));
	if (!buffer) 
	{
   		perror("realloc");
   		fprintf(stderr, "Memoria esaurita....\n");
	}

	SYSCALL_EXIT("readn", notused, readn(sockfd, buffer, l*sizeof(char)), "read","");
	printf("%s\n", buffer);



    close(sockfd);
    if (buffer) free(buffer);
	return 0;
}