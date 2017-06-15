#ifndef CONCORRENZA_H_
#define CONCORRENZA_H_

/** Permessi per la gestione della shared memory */
#define SHAREDMEMORY_PERMESSI 0644
/** Permessi per la gestione dei semafori */
#define SEMAPHOREPOOL_PERMESSI 0644
/** Permessi per la gestione delle message queue */
#define MESSAGEQUEUE_PERMESSI 0644

#define SEMAFORO_PROCESSI_LIBERI 0
#define SEMAFORO_MUTEX_RISULTATO 1
#define BASE_SEMAFORI_CONTATORI 2

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

int crea_struttura_concorrenza(struct Concorrenza* conc, int ordine, int nProcessi);

int sincronizza_memoria_condivisa(struct Concorrenza* conc);

int distruggi_struttura_concorrenza(struct Concorrenza* conc);

int incrementa_semaforo(struct Concorrenza* conc, int semaforo);

int decrementa_semaforo(struct Concorrenza* conc, int semaforo);

int incrementa_contatore_riga(struct Concorrenza* conc, int riga);

int aspetta_contatore_riga(struct Concorrenza* conc, int riga);

int manda_messaggio_coda(struct Concorrenza* conc, int indice);

int ricevi_messaggio_coda(struct Concorrenza* conc, int* indice);

int estrai_numero_messaggi_coda(struct Concorrenza* conc, int* n);

#endif
