############################################################
# Archivo Makefile del servidor multihilo sobre HTTP
#
# Autores: Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
############################################################

# Banderas de compilacion 
CCFLAGS = -std=gnu11 -Wall -g

# Directorios
SRC = ./src
INCLIB = ./includes
SRCLIB = ./srclib
LIBS = ./lib


all : lib server 
	rm -f *.o  # limpiamos los objetos pero no el ejecutable

lib: 
	@if [ ! -d lib ]; then\
        mkdir lib;\
    fi

# Ejecutable
server: main.o $(LIBS)/libservidor.a $(LIBS)/libhttp.a $(LIBS)/libparser.a
	gcc -o server $^ -L$(LIB) -lservidor -L$(LIB) -lhttp -L$(LIB) -lparser -lpthread -lconfuse  

# Objetos
main.o: $(SRC)/main.c
	gcc $(CCFLAGS) -I$(INCLIB) -c $(SRC)/main.c

servidor.o: $(SRCLIB)/servidor.c $(INCLIB)/servidor.h
	gcc $(CCFLAGS) -I$(INCLIB) -c $(SRCLIB)/servidor.c

http.o:	$(SRCLIB)/http.c $(INCLIB)/http.h
	gcc $(CCFLAGS) -I$(INCLIB) -c $(SRCLIB)/http.c

picohttpparser.o: $(SRCLIB)/picohttpparser.c $(INCLIB)/picohttpparser.h
	gcc $(CCFLAGS) -I$(INCLIB) -c $(SRCLIB)/picohttpparser.c

# Librerias
$(LIBS)/libservidor.a: servidor.o
	ar -rv $(LIBS)/libservidor.a servidor.o

$(LIBS)/libhttp.a: http.o
	ar -rv $(LIBS)/libhttp.a http.o

$(LIBS)/libparser.a: picohttpparser.o
	ar -rv $(LIBS)/libparser.a picohttpparser.o

# Limpiar el directorio
clean:
	rm -f *.o server
