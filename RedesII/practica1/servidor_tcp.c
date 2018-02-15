/* Servidor iterativo TCP 
*
*  Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
*/

#include "servidor_tcp.h"

/* Variables globales */
int fin = 0; /* Indica cuando se cierra el servidor */
int sockval; /* Descriptor del socket */

/***************************************/

/* Definicion de funciones */
void handleSignal(int nsignal){
	printf("Control C pulsado\n");
	fin = 1;
	close(sockval); /* Cerramos el socket del servidor */

	printf("Conexion finalizada\n");
	syslog(LOG_INFO, "Conexion finalizada");
	exit(EXIT_SUCCESS);
}


int main(int argc, char **argv) {
	int connfd;
	socklen_t clilen;
	struct sockaddr cliaddr;
	char buffer[TAM];

	if (signal(SIGINT,handleSignal)==SIG_ERR) {
		printf("Error: Fallo al capturar la senal SIGINT.\n");
		exit(EXIT_FAILURE);
	}

	if (demonizar() == ERROR) {
		syslog(LOG_ERR, "Error demonizando");
		exit(EXIT_FAILURE);
	}

	/* Iniciamos el servidor llamando a socket, bind y listen, y actualizamos el Log del sistema */
	sockval = iniciar_servidor();

	while (fin == 0) {
		connfd = accept(sockval, &cliaddr, &clilen); /* Aceptamos conexiones de clientes */
		procesar_peticion(connfd, buffer); /* procesado de la peticion del cliente */
		memset(buffer, 0, sizeof(buffer));

		close(connfd);	/* Cerramos la conexion con el cliente desde el servidor */
	}

	

	exit(EXIT_SUCCESS);
}

int iniciar_servidor() {
	int sockval;
	struct sockaddr_in Direccion;

	syslog(LOG_INFO, "Creando socket");
	if ( (sockval = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		syslog(LOG_ERR, "Error creando socket");
		exit(EXIT_FAILURE);
	}

	Direccion.sin_family=AF_INET; /* Familia TCP/IP */
	Direccion.sin_port=htons(NFC_SERVER_PORT); /* Asignando puerto */
	Direccion.sin_addr.s_addr=htonl(INADDR_ANY); /* Aceptar todas las direcciones */
	bzero(( void * )&(Direccion.sin_zero), 8);

	syslog(LOG_INFO, "Enlazando socket");
	if (bind (sockval, ( struct sockaddr * )&Direccion, sizeof (Direccion))<0) {
		syslog(LOG_ERR, "Error enlazando socket");
		exit(EXIT_FAILURE);
	}

	syslog(LOG_INFO, "Escuchando conexiones");
	if (listen (sockval, MAX_CONNECTIONS)<0){
		syslog(LOG_ERR, "Error escuchando");
		exit(EXIT_FAILURE);
	}

	return sockval;
}

void procesar_peticion(int connfd, char *buffer) {

	syslog(LOG_INFO, "Nuevo acceso");

	if (recv(connfd, buffer, sizeof(buffer)*TAM, 0) == -1) {
		syslog(LOG_ERR, "Error recibiendo datos");
		exit(errno);
	}

	/*fprintf(stdout, "%s", buffer);	*//* Mostramos el mensaje introducido por el cliente */
	syslog(LOG_INFO, "%s", buffer);
	syslog(LOG_INFO, "Saliendo del servicio");

	return;
}


int demonizar (void){
	pid_t pid;

	pid = fork(); /** Creamos un hijo del proceso */
	if (pid < 0) exit (EXIT_FAILURE);
	if (pid > 0) exit (EXIT_SUCCESS); /* Termina con el proceso padre */

	umask(0); /*Establece los permisos para los ficheros creados por este proceso 
			   No anulamos permisos de otros, grupos o usuarios */

	/** Como corre en segundo plano queremos que las acciones queden registradas en ficheros logs */
	setlogmask (LOG_UPTO (LOG_INFO)); 
	openlog ("Mensajes del sistema", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL3);
	syslog (LOG_ERR, "Iniciando nuevo servidor");

	if (setsid < 0) { /** Hacemos que este proceso sea el lider de la nueva sesion */
		syslog(LOG_ERR, "Error creando el SID para el hijo");
		return ERROR;
	}

	if ((chdir ("/")) < 0) { /** Cambiamos al directorio raiz */
		syslog (LOG_ERR, "Error cambiando el directorio = \"/\"");
		return ERROR;
	}

	/** Cerramos descriptores ficheros, stdin ,stdout, stderr */
	syslog (LOG_INFO, "Cerramos los descriptores de ficheros");
	close (STDIN_FILENO);
	close (STDOUT_FILENO);
	close (STDERR_FILENO);

	return OK;

}

