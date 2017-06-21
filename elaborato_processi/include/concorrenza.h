/**
 * @file
 * @brief Funzioni a basso livello per la gestione della concorrenza
 */

#ifndef CONCORRENZA_H_
#define CONCORRENZA_H_

/** Permessi per la gestione della memoria condivisa, semafori e coda di messaggi */
#define CONCORRENZA_PERMESSI 0644

/** Numero del semaforo associato al contatore dei processi liberi */
#define SEMAFORO_PROCESSI_LIBERI 0

/** Numero del semaforo associato al mutex della cella 'risultato' */
#define SEMAFORO_MUTEX_RISULTATO 1

/** Numero di base dei semafori mutex del contatore */
#define BASE_SEMAFORI_MUTEX_CONTATORI 2

/** Struttura che contiene le informazioni necessarie alla gestione delle
 * system call sulla concorrenza */
struct Concorrenza 
{
	/** chiavi delle system call shmget, msgget, semget */
	struct Chiave {
		/** id della matrice A */
		int idMatriceA;
		/** id della matrice B */
		int idMatriceB;
		/** id della matrice C */
		int idMatriceC;
		/** id vettore celle   */
		int idCelle;
		/** id semafori        */
		int idSemafori;
		/** id coda messaggi   */
		int idCodaMessaggi;
		/** id contatori       */
		int idContatori;
		/** id variabile risultato */
		int idRisultato;
	} 
	/** struttura contenente le chiavi*/
	chiavi; 

	/** array condiviso di nProcessi celle che serve alla gestione dei processi da parte del processo padre */
	struct Cella {
		/** 1 se il processo è libero */
		int libero; 
		/** dati da inviare: indice i della coppia (i,j) che identifica la cella di una matrice */	
		int i;
		/** dati da inviare: indice j della coppia (i,j) che identifica la cella di una matrice */	
		int j;
		/** dati da inviare: riga di cui fare la somma */
		int riga; 
		/** descrittori della pipe */
		int pipe[2]; 	
	} 
	/** array di celle*/
	*celle; 

	/** numero di processi */
	int nProcessi; 
	/** ordine della matrice */		
	int ordine;		
	/** indirizzo della matrice A (condivisa) */	
	int* matriceA; 	
	/** indirizzo della matrice B (condivisa) */	
	int* matriceB;	
	/** indirizzo della matrice C (condivisa) */	
	int* matriceC;	
	/** array di contatori (condivisi). All'i-esimo elemento di contatori corrisponde il contatore del numero di celle completate della i-esima riga della matrice C */ 	
	int* contatori;		
	/** cella risultato (condivisa) */
	int* risultato;		
};

/* ================= CREAZIONE STRUTTURA ================= */

/** Inizializzazione della struttura concorrenza
 * @param[in,out] conc struttura da inizializzare
 * @param[in] ordine ordine della matrice quadrata
 * @param[in] nProcessi numero di processi da istanziare
 * @return 0 in caso di successo, -1 in caso di errore. In caso di errore deve essere chiamato distruggi_struttura_concorrenza()
 */
int crea_struttura_concorrenza(struct Concorrenza* conc, int ordine, int nProcessi);

/**
 * Dealloca la struttura
 * @param[in] conc struttura
 * @return 0
 */
int distruggi_struttura_concorrenza(struct Concorrenza* conc);

/* ================= GESTIONE SEMAFORI ================= */

/**
 * Inizializza i semafori della struttura
 * @param[in] conc struttura
 * @return 0 se tutto ok, -1 in caso di errore
 */
int inizializza_semafori(struct Concorrenza* conc);

/**
 * Aumenta di 1 il valore del semaforo
 * @param[in] conc struttura
 * @param[in] semaforo numero univoco del semaforo
 * @return 0 se tutto ok, -1 in caso di errore
 */
int incrementa_semaforo(struct Concorrenza* conc, int semaforo);

/**
 * Diminuisce di 1 il valore del semaforo (se negativo blocca il processo)
 * @param[in] conc struttura
 * @param[in] semaforo numero univoco del semaforo
 * @return 0 se tutto ok, -1 in caso di errore
 */
int decrementa_semaforo(struct Concorrenza* conc, int semaforo);

/**
 * Un processo lavoratore chiama questa funzione quando ha finito il suo compito ed è pronto a riceverne uno nuovo
 * @param[in] conc struttura
 */
void segnala_processo_libero(struct Concorrenza* conc);

/**
 * Il processo gestore chiama questa funzione per aspettare se non ci sono processi liberi attualmente
 * @param[in] conc struttura
 */
void aspetta_processo_libero(struct Concorrenza* conc);

/**
 * Un processo lavoratore intento in una operazione di moltiplicazione chiama questa funzione per segnalare la terminazione del suo lavoro
 * @param[in] conc struttura
 * @param[in] riga riga di cui si è completata la cella
 */
void segnala_cella_della_riga_completata(struct Concorrenza* conc, int riga);

/**
 * Un processo lavoratore intento in una operazione di somma chiama questa funzione per aspettare che tutti i lavori di moltiplicazione
 * sulla riga a lui assegnata siano finiti
 * @param[in] conc struttura
 * @param[in] riga riga da aspettare
 */
void aspetta_intera_riga_completata(struct Concorrenza* conc, int riga);

/* ================= GESTIONE CODA ================= */

/**
 * Un processo lavoratore manda sulla coda di messaggi il suo identificativo (indice nell'array celle) per segnalare la fine del suo lavoro
 * @param[in] conc struttura
 * @param[in] indice numero identificativo associato al processo
 * @return 0 se tutto ok, -1 altrimenti
 */
int manda_indice_in_coda(struct Concorrenza* conc, int indice);

/**
 * Il processo gestore estrae dalla coda l'indice relativo all'array celle di un processo che ha finito il suo compito
 * @param[in] conc struttura
 * @param[out] indice indice da riempire
 * @return 0 se tutto ok, -1 altrimenti
 */
int ricevi_indice_dalla_coda(struct Concorrenza* conc, int* indice);

/**
 * Mette nella variabile n il numero di messaggi in coda
 * @param[in] conc struttura
 * @param[out] n variabile da riempire
 * @return 0 se tutto ok, -1 altrimenti
 */
int ottieni_numero_messaggi_coda(struct Concorrenza* conc, int* n);

/* ================= GESTIONE MEMORIA CONDIVISA ================= */

/**
 * Inizializza la memoria condivisa
 * @param[in] conc struttura
 * @return 0 se tutto ok, -1 altrimenti
 */
int inizializza_memoria_condivisa(struct Concorrenza* conc);

#endif
