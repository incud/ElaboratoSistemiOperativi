#include "disco.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int apri_file_lettura(const char* path, struct BufferedReader* br)
{
	int fd = open(path, O_RDONLY, 0600);
	if(fd < 0) {
		stampa_formattato(STDOUT_FILENO, "\tImpossibile aprire il file: %s\n", strerror(errno)); 
		return -1;
	} else {
		br->fileno = fd;
		br->letti = 0;
		br->ultimo = 0;
		br->fine = 0;
		return 0;
	}
}

int chiudi_file_lettura(struct BufferedReader* br)
{
	return close(br->fileno);
}

static int ricarica_bufferedreader(struct BufferedReader* br)
{
	int letti = read(br->fileno, br->buffer, DISKIO_BUFFER_SIZE);
	if(letti < 0) {
		return -1;
	} else if(letti == 0) {
		br->fine = 1;
		return 0;
	} else {
		br->letti = letti;
		br->ultimo = 0;
		return 1;
	}
}

int leggi_carattere(struct BufferedReader* br, char* car)
{
	if(br->ultimo >= br->letti) {
		int esito = ricarica_bufferedreader(br);
		if(esito < 0) { return -1; }
		if(esito == 0) { return 0; }
	}

	*car = br->buffer[br->ultimo];
	br->ultimo++;
	return 1;
}

int leggi_intero(struct BufferedReader* br, int* intero)
{
	char car;
	int esito;

	*intero = 0;

	// salto caratteri non numerici (suppongo solo spazi)
	do {
		esito = leggi_carattere(br, &car);
		if(esito < 0) { return -1; }
		if(esito == 0) { return 0; }
	} while(car < '0' && car > '9');

	// inizio a metter dentro le cifre (attento agli overflow!)
	do {
		int cifra = car - '0';
		*intero = *intero * 10 + cifra;

		esito = leggi_carattere(br, &car);
		if(esito < 0) { return -1; }
		if(esito == 0) { return 0; }

	} while(car >= '0' && car <= '9');

	return 0;
}

int leggi_matrice_quadrata(struct BufferedReader* br, int* matrice, const int ordine)
{
	int i, numero;
	for(i = 0; i < ordine * ordine; i++) {
		int esito = leggi_intero(br, &numero);
		if(esito < 0) { 
			stampa_formattato(STDOUT_FILENO, "\tErrore durante la lettura della matrice\n"); 
		} else {
			stampa_formattato(STDOUT_FILENO, "\tLetto: %i\n", numero); 
			matrice[i] = numero;
		}
	}

	return 1;
}

int leggi_matrice_quadrata_automatizzato(const char* path, int* matrice, const int ordine)
{
	struct BufferedReader br;

	// rileggo la matrice	
	int esito = apri_file_lettura(path, &br);
	if(esito < 0) { return -1; }

	esito = leggi_matrice_quadrata(&br, matrice, ordine);
	if(esito < 0) { return -1; }

	esito = chiudi_file_lettura(&br);
	if(esito < 0) { return -1; }

	return 0;
}

int stampa_formattato(int fileno, const char* formato, ...)
{
	char buffer[DISKIO_BUFFER_SIZE];

	va_list args;
    va_start(args, formato);

	int n = vsnprintf(buffer, DISKIO_BUFFER_SIZE, formato, args);
	buffer[n] = '\0';
	int esito = write(fileno, buffer, n);

	va_end(args);

	return esito;
}

int stampa_matrice_quadrata(const char* path, const int* matrice, const int ordine)
{
	int fileno = open(path, O_WRONLY | O_CREAT , 0666);
	if(fileno < 0) { 
		stampa_formattato(STDOUT_FILENO, "\tImpossibile aprire il file: %s\n", strerror(errno)); 
		return -1; 
	}

	int i;
	for(i = 0; i < ordine * ordine; i++) {
		int esito = stampa_formattato(fileno, "%i ", matrice[i]);
		if(esito < 0) { 
			stampa_formattato(STDOUT_FILENO, "\tErrore durante la scrittura della matrice: %s\n", strerror(errno)); 
			return -1;
		}
	}

	close(fileno);

	return 1;
}
