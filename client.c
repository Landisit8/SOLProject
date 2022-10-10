#define _POSIX_C_SOURCE 199309L
#include <API.h>
#include <utils.h>
#include <conn.h>
#include <ops.h>
//#include <utilslist.h>

#define MAX 2048

char* cartellaLettura = NULL;
char* cartellaEspulsi = NULL;
char* SOCKET = NULL;

//lista* option;

/**
 * \ Guarda se sei dentro la directory corrente
 */
int isdot (const char dir[]){
	int l = strlen(dir);
	if (l > 0 && dir[l - 1] == '.') return 1;
	return 0;
}

/**
 * \ ottiene il pathname assoluto di un file
 */
char* cwd(){
	char* buf = alloca(MAX);

	if (getcwd(buf, MAX) == NULL){
		perror("Errore durante getcwd");
		free(buf);
		return NULL;
	}

	return buf;
}

/**
 * \
 * 
 */
// return -1: Errore
// return 0: non è risucito ad entrare nella cartella
// return 1: successo
int writeDir(const char* dirname,long* nume){
		if (chdir(dirname) == -1){
			// quando ho finito le cartelle da ciclare
			return 0;
		}

		DIR *dir;
		if ((dir = opendir(".")) == NULL){
			// errore nell'entrare nella cartella
			return -1;
		} else {
			struct dirent *file;

			while ((errno = 0, file = readdir(dir)) != NULL) {
				struct stat statb;

				// prendo le statistiche del file/cartelle
				if (stat(file->d_name, &statb) == -1){
					print_error("ERRORE stat %s\n", file->d_name);
					return -1;
				}

				// se il file è una cartella
				if (S_ISDIR(statb.st_mode)) {
					// controllo di non essere nella directory corrente
					if (!isdot(file->d_name)){
						// chiamata ricorsiva
						if (writeDir(file->d_name, nume) != 0) {
							// Torno indietro alla directory precedente
							if (chdir("..") == -1){
								fprintf(stderr,"impossibile tornare dalla directory precedente");
								return -1;
							}
						}
					}
				} else {
					if (*nume == 0) return 1;
					char* buf = cwd();
					buf = strncat(buf,"/",2);
					buf = strncat(buf, file->d_name, strlen(file->d_name));
					
					// chiamo la write file per vedere se è andato tutto bene
					if (writeFile(buf,cartellaEspulsi) == 0)	*nume = *nume - 1;
					free(buf);
				}

			}
			//	errore
			if (errno != 0)	perror("readdir");
			closedir(dir);
		}
		return 1;
}

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

/**
 *	\ Funzione che prende i valori inseriti dall'utente 
 *	\ e decide l'operazione da fare
*/
int parsing (int n, char** valori){
	struct timespec abstime;
	int opt;
	int r;
	int p = 0;
	//int bool = 0;
	size_t sz;
	long num = 0;
	long nume = -1;
	long sleeptime = 0;
	char* token;
	char* file;
	char *tmp = NULL;
	char* nome;
	char* dirname;
	void* buf = NULL;

	while ((opt = getopt(n,valori,"hf:w:W:D:r:R::d:t:l:u:c:p")) != -1){
		//nodo* node = alloca(sizeof(nodo*));
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
			/*
				bool = 1;
				node->lettera = 'w';
				listHead(node, option);
				*/
				token = strtok(optarg, ",");
				while (token != NULL){
					// prendo il dirname
					if (num == 0){
						dirname = alloca(strlen(token) + 1);
						strncpy(dirname, token, strlen(token));
						num++;;
					}
					if (num == 1) isNumber(token, &nume);
					token = strtok(NULL,",");
				}
				if (p){
					if (nume>0) fprintf(stdout,"Scrivo %ld file ", nume);
					else fprintf(stdout, "Scrivo tutti i file ");
					fprintf(stdout, "da questa cartella %s\n", dirname);
				}

				if (nume == 0)	nume = -1;

				if (writeDir(dirname, &nume) < 0)	return -1;	
				if (dirname)	free(dirname);
			break;
			case 'W':
				/*bool = 1;
				node->lettera = 'W';
				listHead(node, option);
				*/
				token = strtok(optarg,",");

				while(token != NULL)
				{
					file = alloca(strlen(token) + 1);
					strncpy(file, token, strlen(token) + 1);
					if (cartellaEspulsi)
						r = writeFile(file, cartellaEspulsi);
					//printf("valore di r: %d\n", r);
					else
						r = writeFile(file, NULL);
					if(r == -1)
					{
						perror("ERROR: write to file");
					}
					token = strtok(NULL, ",");
					free(file);
				}
			break;
			case 'D':
				cartellaEspulsi = alloca(strlen(optarg) + 1);
				strncpy(cartellaEspulsi,optarg,strlen(optarg) +1);
				if (p) fprintf(stdout,"I file espulsi vengono salvati in %s\n", optarg);
				/*if (doubleV(option) == 0){
					
				} else {
					fprintf(stderr, "NON E' STATO INSERITO W o w");
					return -1;
				}*/
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
					if (cartellaLettura) writeBytes(nome,buf,sz,"./read");
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

				if (cartellaLettura){
					r = readNFiles(num, cartellaLettura);
					if (r == -1){
					errno = ECONNREFUSED;
					perror("readNFiles");
				}
				} else {
					r = readNFiles(num, NULL);
					if (r == -1){
					errno = ECONNREFUSED;
					perror("readNFiles");
					}
				}
			break;
			case 'd':
				cartellaLettura = alloca(strlen(optarg) + 1);
				strncpy(cartellaLettura, optarg, strlen(optarg) + 1);
				if (p)	fprintf(stdout,"I file letti vengono salvati in %s\n", cartellaLettura);
				/*if (doubleV(option) == 0){
				} else {
					fprintf(stderr, "NON E' STATO INSERITO W o w");
					return -1;
				}*/
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
		/*
		if (bool == 0)	free(node);
			else	bool = 0;
			*/
	sleep(sleeptime);
	}
	return 0;
}


int main(int argc, char* argv[])
{
	//list_Start(option);
	if (parsing(argc,argv) == -1){
		perror("ERROR: opzioni non valide");
        exit(EXIT_FAILURE);
	}
	if(closeConnection(SOCKET) != 0)
    {
        perror("ERROR: Unable to close connection correctly with server");
		free(SOCKET);
        exit(EXIT_FAILURE);
    }
	//listClean(option);
	free(cartellaEspulsi);
	free(cartellaLettura);
	free(SOCKET);
	return 0;
}