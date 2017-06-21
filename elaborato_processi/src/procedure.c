#include "disco.h"
#include "procedure.h"
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ================= GESTIONE SEGNALI ================= */

static int PID_PADRE;
static struct Concorrenza* CONC_GLOBAL;

void registra_padre_nel_handler_interrupt(int pidPadre, struct Concorrenza* conc)
{
	PID_PADRE = pidPadre;
	CONC_GLOBAL = conc;
	signal(SIGUSR1, handler_interrupt); // segnale di kill inviato dalla funzione 'termina_tutti_figli'
	signal(SIGTERM, handler_interrupt);
	signal(SIGINT,  handler_interrupt); // CTRL+c
	signal(SIGTSTP, handler_interrupt); // CTRL+z
}

void termina_tutti_figli()
{
	kill(-1 * getpid(), SIGUSR1); // mando segnale al processo corrente e tutti i suo figli
}

void handler_interrupt(int segnale)
{
	if(getpid() != PID_PADRE) {
		exit(0);
	} else {
		termina_tutti_figli();
		distruggi_struttura_concorrenza(CONC_GLOBAL);
		exit(1);
	}
}

/* ================= PROCEDURE ================= */

int inizializza_processi(struct Concorrenza* conc)
{
	int i;

	// inizializzo contatori
	for(i = 0; i < conc->ordine; i++) {
		conc->contatori[i] = 0;
	}

	// inizializzo cella risultati
	conc->risultato[0] = 0;

	// inizializzo celle
	for(i = 0; i < conc->nProcessi; i++) {
		conc->celle[i].libero = 1;
	}

	// creo i processi
	for(i = 0; i < conc->nProcessi; i++) {

		// creazione pipe
		if(pipe(conc->celle[i].pipe) < 0) {
			return -1;
		}

		// creazione processo figlio
		int esito = fork();
		if(esito < 0) {
			return -1;

		} else if(esito == 0) {
			// figlio
			close(conc->celle[i].pipe[1]); // chiudo la pipe in scrittura, non mi serve
			avvia_procedura_lavoratore(conc, i);

		} else {
			// padre
			close(conc->celle[i].pipe[0]); // chiudo la pipe in lettura, non mi serve
		}
	}

	return 0;
}

static void svuota_lista_completati(struct Concorrenza* conc) {
	
	int nMessaggi, i;

	// estraggo numero messaggi
	if(ottieni_numero_messaggi_coda(conc, &nMessaggi) < 0) {
		stampa(STDOUT_FILENO, "[Padre ] Impossibile prendere numero messaggi coda\n");
		nMessaggi = 0;
	} else {
		stampa(STDOUT_FILENO, "[Padre ] Nella coda ci sono %i messaggi\n", nMessaggi);
	}

	// estraggo messaggi
	for(i = 0; i < nMessaggi; i++) {
		int indice;
		if(ricevi_indice_dalla_coda(conc, &indice) < 0) {
			stampa(STDOUT_FILENO, "[Padre ] Errore nella ricezione del messaggio\n");
		} else {
			conc->celle[indice].libero = 1;
		}
	}
}

static int cerca_processo_libero(struct Concorrenza* conc) {
	int i;
	for(i = 0; i < conc->nProcessi; i++) {
		if(conc->celle[i].libero == 1) {
			return i;
		}
	}
	// errore: nessun indice libero
	stampa(STDOUT_FILENO, "[Padre ] Non ho trovato nessun processo libero (il semaforo non funziona?)\n");
	return -1;
}

int avvia_procedura_gestore(struct Concorrenza* conc)
{
	int i, j;

	// eseguo moltiplicazioni
	for(i = 0; i < conc->ordine; i++) {
		for(j = 0; j < conc->ordine; j++) {
			aspetta_processo_libero(conc);
			svuota_lista_completati(conc);
			int indice = cerca_processo_libero(conc);
			// riempio la struttura
			conc->celle[indice].libero = 0;
			conc->celle[indice].i = i;
			conc->celle[indice].j = j;
			// mando il messaggio
			if(write(conc->celle[indice].pipe[1], "M", 1) < 0) {
				stampa(STDOUT_FILENO, "[Padre ] Impossibile inviare comando attraverso PIPE\n");
			} else {
				stampa(STDOUT_FILENO, "[Padre ] Inviato a (Proc %i) la cella i=%i; j=%i\n", indice, conc->celle[indice].i, conc->celle[indice].j);
			}
		}
	}

	// eseguo somme
	for(i = 0; i < conc->ordine; i++) {
		aspetta_processo_libero(conc);
		svuota_lista_completati(conc);
		int indice = cerca_processo_libero(conc);
		// riempio la struttura
		conc->celle[indice].libero = 0;
		conc->celle[indice].riga = i;
		// mando il messaggio
		if(write(conc->celle[indice].pipe[1], "S", 1) < 0) {
			stampa(STDOUT_FILENO, "[Padre ] Impossibile inviare comando attraverso PIPE\n");
		} else {
			stampa(STDOUT_FILENO, "[Padre ] Inviato a (Proc %i) la riga %i\n", indice, conc->celle[indice].riga);
		}
	}

	// mando i messaggi di uscita
	for(i = 0; i < conc->nProcessi; i++) {
		if(write(conc->celle[i].pipe[1], "E", 1) < 0) {
			stampa(STDOUT_FILENO, "[Padre ] Impossibile inviare messaggio\n");
		} else {
			stampa(STDOUT_FILENO, "[Padre ] Inviato messaggio d'uscita al processo %i\n", i);
		}
	}

	return 0;
}

void esegui_moltiplicazione(struct Concorrenza* conc, int index)
{
	int i, j, k, N = conc->ordine, risultato;
	i = conc->celle[index].i;
	j = conc->celle[index].j;
	risultato = 0;

	stampa(STDOUT_FILENO, "[Proc %i] Moltiplica i=%i; j=%i\n", index, i, j);
	for(k = 0; k < N; k++) {
		risultato += conc->matriceA[i*N + k] * conc->matriceB[k*N + j];
	}
	conc->matriceC[i*N + j] = risultato;

	segnala_cella_della_riga_completata(conc, i);
}

void esegui_somma(struct Concorrenza* conc, int index)
{
	int N = conc->ordine, risultato, riga, j;
	riga = conc->celle[index].riga;
	risultato = 0;

	aspetta_intera_riga_completata(conc, riga);

	stampa(STDOUT_FILENO, "[Proc %i] Somma riga=%i\n", index, riga);
	for(j = 0; j < N; j++) {
		risultato += conc->matriceC[riga * N + j];
	}

	decrementa_semaforo(conc, SEMAFORO_MUTEX_RISULTATO);
	conc->risultato[0] += risultato;
	incrementa_semaforo(conc, SEMAFORO_MUTEX_RISULTATO);
}

int avvia_procedura_lavoratore(struct Concorrenza* conc, int index)
{
	while(1) {
		char messaggio;
		read(conc->celle[index].pipe[0], &messaggio, 1);

		switch(messaggio) {
			case 'M':
				esegui_moltiplicazione(conc, index);
				break;
			case 'S':
				esegui_somma(conc, index);
				break;
			case 'E':
				stampa(STDOUT_FILENO, "[Proc %i] Uscita\n", index);
				exit(0);
			default:
				stampa(STDOUT_FILENO, "[Proc %i] Messaggio sconosciuto: %c\n", index, messaggio);
				break;
		}

		if(manda_indice_in_coda(conc, index) < 0) {
			stampa(STDOUT_FILENO, "[Proc %i] Errore nell'invio della conferma al padre: %s\n", index, strerror(errno));
		}

		segnala_processo_libero(conc);
	}
}
