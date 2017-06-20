#ifndef PROCEDURE_H_
#define PROCEDURE_H_

#include "concorrenza.h"

/* ================= GESTIONE SEGNALI ================= */

/**
 * Inserisce i dati necessari per il corretto uso del handler di interrupt (da codice e da comando CTRL+C)
 * @param[in] pidPadre pid del processo padre/gestore
 * @param[in] con struttura globale
 */
void registra_padre_nel_handler_interrupt(int pidPadre, struct Concorrenza* conc);

/**
 * Gestore degli interrupt 
 * @param[in] segnale codice del segnale
 */
void handler_interrupt(int segnale);

/**
 * Uccide tutti i figli
 */
void termina_tutti_figli();

/* ================= PROCEDURE ================= */

/** 
 * Chiama la fork che alloca i processi figli 
 * @param[in] conc struttura gestore della concorrenza
 * @return 0 se tutto ok, -1 altrimenti
 */
int inizializza_processi(struct Concorrenza* conc);

/** 
 * Avvia il gestore dei processi
 * @param[in] conc struttura gestore della concorrenza
 * @return 0
 */
int avvia_procedura_gestore(struct Concorrenza* conc);

/**
 * Funzione utilizzata da 'inizializza_processi' che va partire il lavoro dei processi figli
 * @param[in] conc struttura gestore della concorrenza
 * @param[in] index indice nella struttura globale condivisa conc->celle
 * @return 0
 */
int avvia_procedura_lavoratore(struct Concorrenza* conc, int index);

#endif