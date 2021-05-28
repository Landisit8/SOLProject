#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <parsing.h>
#include <utils.h>

#define config "./setup/config.txt"

//	attraverso il file config che contiene i 3 parametri, estrabolo le informaizoni
//	e le carico in 3 variabili.
int parsing (long* thrw, long* memMax, char** sktname)
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
	if ((buffer=malloc(MAXS*sizeof(char))) == NULL)
	{
		perror("Error malloc");
		free(buffer);
		return -1;
	}
	
	while(fgets(buffer,MAXS,fd) != NULL)
	{
		//	controllo di aver letto tutta la riga
		char* nline;
		if ((nline=strchr(buffer, '\n')) == NULL)
		{
			fprintf(stderr, "buffer troppo piccolo, aumentare il MAX (%d) ricompilando con '-DMAX=<n>' \n", MAXS);
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
		if ((string=malloc(MAXS*sizeof(char))) == NULL)
		{
			perror("Error malloc");
			free(string);
			return -1;
		}
		char *token = strtok(buffer,"=");
		token = strtok(NULL, "=");
		strncpy(string, token, MAXS);
		switch(isNumber(string, &tmp))
		{
			case 0:
				c++;
				if (c == 1)	*thrw = tmp;
				else if (c == 2) *memMax = tmp;
			break;
			case 1:
				if (c < 2) fprintf(stderr, "non hai inserito un numero \n");
				else strncpy(*sktname, string, MAXS);
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