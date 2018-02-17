/***************************************************************************
 servidor_tcp.h
 Definiciones y cabeceras para servidor_tcp.h
 
 Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
***************************************************************************/

#ifndef __SERVIDOR_TCP_H
#define __SERVIDOR_TCP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <errno.h>
#include <math.h>

/* Definicion de valores constantes */
#define TAM 1024
#define MAX_CONNECTIONS 3
#define NFC_SERVER_PORT 2000
#define OK 1
#define ERROR 0

/* Declaracion de funciones */
int iniciar_servidor();
void procesar_peticion(int connfd, char *buffer);
int demonizar (void);

#endif