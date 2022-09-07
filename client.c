#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <time.h>
#include <dirent.h>

#include <API.h>
#include <utils.h>
#include <conn.h>
#include <ops.h>

#define MAX 2048


char* SOCKET = NULL;
/*
int writeDir(const char* dirname,long* n){
	if (chdir(dirname) == -1)	return 0;

	DIR *d;

	if ((d = opendir(".")) == NULL)	return -1;
	else {
		struct dir *file;
		
	}
}
*/

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
	void* buf = NULL;
	size_t sz;
	int r;
	char* token;
	char* file;
	char *tmp = NULL;
	long num = 0;
	long nume = -1;
	char* nome;
	long sleeptime = 0;
	int p;
	char* dirname;

	while ((opt = getopt(n,valori,"hf:w:W:D:r:R::d:t:l:u:c:p")) != -1){
		switch(opt) 
		{
			case 'h':
				listaHelp();
			break;
			case 'f':
				//	Preparazione alla connessione
				if ((clock_gettime(CLOCK_REALTIME, &abstime)) == -1){
					errno = -1;
					perror("ERROR: -f");
				}
				abstime.tv_sec += 2;
				if (p) fprintf(stdout, "APERTURA CONNESSIONE A: %s \n", optarg);
				if ((openConnection(optarg, 1000, abstime)) == -1){
					errno = ECONNREFUSED;
					perror("openConnection");
				}
				SOCKET = alloca(strlen(optarg)+1);
				strncpy(SOCKET, optarg, strlen(optarg)+1);
				if (p) fprintf(stdout, "Connessione riuscita al socket\n");
			break;
			case 'w':
					token = strtok(optarg, ",");
					while (token != NULL){
						// prendo il dirname
						if (num == 0){
							dirname = alloca(strlen(token));
							strncpy(dirname, token, strlen(token));
							num++;;
						}
						if (num == 1) isNumber(token, &num);
						token = strtok(NULL,",");
					}
					if (p){
						if (n>0) fprintf(stdout,"Scrivo %ld file ", nume);
						else fprintf(stdout, "Scrivo tutti i file ");
						fprintf(stdout, "da questa cartella %s\n", dirname);
					}

					if (nume == 0)	n = -1;

					//if (writeDir(dirname, &nume) < 0)
					return -1;		
			break;
			case 'W':
				token = strtok(optarg,",");

				while(token != NULL)
				{
					file = alloca(strlen(token) + 1);
					strncpy(file, token, strlen(token) + 1);
					file[strlen(token) + 1] = '\0';
					r = writeFile(file, NULL);
					//printf("valore di r: %d\n", r);
					if(r == -1)
					{
						perror("ERROR: write to file");
					}
					token = strtok(NULL, ",");
				}
			break;
			case 'D':

			break;
			case 'r':
				r = readFile(optarg, &buf, &sz);
				if (r == -1){
					errno = ECONNREFUSED;
					perror("readFile");
				}
				if (r == 0){	//da aggiungere il controllo se la cartella è NULL
					nome = strrchr(optarg, '/');
					nome++; 
					writeBytes(nome,buf,sz,"./read");
				}
			break;
			case 'R':
				//	preso da internet, https://stackoverflow.com/questions/1052746/getopt-does-not-parse-optional-arguments-to-parameters
				tmp = optarg;
				//	controllo se c'è l'argomento opzionale
				if (!optarg && NULL != valori[optind] && '-' != valori[optind][0]){
					tmp = valori[optind++];
				}
				//	se tmp esiste:
				if (tmp) isNumber(tmp, &num);

				r = readNFiles(num, "./read");
				if (r == -1){
					errno = ECONNREFUSED;
					perror("readNFiles");
				}
			break;
			case 'd':
			
			break;
			case 't':
				if((isNumber(optarg, &sleeptime)) == 1)
				{
					printf("opzione %s non e' un numero\n", optarg);
					return EXIT_FAILURE;
				}
				if(p) fprintf(stdout, "Timeout tra le richieste impostato su %ld\n\n", sleeptime);

			break;
			case 'l':
				r = lockFile(optarg);
				if (r == -1){
					errno = ECONNREFUSED;
					perror("lockFile");
				}
			break;
			case 'u':
				r = unlockFile(optarg);
				if (r == -1){
					errno = ECONNREFUSED;
					perror("unlockFile");
				}
			break;
			case 'c':
				r = removeFile(optarg);
				if (r == -1){
					errno = ECONNREFUSED;
					perror("removeFile");
				}
			break;
			case 'p':
				printf("P attivato\n\n");
                p = 1;
				set_p();
			break;
			case ':': 
               	printf("Errore, lista dei comandi: \n");
				listaHelp();
			break;
            case '?': 
                printf("Errore, lista dei comandi: \n");
				listaHelp();
			break;
		}
	sleep(sleeptime*1000);
	}
	return 0;
}


int main(int argc, char* argv[])
{
	parsing(argc,argv);
	if(closeConnection(SOCKET) != 0)
    {
        perror("ERROR: Unable to close connection correctly with server");
        exit(EXIT_FAILURE);
    }
	return 0;
}