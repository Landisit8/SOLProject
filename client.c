#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <time.h>

#include <utils.h>
#include <conn.h>
#include <ops.h>

#define MAX 2048

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
	int opt;
	while ((opt = getopt(n,valori,"hfwWDrEdtlucp:")) != 1)
		switch(opt) 
		{
			case 'h':
				listaHelp();
				return -1;
			break;
			case 'f':
			/*
				// Il socket deve essere nella cartella
				if (add_current_folder(&SOCKNAME, optarg) == -1){
					errno = -1;
					perror("ERROR: -f");
					return -1;
				}
				//	Preparazione alla connessione
				if ((clock_gettime(CLOCK_REALTIME, &abstime)) == -1){
					errno = -1;
					perror("ERROR: -f");
					return -1;
				}
				abstime.tv_sec += 2;
				if (print) fprintf(stdout, "APERTURA CONNESSIONE A: %s \n", SOCKNAME);
				if ((openConnection(SOCKNAME, 1000, abstime)) == -1){
					errno = ECONNREFUSED;
					perror("openConnection");
					return -1;
				} else if(print) fprintf(stdout, "Connessione riuscita al socket\n\n");
				*/
			break;
			case 'w':

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
			default:
				printf("Errore, lista dei comandi: \n");
				listaHelp();
				return -1;
			break;
		}
	return 0;
}


int main(int argc, char* argv[])
{

	char *buffer=NULL;
	int tmp = 0;
	
	if ((tmp = parsing(argc,argv)) == 0){

		/*
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
		*/
	}
    if (buffer) free(buffer);
	return 0;
}