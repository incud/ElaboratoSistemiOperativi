#include "diskio.h"

// STDOUT_FILENO definito in diskio.h che include unistd.h

#define ORDINE 3

int main() 
{
	int i;
	int matrice[ORDINE*ORDINE] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

	// stampo la matrice su file
	int esito = stampa_matrice_quadrata("mio_file.txt", matrice, ORDINE);
	stampa_formattato(STDOUT_FILENO, "Esito della scrittura della matrice su file: %i\n", esito);

	// rileggo la matrice	
	esito = leggi_matrice_quadrata_automatizzato("mio_file.txt", matrice, ORDINE);
	stampa_formattato(STDOUT_FILENO, "Esito della lettura della matrice da file: %i\n", esito);

	// modifico la matrice
	for(i = 0; i < ORDINE*ORDINE; i++) {
		matrice[i] = matrice[i] * 10;
	}

	// la riscrivo - così, perchè ho voglia
	esito = stampa_matrice_quadrata("mio_file2.txt", matrice, ORDINE);
	stampa_formattato(STDOUT_FILENO, "Esito della scrittura della matrice su file: %i\n", esito);

	// rileggo la matrice	
	esito = leggi_matrice_quadrata_automatizzato("mio_file2.txt", matrice, ORDINE);
	stampa_formattato(STDOUT_FILENO, "Esito della lettura della matrice da file: %i\n", esito);

	// stampo a video
	stampa_formattato(STDOUT_FILENO, "\nMatrice:\n", esito);
	for(i = 0; i < ORDINE * ORDINE; i++) {
		stampa_formattato(STDOUT_FILENO, "%i ", matrice[i]);
		if((i+1) % ORDINE == 0) {
			stampa_formattato(STDOUT_FILENO, "\n");
		}
	}

	return 0;
}
