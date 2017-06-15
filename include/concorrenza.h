#ifndef CONCORRENZA_H_
#define CONCORRENZA_H_

/** Permessi per la gestione della memoria condivisa, semafori e coda di messaggi */
#define CONCORRENZA_PERMESSI 0644

#define SEMAFORO_PROCESSI_LIBERI 0
#define SEMAFORO_MUTEX_RISULTATO 1
#define BASE_SEMAFORI_MUTEX_CONTATORI 2

struct Concorrenza 
{
	struct Chiave {
		int idMatriceA;
		int idMatriceB;
		int idMatriceC;
		int idCelle;
		int idSemafori;
		int idCodaMessaggi;
		int idContatori;
		int idRisultato;
	} chiavi;

	struct Cella {
		int libero;
		int i, j, riga;
		int pipe[2];
	} *celle;

	int nProcessi;
	int ordine;
	int* matriceA;
	int* matriceB;
	int* matriceC;
	int* contatori;
	int* risultato;
};

/* ================= CREAZIONE STRUTTURA ================= */

int crea_struttura_concorrenza(struct Concorrenza* conc, int ordine, int nProcessi);

int distruggi_struttura_concorrenza(struct Concorrenza* conc);

/* ================= GESTIONE SEMAFORI ================= */

int inizializza_semafori(struct Concorrenza* conc);

int incrementa_semaforo(struct Concorrenza* conc, int semaforo);

int decrementa_semaforo(struct Concorrenza* conc, int semaforo);

void segnala_processo_libero(struct Concorrenza* conc);

void aspetta_processo_libero(struct Concorrenza* conc);

void segnala_cella_della_riga_completata(struct Concorrenza* conc, int riga);

void aspetta_intera_riga_completata(struct Concorrenza* conc, int riga);

/* ================= GESTIONE CODA ================= */

int manda_indice_in_coda(struct Concorrenza* conc, int indice);

int ricevi_indice_dalla_coda(struct Concorrenza* conc, int* indice);

int ottieni_numero_messaggi_coda(struct Concorrenza* conc, int* n);

/* ================= GESTIONE MEMORIA CONDIVISA ================= */

int inizializza_memoria_condivisa(struct Concorrenza* conc);

#endif
