#ifndef PROCEDURE_H_
#define PROCEDURE_H_

#include "concorrenza.h"

int inizializza_processi(struct Concorrenza* conc);

int avvia_procedura_gestore(struct Concorrenza* conc);

int avvia_procedura_lavoratore(struct Concorrenza* conc, int index);

#endif