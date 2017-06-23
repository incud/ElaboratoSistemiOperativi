/**
 * @file
 * @brief Main del programma
 */ 

#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "disco.h"

/** indirizzo della matrice A */
int *matriceA = NULL;
/** indirizzo della matrice B */
int *matriceB = NULL;
/** indirizzo della matrice C */
int *matriceC = NULL;
/** Cella che conterrà la somma di tutti gli elementi di C */
int risultato = 0;
/** Mutex del risultato */
pthread_mutex_t lockRisultato; //, lockSignalHandler;
/** Ordine delle matrici */
int ordine = 0;

/** Inizializza le variabili globali */
void initialize_global_data()
{
	if(ordine <= 0) {
		stampa(STDERR_FILENO, "Non hai inizializzato ordine?\n");
		exit(1);
	}

	// allocazione memoria
	matriceA = malloc(sizeof(int) * ordine * ordine);
	matriceB = malloc(sizeof(int) * ordine * ordine);
	matriceC = malloc(sizeof(int) * ordine * ordine);

	// allocazione mutex
	if (pthread_mutex_init(&lockRisultato, NULL) != 0) {
        stampa(STDERR_FILENO, "Impossibile creare mutex\n");
        free(matriceA); free(matriceB); free(matriceC);
		exit(1);
    }

    // gestione del thread che forza l'esecuzione dell'handler in un solo thread
    // perchè in generale il comportamento è indefinito, il linux recente il segnale
    // arriva alla root thread e basta
    /* if (pthread_mutex_init(&lockSignalHandler, NULL) != 0) {
        stampa(STDERR_FILENO, "Impossibile creare mutex\n");
        free(matriceA); free(matriceB); free(matriceC);
        pthread_mutex_destroy(&lockRisultato);
		exit(1);
    } */
}

/** Libera le risorse associate alle variabili globali */
void terminate_global_data()
{
	free(matriceA);
	free(matriceB);
	free(matriceC);
	pthread_mutex_destroy(&lockRisultato);
	// pthread_mutex_destroy(&lockSignalHandler);
}

/** Gestore dei segnali
 * @param[in] signal numero del segnale
 */
void signal_handler(int signal) 
{
	// pthread_mutex_lock(&lockSignalHandler);

	stampa(STDOUT_FILENO, "Ricevuto segnale %s, terminazione\n", strsignal(signal));
	terminate_global_data();
	exit(0);
}

/** Struttura per il passaggio di dati alle thread */
struct ThreadData {
	/** Carattere che indica il compito da svolgere. 'M' per la moltiplicazione, 'S' per la somma */
	char compito;
	/** Nella modalità moltiplicazione, indica la riga della matrice A da utilizzare */
	int i;
	/** Nella modalità moltiplicazione, indica la colonna della matrice B da utilizzare */
	int j;
	/** Nella modalità somma, indica la riga della matrice C da utilizzare */
	int riga;
};

/** Funzione lavoratore
 * @param[in] argument parametro di tipo struct ThreadData
 * @return NULL
 */
void* execute_work(void* argument)
{
	struct ThreadData* threadData = (struct ThreadData*)argument;
	int i = threadData->i, j = threadData->j, riga = threadData->riga, k, temp;

	switch(threadData->compito) {
		case 'M':
			stampa(STDOUT_FILENO, "Esecuzione moltiplicazione su cella (%i,%i)\n", i, j);
			for(k = 0, temp = 0; k < ordine; k++) {
				temp += matriceA[i*ordine + k] * matriceB[k*ordine + j];
			}
			matriceC[i*ordine + j] = temp;
			break;

		case 'S':
			stampa(STDOUT_FILENO, "Esecuzione somma su riga %i\n", riga);
			for(k = 0, temp = 0; k < ordine; k++) {
				temp += matriceC[riga*ordine + k];
			}
			pthread_mutex_lock(&lockRisultato);
			risultato += temp;
			pthread_mutex_unlock(&lockRisultato);
			break;

		default:
			stampa(STDERR_FILENO, "Comando '%c' non riconosciuto\n", threadData->compito);
			break;
	}

	free(argument);
	return NULL; // pthread_exit
}

int main(int argc, char* argv[])
{
	int i;
	pthread_t* tids = NULL;

	// estraggo parametri da riga di comando
	char *pathA, *pathB, *pathC;
	if(argc == 5) {
		pathA = argv[1];
		pathB = argv[2];
		pathC = argv[3];
		ordine = atoi(argv[4]);
		stampa(STDOUT_FILENO, "Avviato il programma con ordine della matrice %i\n", ordine);
	} else {
		stampa(STDOUT_FILENO, "Il numero di parametri non è corretto. Chiama\n\t./eseguibile pathA pathB pathC ordine\n");
		exit(0);
	}

	// inzializzo dati globali e segnali
	signal(SIGTERM, signal_handler); // usato nel codice
	signal(SIGINT,  signal_handler); // CTRL+c
	signal(SIGTSTP, signal_handler); // CTRL+z
	initialize_global_data();

	// leggo matrici
	if(leggi_matrice_quadrata_automatizzato(pathA, matriceA, ordine) < 0) {
		stampa(STDERR_FILENO, "Non e' stato possibile leggere la matrice A da file '%s'\n", pathA);
		kill(getpid(), SIGTERM);
	}
	if(leggi_matrice_quadrata_automatizzato(pathB, matriceB, ordine) < 0) {
		stampa(STDERR_FILENO, "Non e' stato possibile leggere la matrice B da file '%s'\n", pathB);
		kill(getpid(), SIGTERM);
	}

	// creazione thread moltiplicazione
	tids = malloc(sizeof(pthread_t) * ordine * ordine);
	for(i = 0; i < ordine * ordine; i++) {
		struct ThreadData* data = malloc(sizeof(struct ThreadData));
		data->compito = 'M';
		data->i = i / ordine;
		data->j = i % ordine;
		if(pthread_create(tids + i, NULL, &execute_work, data) != 0) {
			stampa(STDERR_FILENO, "Non e' stato possibile allocare l'%i-esimo thread\n", i);
			free(tids);
			kill(getpid(), SIGTERM);
		}
	}
	for(i = 0; i < ordine * ordine; i++) { // aspetto terminazione di tutti i processi moltiplicazione
		pthread_join(tids[i], NULL);
	}
	free(tids);
	tids = NULL;

	// creazione thread somma
	tids = malloc(sizeof(pthread_t) * ordine);
	for(i = 0; i < ordine; i++) {
		struct ThreadData* data = malloc(sizeof(struct ThreadData));
		data->compito = 'S';
		data->riga = i;
		if(pthread_create(tids + i, NULL, &execute_work, data) != 0) {
			stampa(STDERR_FILENO, "Non e' stato possibile allocare l'%i-esimo thread\n", i);
			free(tids);
			kill(getpid(), SIGTERM);
		}
	}
	for(i = 0; i < ordine; i++) { // aspetto terminazione di tutti i processi moltiplicazione
		pthread_join(tids[i], NULL);
	}
	free(tids);
	tids = NULL;

	// stampa risultato
	stampa(STDOUT_FILENO, "Il risultato e': %i\n", risultato);

	// stampa matrice su file
	if(stampa_matrice_quadrata(pathC, matriceC, ordine) < 0) {
		stampa(STDERR_FILENO, "Non e' stato possibile stampare la matrice C su file '%s'\n", pathC);
		kill(getpid(), SIGTERM);
	}

	// terminazione programma
	terminate_global_data();
	exit(0);
}

