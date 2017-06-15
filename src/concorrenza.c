#include "disco.h"
#include "concorrenza.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

int crea_struttura_concorrenza(struct Concorrenza* conc, int ordine, int nProcessi)
{
	int i;

	conc->chiavi.idMatriceA = 0;
	conc->chiavi.idMatriceB = 0;
	conc->chiavi.idMatriceC = 0;
	conc->chiavi.idCelle    = 0;
	conc->chiavi.idSemafori = 0;
	conc->chiavi.idCodaMessaggi = 0;

	conc->chiavi.idMatriceA = shmget(IPC_PRIVATE, sizeof(int) * ordine * ordine,      SHAREDMEMORY_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idMatriceB = shmget(IPC_PRIVATE, sizeof(int) * ordine * ordine,      SHAREDMEMORY_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idMatriceC = shmget(IPC_PRIVATE, sizeof(int) * ordine * ordine,      SHAREDMEMORY_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idCelle    = shmget(IPC_PRIVATE, sizeof(*(conc->celle)) * nProcessi, SHAREDMEMORY_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idContatori= shmget(IPC_PRIVATE, sizeof(int) * ordine,               SHAREDMEMORY_PERMESSI | IPC_CREAT | IPC_EXCL);
	conc->chiavi.idRisultato= shmget(IPC_PRIVATE, sizeof(int),                        SHAREDMEMORY_PERMESSI | IPC_CREAT | IPC_EXCL);
	if(conc->chiavi.idMatriceA < 0 || conc->chiavi.idMatriceB < 0 || conc->chiavi.idMatriceB < 0 || conc->chiavi.idCelle < 0 || conc->chiavi.idContatori < 0 || conc->chiavi.idRisultato < 0) {
		stampa_formattato(STDERR_FILENO, "Errore durante creazione delle aree condivise\n");
		distruggi_struttura_concorrenza(conc);
		return -1;
	}

	conc->chiavi.idSemafori = semget(IPC_PRIVATE, 1 + 1 + ordine + ordine, SEMAPHOREPOOL_PERMESSI | IPC_CREAT | IPC_EXCL);
	if(conc->chiavi.idSemafori < 0) {
		stampa_formattato(STDERR_FILENO, "Errore durante creazione dei semafori\n");
		distruggi_struttura_concorrenza(conc);
		return -1;
	}

	union semun { int val; struct semid_ds *buf; ushort * array; } argument = { .val = nProcessi };
	if(semctl(conc->chiavi.idSemafori, SEMAFORO_PROCESSI_LIBERI, SETVAL, argument) < 0) {
		stampa_formattato(STDERR_FILENO, "Errore durante assegnazione SEMAFORO_PROCESSI_LIBERI\n");
		distruggi_struttura_concorrenza(conc);
		return -1;
	}

	// il mutex a 1
	argument.val = 1;
	if(semctl(conc->chiavi.idSemafori, SEMAFORO_MUTEX_RISULTATO, SETVAL, argument) < 0) {
		stampa_formattato(STDERR_FILENO, "Errore durante assegnazione SEMAFORO_MUTEX_RISULTATO\n");
		distruggi_struttura_concorrenza(conc);
		return -1;
	}

	// tutti i semafori contatore mutex a 1
	argument.val = 1;
	for(i = 0; i < ordine; i++) {
		conc->contatori[i] = 0;
		if(semctl(conc->chiavi.idSemafori, BASE_SEMAFORI_CONTATORI + i, SETVAL, argument) < 0) {
			stampa_formattato(STDERR_FILENO, "Errore durante assegnazione SEMAFORO_MUTEX_RISULTATO\n");
			distruggi_struttura_concorrenza(conc);
			return -1;
		}
	}

	// tutti i semafori contatore blocco a 0
	argument.val = 0;
	for(i = 0; i < ordine; i++) {
		if(semctl(conc->chiavi.idSemafori, BASE_SEMAFORI_CONTATORI + ordine + i, SETVAL, argument) < 0) {
			stampa_formattato(STDERR_FILENO, "Errore durante assegnazione SEMAFORO_MUTEX_RISULTATO\n");
			distruggi_struttura_concorrenza(conc);
			return -1;
		}
	}
 
	conc->chiavi.idCodaMessaggi = msgget(IPC_PRIVATE, MESSAGEQUEUE_PERMESSI | IPC_CREAT | IPC_EXCL);
	if(conc->chiavi.idCodaMessaggi < 0) {
		stampa_formattato(STDERR_FILENO, "Errore durante creazione coda messaggi\n");
		distruggi_struttura_concorrenza(conc);
		return -1;
	}

	conc->ordine = ordine;
	conc->nProcessi = nProcessi;

	return 0;
}

int sincronizza_memoria_condivisa(struct Concorrenza* conc)
{
	conc->matriceA  = shmat(conc->chiavi.idMatriceA, NULL, 0);
	conc->matriceB  = shmat(conc->chiavi.idMatriceB, NULL, 0);
	conc->matriceC  = shmat(conc->chiavi.idMatriceC, NULL, 0);
	conc->celle     = shmat(conc->chiavi.idCelle, NULL, 0);
	conc->risultato = shmat(conc->chiavi.idRisultato, NULL, 0);
	conc->contatori = shmat(conc->chiavi.idContatori, NULL, 0);
	if(conc->matriceA == NULL || conc->matriceB == NULL || conc->matriceC == NULL || conc->celle == NULL || conc->risultato == NULL || conc->contatori == NULL) {
		stampa_formattato(STDERR_FILENO, "Errore durante 'ottenimento' delle aree condivise\n");
		distruggi_struttura_concorrenza(conc);
		return -1;
	} else {
		return 0;
	}
}

int distruggi_struttura_concorrenza(struct Concorrenza* conc)
{
	if(conc->chiavi.idMatriceA > 0) {
		if(shmctl(conc->chiavi.idMatriceA, IPC_RMID, NULL) < 0) {
			stampa_formattato(STDERR_FILENO, "Errore durante chiusura area matrice A\n");
		}
	}
	if(conc->chiavi.idMatriceB > 0) {
		if(shmctl(conc->chiavi.idMatriceB, IPC_RMID, NULL) < 0) {
			stampa_formattato(STDERR_FILENO, "Errore durante chiusura area matrice B\n");
		}
	}
	if(conc->chiavi.idMatriceC > 0) {
		if(shmctl(conc->chiavi.idMatriceC, IPC_RMID, NULL) < 0) {
			stampa_formattato(STDERR_FILENO, "Errore durante chiusura area matrice C\n");
		}
	}
	if(conc->chiavi.idCelle > 0) {
		if(shmctl(conc->chiavi.idCelle, IPC_RMID, NULL) < 0) {
			stampa_formattato(STDERR_FILENO, "Errore durante chiusura area celle\n");
		}
	}
	if(conc->chiavi.idSemafori > 0) {
		if(semctl(conc->chiavi.idSemafori, 0 /* semnum indifferente, li cancella tutti */, IPC_RMID) < 0) {
			stampa_formattato(STDERR_FILENO, "Errore durante chiusura semafori\n");
		}
	}
	if(conc->chiavi.idCodaMessaggi > 0) {
		if(msgctl(conc->chiavi.idCodaMessaggi, IPC_RMID, NULL) < 0) {
			stampa_formattato(STDERR_FILENO, "Errore durante chiusura semafori\n");
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

int incrementa_contatore_riga(struct Concorrenza* conc, int riga) 
{
	decrementa_semaforo(conc, BASE_SEMAFORI_CONTATORI + riga); // mutex
	conc->contatori[riga]++;
	if(conc->contatori[riga] == conc->ordine) {
		incrementa_semaforo(conc, BASE_SEMAFORI_CONTATORI + conc->ordine + riga); // sblocco
	}
	incrementa_semaforo(conc, BASE_SEMAFORI_CONTATORI + riga); // mutex
	return 0;
}

int aspetta_contatore_riga(struct Concorrenza* conc, int riga) 
{
	decrementa_semaforo(conc, BASE_SEMAFORI_CONTATORI + conc->ordine + riga); // sblocco
	return 0;
}

int manda_messaggio_coda(struct Concorrenza* conc, int indice)
{
	struct Messaggio { int type; int indice; } messaggio = { .type = 1 /* tipo >= 1 */, .indice = indice };
	return msgsnd(conc->chiavi.idCodaMessaggi, &messaggio, sizeof(int), 0 /* flag */);
}

int ricevi_messaggio_coda(struct Concorrenza* conc, int* indice)
{
	struct Messaggio { int type; int indice; } messaggio;
	int esito = msgrcv(conc->chiavi.idCodaMessaggi, &messaggio, sizeof(int), 0 /* ogni tipo di messaggio */, 0 /* flag */);
	*indice = messaggio.indice;
	return esito;
}

int estrai_numero_messaggi_coda(struct Concorrenza* conc, int* n)
{
	struct msqid_ds info;
	if(msgctl(conc->chiavi.idCodaMessaggi, IPC_STAT, &info) < 0) {
		return -1;
	} else {
		*n = info.msg_qnum;
		return 0;
	}
}