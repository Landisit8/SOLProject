#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define config "./config.txt"
#define MAX 1024

/**
 *	\brief controlla se la stinga passata come primo argomento e' un numero.
 *	\return 0 ok 1 non e' un numero 2 oveeflow/underflow
*/
static int isNumber(const char* s, long* n) {
	if (s == NULL) return 1;
	if (strlen(s) == 0) return 1;
	char* e = NULL;
	errno=0;
	long val = strtol(s, &e, 10);
	if (errno == ERANGE) return 2;		//	overflow/underflow
	if (e != NULL && *e == (char)0) {
		*n = val;
		return 0;	//successo
	}
	return 1;	//non e' un numero
}

int parsing (long* thrw, long* memMax, char* sktname)
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
	if ((buffer=malloc(MAX*sizeof(char))) == NULL)
	{
		perror("Error malloc");
		free(buffer);
		return -1;
	}

	while(fgets(buffer,MAX,fd) != NULL)
	{
		//	controllo di aver letto tutta la riga
		char* nline;
		if ((nline=strchr(buffer, '\n')) == NULL)
		{
			fprintf(stderr, "buffer troppo piccolo, aumentare il MAX (%d) ricompilando con '-DMAX=<n>' \n", MAX);
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
			if ((string=malloc(MAX*sizeof(char))) == NULL)
		{
			perror("Error malloc");
			free(string);
			return -1;
		}
		char *token = strtok(buffer,"=");
		token = strtok(NULL, "=");
		strncpy(string, token, MAX);
		switch(isNumber(string, &tmp))
		{
			case 0:
				c++;
				if (c == 1)	*thrw = tmp;
				else if (c == 2) *memMax = tmp;
			break;
			case 1:
				if (c < 2) fprintf(stderr, "non hai inserito un numero \n");
				else strncpy(sktname, string, MAX);
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
	return 0;
}

int main ()
{
	long thrw;
	long memMax;
	char* sktname;
	if ((sktname = malloc(MAX*sizeof(char))) == NULL)
	{
		perror("malloc");
		free(sktname);
		return EXIT_FAILURE;
	}
	parsing(&thrw,&memMax,sktname);

	printf("nthread:%ld - memoria:%ld - socketname:%s \n", thrw, memMax, sktname);

	return 0;
}