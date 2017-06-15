#ifndef PROCEDURE_H_
#define PROCEDURE_H_

#include "concorrenza.h"

/* ================= GESTIONE SEGNALI ================= */

void registra_padre_nel_handler_interrupt(int pidPadre, struct Concorrenza* conc);

void handler_interrupt(int segnale);

void termina_tutti_figli();

/* ================= PROCEDURE ================= */

int inizializza_processi(struct Concorrenza* conc);

int avvia_procedura_gestore(struct Concorrenza* conc);

int avvia_procedura_lavoratore(struct Concorrenza* conc, int index);

#endif