/***************************************************************************
* servidor.c
*
* Implementacion de las funciones para el manejo de sockets y 
* todo lo relacionado con poner en marcha nuestro servidor 
*
* Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
***************************************************************************/

#include "servidor.h"

/* Definicion de estructuras */
typedef struct {
	char server_root[TAM];
	int max_clients;
	int listen_port;
	char server_signature[TAM];
} Configuracion;


/* Variables globales */
Configuracion conf;

/* Definicion de funciones */
/********
* FUNCIÓN: int iniciar_servidor() 
* ARGS_IN:  
* DESCRIPCIÓN: Funcion que inicializa el servidor 
* ARGS_OUT: int - el descriptor del socket inicializado
*********/
int iniciar_servidor() {
	int sockval;
	struct sockaddr_in Direccion;

	syslog(LOG_INFO, "Creando socket");
	/* Llamamos a socket con el dominio AF_INET para usar los protocolos ARPA, tipo SOCK_STREAM para indicar TCP, y protocolo a 0 */
	if ( (sockval = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		syslog(LOG_ERR, "Error creando socket");
		exit(EXIT_FAILURE);
	}

	Direccion.sin_family=AF_INET; /* Familia TCP/IP */
	Direccion.sin_port=htons(conf.listen_port); /* Asignando puerto */
	Direccion.sin_addr.s_addr=htonl(INADDR_ANY); /* Aceptar todas las direcciones */
	bzero(( void * )&(Direccion.sin_zero), 8);

	syslog(LOG_INFO, "Enlazando socket");
	/* Llamamos a bind con el descriptor de fichero socket devuelto anteriormente, la estructura antes rellena y su tamanio */
	if (bind (sockval, ( struct sockaddr * )&Direccion, sizeof (Direccion))<0) {
		syslog(LOG_ERR, "Error enlazando socket");
		exit(EXIT_FAILURE);
	}

	syslog(LOG_INFO, "Escuchando conexiones");
	/* Llamamos a listen con el descriptor de fichero de socket devuelto anteriormente 
	   y el numero de conexiones permitidas que aparece en el fichero de configuracion */
	if (listen (sockval, conf.max_clients)<0){
		syslog(LOG_ERR, "Error escuchando");
		exit(EXIT_FAILURE);
	}

	return sockval;
}



/********
* FUNCIÓN: int demonizar (char *servicio) 
* ARGS_IN:  char *servicio: nombre del servidor
* DESCRIPCIÓN: Funcion que demoniza el proceso servidor 
* ARGS_OUT: int - OK si todo ha ido bien, ERROR si no
*********/
int demonizar (char *servicio){
	pid_t pid;

	pid = fork(); /* Creamos un hijo del proceso */
	if (pid < 0) exit (EXIT_FAILURE);
	if (pid > 0) exit (EXIT_SUCCESS); /* Termina con el proceso padre */

	umask(0); /*Establece los permisos para los ficheros creados por este proceso 
			   No anulamos permisos de otros, grupos o usuarios */

	/* Como corre en segundo plano queremos que las acciones queden registradas en ficheros logs */
	setlogmask (LOG_UPTO (LOG_INFO)); 
	openlog (servicio, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL3);
	syslog (LOG_INFO, "Iniciando nuevo servidor");

	if (setsid < 0) { /* Hacemos que este proceso sea el lider de la nueva sesion */
		syslog(LOG_ERR, "Error creando el SID para el hijo");
		return ERROR;
	}

	if ((chdir ("/")) < 0) { /* Cambiamos al directorio raiz */
		syslog (LOG_ERR, "Error cambiando el directorio = \"/\"");
		return ERROR;
	}

	/* Cerramos descriptores ficheros, stdin ,stdout, stderr */
	syslog (LOG_INFO, "Cerramos los descriptores de ficheros");
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	return OK;

} 



/********
* FUNCIÓN: int procesar_fichero_conf (char *f) 
* ARGS_IN:  char *f: nombre del fichero de configuracion del servidor
* DESCRIPCIÓN: Funcion que parsea el fichero de configuracion	
* ARGS_OUT: int - OK 
*********/
int procesar_fichero_conf (char *f){
	char *server_root = NULL;
	char *server_signature = NULL;
	long int max_clients, listen_port;
	cfg_t *cfg;

	/* Valores por defecto */
	conf.max_clients = 1;
	conf.listen_port = 8080;
	strcpy(conf.server_root, "/");
	strcpy(conf.server_signature, "Generic Server");

	cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("server_root", &server_root),
		CFG_SIMPLE_INT("max_clients", &max_clients),
		CFG_SIMPLE_INT("listen_port", &listen_port),
		CFG_SIMPLE_STR("server_signature", &server_signature),
		CFG_END()
	}; 

	cfg = cfg_init(opts, 0);
	cfg_parse(cfg, f);

	conf.max_clients = max_clients;
	conf.listen_port = listen_port;
	strcpy(conf.server_root, server_root);
	strcpy(conf.server_signature, server_signature);

	cfg_free(cfg);
	free(server_root);
	free(server_signature);
	
	return OK;
}


/********
* FUNCIÓN: char * get_server_root ()
* ARGS_IN:  
* DESCRIPCIÓN: Funcion que devuelve la ruta del servidor
* ARGS_OUT: char * - ruta del servidor
*********/
char * get_server_root () {
	return conf.server_root;
}



/********
* FUNCIÓN: char * get_server_signature ()
* ARGS_IN:  
* DESCRIPCIÓN: Funcion que devuelve el nombre del servidor
* ARGS_OUT: char * - nombre del servidor
*********/
char * get_server_signature () {
	return conf.server_signature;
}


