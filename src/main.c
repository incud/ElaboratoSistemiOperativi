#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "disco.h"
#include "concorrenza.h"
#include "procedure.h"

int main(int argc, char* argv[]) 
{
	struct Concorrenza conc;
	int ordine = 3, nProcessi = 7, i;

	// inizializzazione struttura concorrenza
	crea_struttura_concorrenza(&conc, ordine, nProcessi);
	sincronizza_memoria_condivisa(&conc);

	// inizializzazione matrice
	for(i = 0; i < ordine * ordine; i++) {
		conc.matriceA[i] = 1*i;
		conc.matriceB[i] = 2*i;
		conc.matriceC[i] = 3*i;
	}

	for(i = 0; i < ordine * ordine; i++) {
		stampa_formattato(STDOUT_FILENO, "%i %i %i\n", conc.matriceA[i], conc.matriceB[i], conc.matriceC[i]);
	}

	// avvio programma
	if(inizializza_processi(&conc) >= 0) {
		avvia_procedura_gestore(&conc);
	}

	for(i = 0; i < nProcessi; i++) {
		int status;
		wait(&status);
	}

	stampa_formattato(STDOUT_FILENO, "[Padre ] Uscita\nMatrice:\n", conc.matriceC[i]);


	// stampa matrice
	for(i = 0; i < ordine * ordine; i++) {
		stampa_formattato(STDOUT_FILENO, "%i %i %i\n", conc.matriceA[i], conc.matriceB[i], conc.matriceC[i]);
	}

	stampa_formattato(STDOUT_FILENO, "Risultato: %i\n", conc.risultato[0]);

	// distruzione struttura concorrenza
	distruggi_struttura_concorrenza(&conc);

	return 0;
}