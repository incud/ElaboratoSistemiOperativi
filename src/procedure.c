#include "disco.h"
#include "procedure.h"
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void chiudi_pipe_fino_indice(struct Concorrenza* conc, int index)
{
	int i;
	for(i = 0; i < index; i++) {
		close(conc->celle[i].pipe[1]);
	}
}

int inizializza_processi(struct Concorrenza* conc)
{
	int i;
	for(i = 0; i < conc->nProcessi; i++) {

		conc->celle[i].libero = 1;

		// creazione pipe
		if(pipe(conc->celle[i].pipe) < 0) {
			chiudi_pipe_fino_indice(conc, i);
			kill(-1 * getpid(), SIGUSR1); // uccido tutti i figli
			return -1;
		}

		// creazione processo figlio
		int esito = fork();
		if(esito < 0) {
			chiudi_pipe_fino_indice(conc, i);
			kill(-1 * getpid(), SIGUSR1); // uccido tutti i figli
			return -1;

		} else if(esito == 0) {
			// figlio
			close(conc->celle[i].pipe[1]); // chiudo la pipe in scrittura, non mi serve
			sincronizza_memoria_condivisa(conc);
			avvia_procedura_lavoratore(conc, i);

		} else {
			// padre
			close(conc->celle[i].pipe[0]); // chiudo la pipe in lettura, non mi serve
		}
	}

	for(i = 0; i < conc->ordine; i++) {
		
	}
	conc->risultato[0] = 0;

	return 0;
}

static void aspetta_processo_libero(struct Concorrenza* conc) {
	decrementa_semaforo(conc, SEMAFORO_PROCESSI_LIBERI);
}

static void svuota_lista_completati(struct Concorrenza* conc) {
	// estraggo numero messaggi
	int nMessaggi, i;
	if(estrai_numero_messaggi_coda(conc, &nMessaggi) < 0) {
		stampa_formattato(STDOUT_FILENO, "[Padre ] Impossibile prendere numero messaggi coda\n");
		nMessaggi = 0;
	} else {
		stampa_formattato(STDOUT_FILENO, "[Padre ] Nella coda ci sono %i messaggi\n", nMessaggi);
	}

	// estraggo messaggi
	for(i = 0; i < nMessaggi; i++) {
		int indice;
		if(ricevi_messaggio_coda(conc, &indice) < 0) {
			stampa_formattato(STDOUT_FILENO, "[Padre ] Errore nella ricezione del messaggio\n");
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
				stampa_formattato(STDOUT_FILENO, "[Padre ] Impossibile inviare comando attraverso PIPE\n");
			} else {
				stampa_formattato(STDOUT_FILENO, "[Padre ] Inviato a (Proc %i) la cella i=%i; j=%i\n", indice, conc->celle[indice].i, conc->celle[indice].j);
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
			stampa_formattato(STDOUT_FILENO, "[Padre ] Impossibile inviare comando attraverso PIPE\n");
		} else {
			stampa_formattato(STDOUT_FILENO, "[Padre ] Inviato a (Proc %i) la riga %i\n", indice, conc->celle[indice].riga);
		}
	}
	
	for(i = 0; i < conc->nProcessi; i++) {
		stampa_formattato(STDOUT_FILENO, "\tIndice %i - Riga %i\n", i, conc->celle[i].riga);
	}

	// mando i messaggi di uscita
	for(i = 0; i < conc->nProcessi; i++) {
		if(write(conc->celle[i].pipe[1], "E", 1) < 0) {
			stampa_formattato(STDOUT_FILENO, "[Padre ] Impossibile inviare messaggio\n");
		} else {
			stampa_formattato(STDOUT_FILENO, "[Padre ] Inviato messaggio d'uscita al processo %i\n", i);
		}
	}

	return 0;
}

static void sblocca_processo(struct Concorrenza* conc) {
	incrementa_semaforo(conc, SEMAFORO_PROCESSI_LIBERI);
}

int avvia_procedura_lavoratore(struct Concorrenza* conc, int index)
{
	int i, j, k, N = conc->ordine, risultato, riga;

	while(1) {
		char messaggio;
		read(conc->celle[index].pipe[0], &messaggio, 1);

		switch(messaggio) {
			case 'M':
			case 'm':

				sleep(5);

				i = conc->celle[index].i;
				j = conc->celle[index].j;
				risultato = 0;

				stampa_formattato(STDOUT_FILENO, "[Proc %i] Moltiplica i=%i; j=%i\n", index, i, j);
				for(k = 0; k < N; k++) {
					risultato += conc->matriceA[i*N + k] * conc->matriceB[k*N + j];
				}
				conc->matriceC[i*N + j] = risultato;

				incrementa_contatore_riga(conc, i);

				sleep(5);

				break;
			case 'S':
			case 's':
				riga = conc->celle[index].riga;
				risultato = 0;

				sleep(1);

				aspetta_contatore_riga(conc, riga);

				stampa_formattato(STDOUT_FILENO, "[Proc %i] Somma riga=%i\n", index, riga);
				for(j = 0; j < N; j++) {
					risultato += conc->matriceC[riga * N + j];
				}

				decrementa_semaforo(conc, SEMAFORO_MUTEX_RISULTATO);
				conc->risultato[0] += risultato;
				incrementa_semaforo(conc, SEMAFORO_MUTEX_RISULTATO);

				sleep(5);

				break;
			case 'E':
			case 'e':
				stampa_formattato(STDOUT_FILENO, "[Proc %i] Uscita\n", index);
				exit(0);
			default:
				stampa_formattato(STDOUT_FILENO, "[Proc %i] Messaggio sconosciuto: %c\n", index, messaggio);
				break;
		}

		if(manda_messaggio_coda(conc, index) < 0) {
			stampa_formattato(STDOUT_FILENO, "[Proc %i] Errore nell'invio della conferma al padre: %s\n", index, strerror(errno));
		}
		sblocca_processo(conc);

		sleep(3);
	}
}
