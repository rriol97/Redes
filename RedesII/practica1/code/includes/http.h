/***************************************************************************
* http.h
*
* Declaracion de las funciones referentes al protocolo HTTP  
*
* Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
***************************************************************************/

#ifndef __HTTP_H
#define __HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <signal.h>
#include <time.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <sys/syscall.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdarg.h>
#include <confuse.h>
#include "picohttpparser.h"


/* Definimos los posibles errores antes un peticion del cliente*/
typedef enum {
	OK = 200,
	BAD_REQUEST = 400,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	INTERNAL_SERVER_ERROR = 500,
	METHOD_NOT_IMPLEMENTED = 501
} Status_Code;

/* Definicion de valores constantes */
#define TAM 8192
#define FICHERO_SALIDA "/scripts/salida.html"
#define FICHERO_SIN_ARGS "/scripts/index.html"
#define INDEX "index.html"

/* Declaracion de funciones */
char * extraer_tipo_fichero (char *str);
Status_Code procesar_peticion(int connfd, char *server_root, char *server_signature);
Status_Code procesar_peticion_get (int connfd, char *real_path, char *content_type, char *fecha, char *ult_mod, int total_bytes, char *args, char *server_root, char *server_signature);
Status_Code procesar_peticion_head (int connfd, char *real_path, char *content_type, char *fecha, char *ult_mod, int total_bytes, char *server_root, char *server_signature);
Status_Code procesar_peticion_post (int connfd, char *buffer, int salto, char *real_path, char *content_type, char *fecha, char *ult_mod, int total_bytes, char *server_root, char *server_signature);
Status_Code procesar_peticion_options (int connfd, char *content_type, char *fecha, int total_bytes, char *server_signature);
int formar_respuesta(char *respuesta, char *method, Status_Code status_code, char *status_msg, char *fecha, char *servidor, char *ult_mod, int total_bytes, char *content_type);
FILE* ejecutar_script(char *content_type, char *path, char *argumentos, char *server_root);

#endif