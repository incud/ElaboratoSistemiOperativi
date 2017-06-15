#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "disco.h"
#include "concorrenza.h"
#include "procedure.h"

int main(int argc, char* argv[]) 
{
	struct Concorrenza conc;

	// estraggo parametri da riga di comando
	int ordine, nProcessi, i;
	char *pathA, *pathB, *pathC;
	if(argc == 6) {
		pathA = argv[1];
		pathB = argv[2];
		pathC = argv[3];
		ordine = atoi(argv[4]);
		nProcessi = atoi(argv[5]);
		stampa(STDOUT_FILENO, "Avviato il programma con ordine della matrice %i e nProcessi %i\n", ordine, nProcessi);
	} else {
		stampa(STDOUT_FILENO, "Il numero di parametri non è corretto. Chiama\n\t./eseguibile pathA pathB pathC ordine nProcessi\n");
		exit(0);
	}

	// inizializzazione
	registra_padre_nel_handler_interrupt(getpid(), &conc);
	if(crea_struttura_concorrenza(&conc, ordine, nProcessi) < 0) {
		stampa(STDOUT_FILENO, "Non e' stato possibile inizializzare le strutture\n");
		distruggi_struttura_concorrenza(&conc);
		exit(1);
	}

	// leggo matrici
	if(leggi_matrice_quadrata_automatizzato(pathA, conc.matriceA, ordine) < 0) {
		stampa(STDOUT_FILENO, "Non e' stato possibile leggere la matrice A da file '%s'\n", pathA);
		distruggi_struttura_concorrenza(&conc);
		exit(1);
	}
	if(leggi_matrice_quadrata_automatizzato(pathB, conc.matriceB, ordine) < 0) {
		stampa(STDOUT_FILENO, "Non e' stato possibile leggere la matrice B da file '%s'\n", pathB);
		distruggi_struttura_concorrenza(&conc);
		exit(1);
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

	// stampa risultato
	stampa(STDOUT_FILENO, "Risultato: %i\n", conc.risultato[0]);

	// stampa matrice su file
	if(stampa_matrice_quadrata(pathC, conc.matriceC, ordine) < 0) {
		stampa(STDOUT_FILENO, "Non e' stato possibile stampare la matrice C su file '%s'\n", pathC);
		distruggi_struttura_concorrenza(&conc);
		exit(1);
	}

	// uscita
	distruggi_struttura_concorrenza(&conc);

	return 0;
}