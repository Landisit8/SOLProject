#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <time.h>

#include <API.h>
#include <utils.h>
#include <conn.h>
#include <ops.h>

#define MAX 2048

char* SOCKET = NULL;

void listaHelp(){
	printf("Opzioni:\n\n");
	printf("-h: Lista dei comandi.\n\n");
	printf("-f filename: Specifica il nome del socket a cui connettersi.\n\n");
	printf("-w dirname[,n=0]: Invia la richiesta di scrittura dei file nella directory 'dirname' nel file storage \n");
	printf("se viene specificato, invia i file. se non viene specificato non c'è limite al numero di file inviati.\n\n");
	printf("-W file 1[,file2]: Lista di nomi di file da scrivere nel server.\n\n");
	printf("-D dirname: Cartella in cui verranno salvati i file espulsi per mancanza di capacità in caso di '-w' e '-W' \n");
 	printf("Quindi deve essere utilizzato con quelle opzioni. se non è specificato, tutti i file verranno eliminati.\n\n");
	printf("-r file1[,file2]: Lista di nomi di file da leggere dal server separati da ','.\n\n");
	printf("-R [n=0]: Permette di leggere n file dalla memoria file, se non viene specificato vengono letti tutti i file.\n\n");
	printf("-d dirname: Cartella di memoria in cui vengono scritti i file letti con -r e -R, se non vengono specificati i file non vengono memorizzati.\n\n");
	printf("-t time: Tempo in millisecondi che intercorre tra l'invio di una richiesta dalla precedente al server, non specificato il valore sarà 0.\n\n");
	printf("-l file1[,file2]: Lista dei nomi di file su cui acquisire la mutua esclusione\n\n");
	printf("-u file1[,file2]: Lista dei nomi di file su cui rilasciare la mutua esclusione.\n\n");
	printf("-c file1[,file2]: Lista di file da rimuovere dal server se presenti.\n\n");
	printf("-p: Abilita' le stampe in tutto il progetto.\n\n");
}

int parsing (int n, char** valori){
	struct timespec abstime;
	int opt;

	while ((opt = getopt(n,valori,"hf:w:W:D:r:R::d:t:l:u:c:p")) != -1)
		switch(opt) 
		{
			case 'h':
				listaHelp();
				return -1;
			break;
			case 'f':
				//	Preparazione alla connessione
				if ((clock_gettime(CLOCK_REALTIME, &abstime)) == -1){
					errno = -1;
					perror("ERROR: -f");
					return -1;
				}
				abstime.tv_sec += 2;
				fprintf(stdout, "APERTURA CONNESSIONE A: %s \n", optarg);
				if ((openConnection(optarg, 1000, abstime)) == -1){
					errno = ECONNREFUSED;
					perror("openConnection");
					return -1;
				}
				SOCKET = alloca(strlen(optarg)+1);
				strncpy(SOCKET, optarg, strlen(optarg)+1);
				fprintf(stdout, "Connessione riuscita al socket\n");
			break;
			case 'w':
					if ((openFile(optarg,0)) == -1){
					errno = ECONNREFUSED;
					perror("openConnection");
					return -1;
				}
			break;
			case 'W':
			break;
			case 'D':

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
			case ':': 
               	printf("Errore, lista dei comandi: \n");
				listaHelp();
				return -1;
			break;
            case '?': 
                printf("Errore, lista dei comandi: \n");
				listaHelp();
				return -1;
			break;
		}
	return 0;
}


int main(int argc, char* argv[])
{
	int tmp;
	tmp = parsing(argc,argv);
	if (tmp == 0)
		if(closeConnection(SOCKET) != 0)
    	{
        	perror("ERROR: Unable to close connection correctly with server");
        	exit(EXIT_FAILURE);
    	}
	return 0;
}