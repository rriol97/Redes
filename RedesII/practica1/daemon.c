#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <math.h>

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

int main (){
	demonizar();
	return 0;
}