/***************************************************************************
* main.c
*
* Pone un servidor HTTP en marcha y procesa peticiones de clientes 
* mediante multihilo 
*
* Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
***************************************************************************/

#include "servidor.h"


/* Definicion de estructuras */
typedef struct {
	int connfd;
} Argumentos;


/* Variables globales */
int sockval; /* Descriptor del socket */

/* Definicion de funciones */
/********
* FUNCIÓN: void handleSignal(int nsignal)
* ARGS_IN:  int nsignal: tipo de senial a asignar el manejador
* DESCRIPCIÓN: Funcion que carga el manejador de la senial nsignal	
* ARGS_OUT: void
*********/
void handleSignal(int nsignal){

	syslog(LOG_INFO, "SIGINT recibido");
	close(sockval); /* Cerramos el socket del servidor */

	syslog(LOG_INFO, "Conexion finalizada");
	exit(EXIT_SUCCESS);
}



/********
* FUNCIÓN: void * accion_hilo(void *args) 
* ARGS_IN:  void *args: direccion de los argumentos para el hilo
* DESCRIPCIÓN: Funcion que hace que cada hilo procese una peticion 
* ARGS_OUT: void *
*********/
void * accion_hilo(void *args) {
	Argumentos *aux;

	if (args == NULL) {
		return NULL;
	}

	aux = (Argumentos *)args;

	syslog(LOG_INFO, "Hilo con tid %ld procesando peticion", syscall(SYS_gettid));
	pthread_detach(pthread_self());

	/* Procesado de la peticion del cliente */
	procesar_peticion(aux->connfd, get_server_root(), get_server_signature());

	/* En caso contrario, o ha ido bien o si ha habido error, ya se ha notificado 
	al cliente que es por su culpa, asi que el servidor continua activo */
	syslog(LOG_INFO, "Saliendo del servicio");

	close(aux->connfd);	/* Cerramos la conexion con el cliente desde el hilo */

	return NULL;
}



/********
* FUNCIÓN: int main()
* ARGS_IN:  
* DESCRIPCIÓN: Funcion principal main 
* ARGS_OUT: exit(EXIT_SUCCESS) si todo va bien, 
*           o exit(EXIT_FAILURE) en caso de error
*********/
int main() {
	socklen_t clilen;
	pthread_t tid; 
	struct sockaddr cliaddr;
	Argumentos args;

	/* Establecemos un manejador para la senial SIGINT */
	if (signal(SIGINT,handleSignal)==SIG_ERR) {
		printf("Error: Fallo al capturar la senal SIGINT.\n");
		exit(EXIT_FAILURE);
	}

	/* Cargamos los datos del fichero de configuracion */
	procesar_fichero_conf(SERVER_CONF);

	/* Ponemos el servidor en modo demonio */
	if (demonizar(get_server_signature()) == ERROR) {
		syslog(LOG_ERR, "Error demonizando");
		exit(EXIT_FAILURE);
	}

	/* Iniciamos el servidor llamando a socket, bind y listen, y actualizamos el Log del sistema */
	sockval = iniciar_servidor();

	clilen = sizeof(cliaddr);
	while (1) {
		args.connfd = accept(sockval, &cliaddr, &clilen); /* Aceptamos conexiones de clientes */
		pthread_create(&tid, NULL, &accion_hilo, (void *) &args); /* Lanzamos un hilo por cada conexion */
	} 

	

	exit(EXIT_SUCCESS);
}






