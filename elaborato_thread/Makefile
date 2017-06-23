SOURCEDIR=src
INCLUDEDIR=include
BUILDDIR=build
TARGET=elaboratothread

CC=gcc
CFLAGS=-Wall -I$(INCLUDEDIR) -ggdb -pthread -lpthread

$(BUILDDIR)/$(TARGET): $(BUILDDIR)/main.o $(BUILDDIR)/disco.o
	@echo "Sto creando l'eseguibile, lo trovi al percorso" $@
	@$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/main.o: $(SOURCEDIR)/main.c
	@echo "Sto compilando" $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/disco.o: $(SOURCEDIR)/disco.c $(INCLUDEDIR)/disco.h
	@echo "Sto compilando" $@
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean help run
clean:
	@rm -rf $(BUILDDIR)
	@mkdir $(BUILDDIR)

help:
	@echo "Esegui il comando 'make'."
	@echo "Si suppone esista una cartella 'build' nella directory corrente. In caso alternativo crearla oppure eseguire 'make clean' e poi 'make'."
	
run:
	@./$(BUILDDIR)/$(TARGET) mat_a mat_b mat_c 3