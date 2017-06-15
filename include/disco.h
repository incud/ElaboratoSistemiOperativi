#ifndef DISCO_H_
#define DISCO_H_

#include <unistd.h> // definisce anche STDOUT_FILENO

#define DISKIO_BUFFER_SIZE 512

struct BufferedReader 
{
	char buffer[DISKIO_BUFFER_SIZE];
	int letti;
	int ultimo;
	int fileno;
	int fine;
};

/**
 * Apre un file in modalita' sola lettura
 * @param[in] path percorso del file
 * @param[out] br buffer del file di lettura
 * @return 0, -1 in caso di errori
 */
int apri_file_lettura(const char* path, struct BufferedReader* br);

/** 
 * Chiude il file
 * @param[in] br buffer del file di lettura
 * @return 0 se tutto ok, -1 in caso di errori
 */
int chiudi_file_lettura(struct BufferedReader* br);

/**
 * Legge il prossimo carattere dal buffer
 * @param[in,out] br buffer del file di lettura
 * @param[out] car carattere da riempire
 * @return 1 se lettura avvenuta, 0 se file finito, -1 in caso di errori
 */ 
int leggi_carattere(struct BufferedReader* br, char* car);

/**
 * Legge il prossimo carattere dal buffer
 * @param[in,out] br buffer del file di lettura
 * @param[out] intero intero da riempire
 * @return 1 se lettura avvenuta, 0 se file finito, -1 in caso di errori
 */ 
int leggi_intero(struct BufferedReader* br, int* intero);

/**
 * Riempie una matrice N*N
 * @param[in,out] br buffer del file di lettura
 * @param[out] matrice matrice da riempire
 * @param[in] ordine ordine della matrice (N)
 * @return 1
 */ 
int leggi_matrice_quadrata(struct BufferedReader* br, int* matrice, const int ordine);

/**
 * Riempie una matrice N*N
 * @param[in] path percorso del file
 * @param[out] matrice matrice da riempire
 * @param[in] ordine ordine della matrice (N)
 * @return 1
 */ 
int leggi_matrice_quadrata_automatizzato(const char* path, int* matrice, const int ordine);

/**
 * Stampa su file secondo il formato della printf
 * @param[in] fileno identificativo del file (di scrittura) dove andrà scritta la stringa
 * @param[in] formato formato della stringa (secondo le regole della printf)
 * @return 0 se tutto ok, < 0 in caso di errori
 */
int stampa(int fileno, const char* formato, ...);

/** 
 * Stampa su file la matrice indicata
 * @param[in] path percorso del file
 * @param[in] matrice matrice da leggere
 * @param[in] ordine ordine della matrice
 * @return 1 se tutto ok, -1 in caso di errori
 */
int stampa_matrice_quadrata(const char* path, const int* matrice, const int ordine);

#endif