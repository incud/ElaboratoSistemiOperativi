#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "disco.h"
#include "concorrenza.h"
#include "procedure.h"

int main(int argc, char* argv[]) 
{
	struct Concorrenza conc;
	int ordine = 3, nProcessi = 7, i;

	// inizializzazione
	registra_padre_nel_handler_interrupt(getpid(), &conc);
	if(crea_struttura_concorrenza(&conc, ordine, nProcessi) < 0) {
		stampa(STDOUT_FILENO, "\tNon e' stato possibile inizializzare le strutture\n");
		distruggi_struttura_concorrenza(&conc);
		exit(1);
	}

	// inizializzazione matrice
	for(i = 0; i < ordine * ordine; i++) {
		conc.matriceA[i] = 1*i;
		conc.matriceB[i] = 2*i;
		conc.matriceC[i] = 3*i;
	}

	// avvio programma
	if(inizializza_processi(&conc) >= 0) {
		avvia_procedura_gestore(&conc);
	}

	// aspetto terminazione dei figli
	for(i = 0; i < nProcessi; i++) {
		int status;
		wait(&status);
	}

	// stampa 
	stampa(STDOUT_FILENO, "[Padre ] Uscita\nMatrice:\n");
	for(i = 0; i < ordine * ordine; i++) {
		stampa(STDOUT_FILENO, "%3i %3i %3i\n", conc.matriceA[i], conc.matriceB[i], conc.matriceC[i]);
	}
	stampa(STDOUT_FILENO, "Risultato: %i\n", conc.risultato[0]);

	// uscita
	distruggi_struttura_concorrenza(&conc);

	return 0;
}