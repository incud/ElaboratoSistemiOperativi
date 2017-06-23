#include "disco.h"
#include "concorrenza.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

/* ================= CREAZIONE STRUTTURA ================= */

int crea_struttura_concorrenza(struct Concorrenza* conc, int ordine, int nProcessi)
{
	conc->ordine = ordine;
	conc->nProcessi = nProcessi;

	// setto a 0 tutte le chiavi (quelle che rimarranno a 0 non verranno distrutte - utile per la deallocazione di risorse in caso di errore)
	conc->chiavi.idMatriceA = -1;
	conc->chiavi.idMatriceB = -1;
	conc->chiavi.idMatriceC = -1;
	conc->chiavi.idCelle    = -1;
	conc->chiavi.idSemafori = -1;
	conc->chiavi.idCodaMessaggi = -1;
	conc->chiavi.idContatori    = -1;
	conc->chiavi.idRisultato    = -1;

	// creazione memoria condivisa
	conc->chiavi.idMatriceA = shmget(IPC_PRIVATE, sizeof(int) * ordine * ordine,      CONCORRENZA_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idMatriceB = shmget(IPC_PRIVATE, sizeof(int) * ordine * ordine,      CONCORRENZA_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idMatriceC = shmget(IPC_PRIVATE, sizeof(int) * ordine * ordine,      CONCORRENZA_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idCelle    = shmget(IPC_PRIVATE, sizeof(*(conc->celle)) * nProcessi, CONCORRENZA_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idContatori= shmget(IPC_PRIVATE, sizeof(int) * ordine,               CONCORRENZA_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idRisultato= shmget(IPC_PRIVATE, sizeof(int),                        CONCORRENZA_PERMESSI | IPC_CREAT | IPC_EXCL);
	if(conc->chiavi.idMatriceA < 0 || conc->chiavi.idMatriceB  < 0 || conc->chiavi.idMatriceC  < 0 || 
	   conc->chiavi.idCelle    < 0 || conc->chiavi.idContatori < 0 || conc->chiavi.idRisultato < 0) {
	   	stampa(STDOUT_FILENO, "Non e' stato possibile creare la memoria condivisa: %s\n", strerror(errno));
		return -1;
	}

	// creazione semafori
	const int N_SEMAFORI = 1 + 1 + ordine + ordine;
	conc->chiavi.idSemafori = semget(IPC_PRIVATE, N_SEMAFORI, CONCORRENZA_PERMESSI | IPC_CREAT | IPC_EXCL);
	if(conc->chiavi.idSemafori < 0) {
		stampa(STDOUT_FILENO, "Non e' stato possibile creare i semafori: %s\n", strerror(errno));
		return -1;
	}

	// creazione coda messaggi
	conc->chiavi.idCodaMessaggi = msgget(IPC_PRIVATE, CONCORRENZA_PERMESSI | IPC_CREAT | IPC_EXCL);
	if(conc->chiavi.idCodaMessaggi < 0) {
		stampa(STDOUT_FILENO, "Non e' stato possibile creare la coda di messaggi: %s\n", strerror(errno));
		return -1;
	}

	if(inizializza_memoria_condivisa(conc) < 0) {
		return -1;
	}

	if(inizializza_semafori(conc) < 0) {
		return -1;
	}

	// concluso correttamente
	return 0;
}

int distruggi_struttura_concorrenza(struct Concorrenza* conc)
{
	stampa(STDOUT_FILENO, "Cancellazione delle strutture condivise:\n");

	if(conc->chiavi.idMatriceA >= 0) {
		if(shmctl(conc->chiavi.idMatriceA, IPC_RMID, NULL) < 0) {
			stampa(STDERR_FILENO, "Errore durante chiusura area matrice A\n");
		} else {
			stampa(STDOUT_FILENO, "\tMatrice A cancellata\n");
		}
	}
	if(conc->chiavi.idMatriceB >= 0) {
		if(shmctl(conc->chiavi.idMatriceB, IPC_RMID, NULL) < 0) {
			stampa(STDERR_FILENO, "Errore durante chiusura area matrice B\n");
		} else {
			stampa(STDOUT_FILENO, "\tMatrice B cancellata\n");
		}
	}
	if(conc->chiavi.idMatriceC >= 0) {
		if(shmctl(conc->chiavi.idMatriceC, IPC_RMID, NULL) < 0) {
			stampa(STDERR_FILENO, "Errore durante chiusura area matrice C\n");
		} else {
			stampa(STDOUT_FILENO, "\tMatrice C cancellata\n");
		}
	}
	if(conc->chiavi.idCelle >= 0) {
		if(shmctl(conc->chiavi.idCelle, IPC_RMID, NULL) < 0) {
			stampa(STDERR_FILENO, "Errore durante chiusura area celle\n");
		} else {
			stampa(STDOUT_FILENO, "\tArray di celle condivise cancellato\n");
		}
	}
	if(conc->chiavi.idSemafori >= 0) {
		if(semctl(conc->chiavi.idSemafori, 0 /* semnum indifferente, li cancella tutti */, IPC_RMID) < 0) {
			stampa(STDERR_FILENO, "Errore durante chiusura semafori\n");
		} else {
			stampa(STDOUT_FILENO, "\tSemafori cancellati\n");
		}
	}
	if(conc->chiavi.idCodaMessaggi >= 0) {
		if(msgctl(conc->chiavi.idCodaMessaggi, IPC_RMID, NULL) < 0) {
			stampa(STDERR_FILENO, "Errore durante chiusura semafori\n");
		} else {
			stampa(STDOUT_FILENO, "\tCoda di messaggi cancellata\n");
		}
	}
	if(conc->chiavi.idContatori >= 0) {
		if(shmctl(conc->chiavi.idContatori, IPC_RMID, NULL) < 0) {
			stampa(STDERR_FILENO, "Errore durante chiusura area celle\n");
		} else {
			stampa(STDOUT_FILENO, "\tArray di contatori condivise cancellato\n");
		}
	}
	if(conc->chiavi.idRisultato >= 0) {
		if(shmctl(conc->chiavi.idRisultato, IPC_RMID, NULL) < 0) {
			stampa(STDERR_FILENO, "Errore durante chiusura area celle\n");
		} else {
			stampa(STDOUT_FILENO, "\tCella risultato condivise cancellato\n");
		}
	}

	return 0;
}

/* ================= GESTIONE SEMAFORI ================= */

int inizializza_semafori(struct Concorrenza* conc)
{
	int i;
	const int BASE_SEMAFORI_BLOCCO_CONTATORI = BASE_SEMAFORI_MUTEX_CONTATORI + conc->ordine;
	union semun { int val; struct semid_ds *buf; ushort * array; } argument;

	// inizializzo il semaforo dei processi liberi al numero di processi (all'inizio tutti liberi)
	argument.val = conc->nProcessi;
	if(semctl(conc->chiavi.idSemafori, SEMAFORO_PROCESSI_LIBERI, SETVAL, argument) < 0) {
		stampa(STDOUT_FILENO, "Non e' stato possibile inizializzare SEMAFORO_PROCESSI_LIBERI: %s\n", strerror(errno));
		return -1;
	}

	// mutex (inizializzato a 1) del semaforo
	argument.val = 1;
	if(semctl(conc->chiavi.idSemafori, SEMAFORO_MUTEX_RISULTATO, SETVAL, argument) < 0) {
		stampa(STDOUT_FILENO, "Non e' stato possibile inizializzare SEMAFORO_MUTEX_RISULTATO: %s\n", strerror(errno));
		return -1;
	}

	// tutti i semafori contatore mutex a 1
	argument.val = 1;
	for(i = 0; i < conc->ordine; i++) {
		conc->contatori[i] = 0;
		if(semctl(conc->chiavi.idSemafori, BASE_SEMAFORI_MUTEX_CONTATORI + i, SETVAL, argument) < 0) {
			stampa(STDOUT_FILENO, "Non e' stato possibile inizializzare BASE_SEMAFORI_MUTEX_CONTATORI: %s\n", strerror(errno));
			return -1;
		}
	}

	// tutti i semafori 'blocco contatore' sono settati a 0
	argument.val = 0;
	for(i = 0; i < conc->ordine; i++) {
		if(semctl(conc->chiavi.idSemafori, BASE_SEMAFORI_BLOCCO_CONTATORI + i, SETVAL, argument) < 0) {
			stampa(STDOUT_FILENO, "Non e' stato possibile inizializzare BASE_SEMAFORI_BLOCCO_CONTATORI: %s\n", strerror(errno));
			return -1;
		}
	}

	return 0;
}

int incrementa_semaforo(struct Concorrenza* conc, int semaforo)
{
	struct sembuf operation = {.sem_num = semaforo, .sem_op = 1, .sem_flg = 0};
	return semop(conc->chiavi.idSemafori, &operation, 1 /* numero di operazioni */);
}

int decrementa_semaforo(struct Concorrenza* conc, int semaforo)
{
	struct sembuf operation = {.sem_num = semaforo, .sem_op = -1, .sem_flg = 0};
	return semop(conc->chiavi.idSemafori, &operation, 1 /* numero di operazioni */);
}

void segnala_processo_libero(struct Concorrenza* conc)
{
	if(incrementa_semaforo(conc, SEMAFORO_PROCESSI_LIBERI) < 0) {
		stampa(STDOUT_FILENO, "Errore durante incremento SEMAFORO_PROCESSI_LIBERI: %s\n", strerror(errno));
	}
}

void aspetta_processo_libero(struct Concorrenza* conc)
{
	if(decrementa_semaforo(conc, SEMAFORO_PROCESSI_LIBERI) < 0) {
		stampa(STDOUT_FILENO, "Errore durante decremento SEMAFORO_PROCESSI_LIBERI: %s\n", strerror(errno));
	}
}

void segnala_cella_della_riga_completata(struct Concorrenza* conc, int riga)
{
	const int BASE_SEMAFORI_BLOCCO_CONTATORI = BASE_SEMAFORI_MUTEX_CONTATORI + conc->ordine;

	decrementa_semaforo(conc, BASE_SEMAFORI_MUTEX_CONTATORI + riga); // mutex
	conc->contatori[riga]++;
	if(conc->contatori[riga] == conc->ordine) {
		incrementa_semaforo(conc, BASE_SEMAFORI_BLOCCO_CONTATORI + riga); // sblocco
	}
	incrementa_semaforo(conc, BASE_SEMAFORI_MUTEX_CONTATORI + riga); // mutex
}

void aspetta_intera_riga_completata(struct Concorrenza* conc, int riga)
{
	const int BASE_SEMAFORI_BLOCCO_CONTATORI = BASE_SEMAFORI_MUTEX_CONTATORI + conc->ordine;

	decrementa_semaforo(conc, BASE_SEMAFORI_BLOCCO_CONTATORI + riga);
}

/* ================= GESTIONE CODA ================= */

int manda_indice_in_coda(struct Concorrenza* conc, int indice)
{
	struct Messaggio { long int type; int indice; } messaggio = { .type = 1 /* tipo >= 1 */, .indice = indice };
	return msgsnd(conc->chiavi.idCodaMessaggi, &messaggio, sizeof(int), 0 /* flag */);
}

int ricevi_indice_dalla_coda(struct Concorrenza* conc, int* indice)
{
	struct Messaggio { long int type; int indice; } messaggio;
	int esito = msgrcv(conc->chiavi.idCodaMessaggi, &messaggio, sizeof(int), 0 /* ogni tipo di messaggio */, 0 /* flag */);
	*indice = messaggio.indice;
	return esito;
}

int ottieni_numero_messaggi_coda(struct Concorrenza* conc, int* n)
{
	struct msqid_ds info;
	if(msgctl(conc->chiavi.idCodaMessaggi, IPC_STAT, &info) < 0) {
		return -1;
	} else {
		*n = info.msg_qnum;
		return 0;
	}
}

/* ================= GESTIONE MEMORIA CONDIVISA ================= */

int inizializza_memoria_condivisa(struct Concorrenza* conc)
{
	conc->matriceA  = shmat(conc->chiavi.idMatriceA, NULL, 0);
	conc->matriceB  = shmat(conc->chiavi.idMatriceB, NULL, 0);
	conc->matriceC  = shmat(conc->chiavi.idMatriceC, NULL, 0);
	conc->celle     = shmat(conc->chiavi.idCelle, NULL, 0);
	conc->risultato = shmat(conc->chiavi.idRisultato, NULL, 0);
	conc->contatori = shmat(conc->chiavi.idContatori, NULL, 0);
	if(conc->matriceA == NULL || conc->matriceB  == NULL || conc->matriceC  == NULL || 
	   conc->celle    == NULL || conc->risultato == NULL || conc->contatori == NULL) {
	   	stampa(STDOUT_FILENO, "Errore durante l'inizializzazione della memoria condivisa: %s\n", strerror(errno));
		return -1;
	} else {
		return 0;
	}
}
