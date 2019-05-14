/***************************************************************************
* servidor.h
*
* Declaraciones y cabeceras para servidor.h
* 
* Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
***************************************************************************/

#ifndef __SERVIDOR_H
#define __SERVIDOR_H

#include "http.h"


/* Definicion de valores constantes */
#define ERROR -1
#define OK 0
#define SERVER_CONF "server.conf"

/* Declaracion de funciones */
int iniciar_servidor();
int demonizar (char *servicio);
int procesar_fichero_conf (char *f);
char * get_server_root ();
char * get_server_signature ();

#endif