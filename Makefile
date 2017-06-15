SOURCEDIR=src
INCLUDEDIR=include
BUILDDIR=build
TARGET=elaboratoincud

CC=gcc
CFLAGS=-Wall -I$(INCLUDEDIR) -ggdb

$(BUILDDIR)/$(TARGET): $(BUILDDIR)/main.o $(BUILDDIR)/disco.o $(BUILDDIR)/concorrenza.o $(BUILDDIR)/procedure.o
	@echo "Sto creando l'eseguibile, lo trovi al percorso" $@
	@$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/main.o: $(SOURCEDIR)/main.c
	@echo "Sto compilando" $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/disco.o: $(SOURCEDIR)/disco.c $(INCLUDEDIR)/disco.h
	@echo "Sto compilando" $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/concorrenza.o: $(SOURCEDIR)/concorrenza.c $(INCLUDEDIR)/concorrenza.h
	@echo "Sto compilando" $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/procedure.o: $(SOURCEDIR)/procedure.c $(INCLUDEDIR)/procedure.h
	@echo "Sto compilando" $@
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@rm -rf $(BUILDDIR)
	@mkdir $(BUILDDIR)