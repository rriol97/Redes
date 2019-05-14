/***************************************************************************
* http.c
*
* Implementacion de las funciones referentes al protocolo HTTP
*
* Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
***************************************************************************/

#include "http.h"

/* Definicion de funciones */
/********
* FUNCIÓN: char * extraer_tipo_fichero (char *str)
* ARGS_IN:  char *str: cadena con la ruta completa del fichero
* DESCRIPCIÓN: Funcion que devuelve la extension de un fichero dentro de las 	
* 			   aceptadas por el servidor, o la cadena vacia si no es una 	
* 			   extension valida.	
* ARGS_OUT: char * - devuelve una cadena con el MIME de la extension del 
* 					 fichero, "/" si es un directorio o la cadena vacia
*					 en caso de error.
*********/
char * extraer_tipo_fichero (char *str) {
    char *token, aux[TAM] = "";  

    strcpy(aux, str);

	token = strtok(aux, ".");

	if (strlen(token)+1 >= strlen(str)) {
		if (token[strlen(token)-1] == '/') {
			return "/";
		}
		return "";
	}

	token = strtok(NULL, "?");

    if (strcmp(token, "txt") == 0){
   		return "text/plain";
    } else if (strcmp(token, "html") == 0 || strcmp(token, "htm") == 0){
   		return "text/html";
    } else if (strcmp(token, "gif") == 0){
   		return "image/gif";
    } else if (strcmp(token, "jpeg") == 0 || strcmp(token, "jpg") == 0){
   		return "image/jpeg";
   	} else if (strcmp(token, "mpeg") == 0 || strcmp(token, "mpg") == 0){
   		return "video/mpeg";
    } else if (strcmp(token, "doc") == 0 || strcmp(token, "docx") == 0){
   		return "application/msword";
    } else if (strcmp(token, "pdf") == 0){
   		return "application/pdf"; 
    } else if (strcmp(token, "py") == 0){
   		return "application/py"; 
    } else if (strcmp(token, "php") == 0){
   		return "application/php"; 
    } else if (strcmp(token, "ico") == 0){
   		return "image/x-icon"; 
    } else {
   		return "";
    }
}



/********
* FUNCIÓN: Status_Code procesar_peticion(int connfd, char *buffer, char *server_root, char *server_signature)
* ARGS_IN:  int connfd: descriptor de la conexion
*			char *server_root: ruta del servidor
*			char *server_signature: nombre del servidor 
* DESCRIPCIÓN: Funcion que procesa la peticion del cliente
* ARGS_OUT: Status_Code - codigo de retorno correspondiente al 
*						  resultado de la peticion
*********/
Status_Code procesar_peticion(int connfd, char *server_root, char *server_signature) {
	const char *method;
	const char *path;
	char ubicacion[TAM] = "", mensaje[TAM] = "", content_type[TAM] = "";
	char argumentos[TAM] = "", auxiliar[TAM] = "", buffer[TAM] = "";
    size_t method_len;
    size_t path_len, last_len = 0, buff_len = 0;
    int minor_version, retorno, ret, total_bytes = 0;
    struct phr_header headers[20];
    size_t num_headers;
    time_t t;
    char ult_mod[TAM], fecha[TAM];
    char *aux;
	struct stat c;
	int i, salto, descr;
	Status_Code code;

	syslog(LOG_INFO, "Nuevo acceso");

	while (1){
		
		while ((retorno = recv(connfd, buffer + buff_len, sizeof(buffer)*TAM - buff_len, 0)) == -1 && errno == EINTR);

		if (retorno < 0){
			syslog(LOG_ERR, "Error recibiendo datos del cliente");
			return INTERNAL_SERVER_ERROR;
		}

		last_len = buff_len;
		buff_len += retorno;

		num_headers = sizeof(headers) / sizeof(headers[0]);
		ret = phr_parse_request(buffer, buff_len, &method, &method_len, &path, &path_len,  \
						&minor_version, headers, &num_headers, last_len);

		if (ret > 0){
			break; /* Parseada la cabecera HTTP entera correctamente*/
		} else if (ret == -1) {
			syslog(LOG_ERR, "Error parseando datos de la peticion");
			return INTERNAL_SERVER_ERROR;

		} else {
			if (buff_len > TAM){
				syslog (LOG_ERR, "Se ha desbordado el buffer");
				return INTERNAL_SERVER_ERROR;
			}
		}
	}

	t = time(NULL);
	aux = strtok(ctime(&t), "\n");
	strcpy(fecha, aux);
	strcat(fecha, "\r\n");

	/* Comprobamos si la peticion es correcta segun su version de HTTP */
	if (minor_version == 1 && strncmp(headers[0].name, "Host", headers[0].name_len)) {
		syslog(LOG_ERR, "Peticion incorrecta");
		strcpy(ubicacion, server_root);
		strcat(ubicacion, "/errors/400_error.html");
		stat(ubicacion, &c);
		total_bytes = c.st_size;
		retorno = formar_respuesta(mensaje, "", BAD_REQUEST, "Bad Request", fecha, server_signature, ult_mod, total_bytes, "text/html");
		descr = open (ubicacion, O_RDONLY);  
		if (descr < 0) {			/* El fichero de error sabemos que existe, asi que este error seria interno del open */
			syslog(LOG_ERR, "Error interno");
			return INTERNAL_SERVER_ERROR;
		}
		/* Enviamos la cabecera con la respuesta de error */
		send(connfd, mensaje, retorno, 0);
		/* Enviamos los datos del fichero html de error */ 
		while (retorno > 0) {
			memset(auxiliar, 0, sizeof(auxiliar));
			retorno = read(descr, auxiliar, sizeof(auxiliar));
			if (retorno == 0) {	/* Final del fichero */
				strcat(auxiliar, "\n");
				send(connfd, auxiliar, 1, 0);
				break;
			}
			send(connfd, auxiliar, retorno, 0);
		}
		close(descr);
		return BAD_REQUEST;
	}

	/* Comprobamos si la URL introducida empieza por '/' o no y si es unicamente '/' */
	if ( strncmp(path, "/", 1) && strncmp(method, "OPTIONS", method_len) && (strncmp(path, "*", 1) || path_len > 1) ) {
		syslog(LOG_ERR, "Peticion incorrecta");
		strcpy(ubicacion, server_root);
		strcat(ubicacion, "/errors/400_error.html");
		stat(ubicacion, &c);
		total_bytes = c.st_size;
		retorno = formar_respuesta(mensaje, "", BAD_REQUEST, "Bad Request", fecha, server_signature, ult_mod, total_bytes, "text/html");
		descr = open (ubicacion, O_RDONLY);  
		if (descr < 0) {			/* El fichero de error sabemos que existe, asi que este error seria interno del open */
			syslog(LOG_ERR, "Error interno");
			return INTERNAL_SERVER_ERROR;
		}
		/* Enviamos la cabecera con la respuesta de error */
		send(connfd, mensaje, retorno, 0);
		/* Enviamos los datos del fichero html de error */ 
		while (retorno > 0) {
			memset(auxiliar, 0, sizeof(auxiliar));
			retorno = read(descr, auxiliar, sizeof(auxiliar));
			if (retorno == 0) {	/* Final del fichero */
				strcat(auxiliar, "\n");
				send(connfd, auxiliar, 1, 0);
				break;
			}
			send(connfd, auxiliar, retorno, 0);
		}
		close(descr);
		return BAD_REQUEST;
	} 

	/* Separamos URL de argumentos */
	strncat(auxiliar, path, path_len);
	aux = strtok(auxiliar, "?");
	strcpy(ubicacion, server_root);
	strcat(ubicacion, aux);
	if (strlen(aux)+1 < path_len) {
		strcpy(argumentos, strtok(NULL, "?"));
	} 

	/* Separamos el caso OPTIONS *, del resto */
	if (strncmp(path, "*", path_len) || strncmp(method, "OPTIONS", method_len)) {
		strcpy(content_type, extraer_tipo_fichero (ubicacion));
		/* Comprobamos si la extension es valida */
		if (strcmp(content_type, "") == 0){	
			syslog(LOG_ERR, "Tipo de archivo no valido\n");
			strcpy(ubicacion, server_root);
			strcat(ubicacion, "/errors/400_error.html");
			stat(ubicacion, &c);
			total_bytes = c.st_size;
			retorno = formar_respuesta(mensaje, "", BAD_REQUEST, "Bad Request", fecha, server_signature, ult_mod, total_bytes, "text/html");
			descr = open (ubicacion, O_RDONLY);  
			if (descr < 0) {			/* El fichero de error sabemos que existe, asi que este error seria interno del open */
				syslog(LOG_ERR, "Error interno");
				return INTERNAL_SERVER_ERROR;
			}
			/* Enviamos la cabecera con la respuesta de error */
			send(connfd, mensaje, retorno, 0);
			/* Enviamos los datos del fichero html de error */ 
			while (retorno > 0) {
				memset(auxiliar, 0, sizeof(auxiliar));
				retorno = read(descr, auxiliar, sizeof(auxiliar));
				if (retorno == 0) {	/* Final del fichero */
					strcat(auxiliar, "\n");
					send(connfd, auxiliar, 1, 0);
					break;
				}
				send(connfd, auxiliar, retorno, 0);
			}
			close(descr);
			return BAD_REQUEST;
		}
		/* La extension es valida asi que obtenemos campos importantes del archivo */
		stat(ubicacion, &c);
		total_bytes = c.st_size;
		aux = strtok(ctime(&c.st_mtime), "\n");
		strcpy(ult_mod, aux);
		strcat(ult_mod, "\r\n");
	} else { /* Caso "OPTIONS *" */
		strcpy(content_type, "*");  
		total_bytes = 0;
	}
	

	/* Procesamos el tipo de peticion correspondiente segun el metodo */
	if (strncmp(method, "GET", method_len) == 0 && method_len == strlen("GET")){
		code = procesar_peticion_get (connfd, ubicacion, content_type, fecha, ult_mod, total_bytes, argumentos, server_root, server_signature);
	} else if (strncmp(method,"POST", method_len) == 0 && method_len == strlen("POST")){
		/* Calculamos todo lo que ocupan las cabeceras de la peticion para saltarnoslas luego y llegar al cuerpo, de donde leer el payload */
		salto = method_len + path_len + strlen("HTTP/1.X\r\n") + 2; /* Metodo + path + HTTP/1.X\r\n + los dos espacios antes y despues del path */
		for (i=0; i < num_headers; i++) {
			salto += headers[i].name_len + 2 + headers[i].value_len + 2;	/* Nombre de cada cabecera, los dos puntos, el espacio, el valor y el \r\n */
		}
		salto += 2;  /* El \r\n extra que separa cabecera de cuerpo */
		code = procesar_peticion_post (connfd, buffer, salto, ubicacion, content_type, fecha, ult_mod, total_bytes, server_root, server_signature);
	} else if (strncmp(method, "OPTIONS", method_len) == 0 && method_len == strlen("OPTIONS")){
		code = procesar_peticion_options(connfd, content_type, fecha, total_bytes, server_signature);
	} else if (strncmp(method, "HEAD", method_len) == 0 && method_len == strlen("HEAD")){
		code = procesar_peticion_head(connfd, ubicacion, content_type, fecha, ult_mod, total_bytes, server_root, server_signature);
	} else {
		strcpy(ubicacion, server_root);
		strcat(ubicacion, "/errors/501_error.html");
		stat(ubicacion, &c);
		total_bytes = c.st_size;
		retorno = formar_respuesta(mensaje, "", METHOD_NOT_IMPLEMENTED, "Method Not Implemented", fecha, server_signature, ult_mod, total_bytes, "text/html");
		descr = open (ubicacion, O_RDONLY);  
		if (descr < 0) {			/* El fichero de error sabemos que existe, asi que este error seria interno del open */
			syslog(LOG_ERR, "Error interno");
			return INTERNAL_SERVER_ERROR;
		}
		/* Enviamos la cabecera con la respuesta de error */
		send(connfd, mensaje, retorno, 0);
		/* Enviamos los datos del fichero html de error */ 
		while (retorno > 0) {
			memset(auxiliar, 0, sizeof(auxiliar));
			retorno = read(descr, auxiliar, sizeof(auxiliar));
			if (retorno == 0) {	/* Final del fichero */
				strcat(auxiliar, "\n");
				send(connfd, auxiliar, 1, 0);
				break;
			}
			send(connfd, auxiliar, retorno, 0);
		}
		close(descr);
		code = METHOD_NOT_IMPLEMENTED;
	}

	return code; /* Para que el servidor sepa que ha ocurrido */
}



/********
* FUNCIÓN: Status_Code procesar_peticion_get (int connfd, char *real_path, char *content_type, char *fecha,  \
												char *ult_mod, int total_bytes, char *args, char *server_root, char *server_signature)
* ARGS_IN:  int connfd: descriptor de la conexion
*			char *real_path: ruta del fichero que se solicita
*			char *content_type: MIME del fichero que se solicita
*			char *fecha: fecha en la que se realiza la peticion 
*			char *ult_mod: fecha de ultima modificacion del fichero que se solicita
*			int total_bytes: numero de bytes del fichero que se solicita
*			char *args: argumentos introducidos en la peticion
*			char *server_root: path del servidor
*			char *server_signature: nombre del servidor 
* DESCRIPCIÓN: Funcion que se encarga de dar respuesta a la peticion GET del cliente
* ARGS_OUT: Status_Code - codigo de retorno correspondiente al 
*						  resultado de la peticion
*********/
Status_Code procesar_peticion_get (int connfd, char *real_path, char *content_type, char *fecha,  \
									char *ult_mod, int total_bytes, char *args, char *server_root, char *server_signature){
	int descr, num_bytes;
	char mensaje[TAM] = "", path[TAM] = ""; 
	FILE *p_ret;
	char *aux;
	struct stat c;

	/* Comprobamos si la peticion es sobre un directorio y no un fichero */
	if (strcmp(content_type, "/") == 0) {
		syslog(LOG_INFO, "Buscando index.html");
		strcpy(path, real_path);
		strcat(path, INDEX);
		stat(path, &c);
		total_bytes = c.st_size;
		num_bytes = formar_respuesta(mensaje, "GET", OK, "OK", fecha, server_signature, ult_mod, total_bytes, "text/html");
		descr = open (path, O_RDONLY);
		/* Control de error sobre la apertura del fichero */
		if (descr < 0) {
			syslog(LOG_ERR, "Error abriendo el archivo index");
			memset(path, 0, sizeof(path));
			strcpy(path, server_root);
			strcat(path, "/errors/404_error.html");
			stat(path, &c);
			total_bytes = c.st_size;
			memset(mensaje, 0, sizeof(mensaje));
			num_bytes = formar_respuesta(mensaje, "", NOT_FOUND, "Not Found", fecha, server_signature, ult_mod, total_bytes, "text/html");
			descr = open (path, O_RDONLY);
			if (descr < 0) {			/* El fichero de error sabemos que existe, asi que este error seria interno del open */
				syslog(LOG_ERR, "Error interno");
				return INTERNAL_SERVER_ERROR;
			}
			/* Enviamos la cabecera con la respuesta de error */
			send(connfd, mensaje, num_bytes, 0);
			/* Enviamos los datos del fichero html de error */ 
			while (num_bytes > 0) {
				memset(mensaje, 0, sizeof(mensaje));
				num_bytes = read(descr, mensaje, sizeof(mensaje));
				if (num_bytes == 0) {	/* Final del fichero */
					strcat(mensaje, "\n");
					send(connfd, mensaje, 1, 0);
					break;
				}
				send(connfd, mensaje, num_bytes, 0);
			}
			close(descr);
			return NOT_FOUND;
		}


		/* Enviamos la cabecera con la respuesta del index */
		send(connfd, mensaje, num_bytes, 0);
		/* Enviamos los datos del fichero index */ 
		while (num_bytes > 0) {
			memset(mensaje, 0, sizeof(mensaje));
			num_bytes = read(descr, mensaje, sizeof(mensaje));
			if (num_bytes == 0) {	/* Final del fichero */
				strcat(mensaje, "\n");
				send(connfd, mensaje, 1, 0);
				break;
			}
			send(connfd, mensaje, num_bytes, 0);
		}
		close(descr);
		return OK;
	}

	/* Vamos a abrir el fichero para comprobar si existe realmente */
	descr = open (real_path, O_RDONLY);
	/* Control de error sobre la apertura del fichero */
	if (descr < 0) {
		syslog(LOG_ERR, "Error abriendo el archivo");
		strcpy(path, server_root);
		strcat(path, "/errors/404_error.html");
		stat(path, &c);
		total_bytes = c.st_size;
		num_bytes = formar_respuesta(mensaje, "", NOT_FOUND, "Not Found", fecha, server_signature, ult_mod, total_bytes, "text/html");
		descr = open (path, O_RDONLY);
		if (descr < 0) {			/* El fichero de error sabemos que existe, asi que este error seria interno del open */
			syslog(LOG_ERR, "Error interno");
			return INTERNAL_SERVER_ERROR;
		}
		/* Enviamos la cabecera con la respuesta de error */
		send(connfd, mensaje, num_bytes, 0);
		/* Enviamos los datos del fichero html de error */ 
		while (num_bytes > 0) {
			memset(mensaje, 0, sizeof(mensaje));
			num_bytes = read(descr, mensaje, sizeof(mensaje));
			if (num_bytes == 0) {	/* Final del fichero */
				strcat(mensaje, "\n");
				send(connfd, mensaje, 1, 0);
				break;
			}
			send(connfd, mensaje, num_bytes, 0);
		}
		close(descr);
		return NOT_FOUND;
	} else {
		/* En caso de que exista, lo cerramos */
		close(descr);
	}

	/* En caso de que el fichero sea un script python o php */
	if (strcmp(content_type, "application/py") == 0 || strcmp(content_type, "application/php") == 0) {
		/* Caso en el que no hay argumentos */
		if (strcmp(args, "") == 0) {
			syslog(LOG_ERR, "No hay argumentos para el script");
			strcpy(path, server_root);
			strcat(path, FICHERO_SIN_ARGS);
			stat(path, &c);
			total_bytes = c.st_size;		
			aux = strtok(ctime(&c.st_mtime), "\n");
			bzero(ult_mod, sizeof(char)*TAM);
			strcpy(ult_mod, aux);
			strcat(ult_mod, "\r\n");

			num_bytes = formar_respuesta(mensaje, "GET", OK, "OK", fecha, server_signature, ult_mod, total_bytes, "text/html");
			descr = open (path, O_RDONLY);
			if (descr < 0) {			/* El fichero del formulario sabemos que existe, asi que este error seria interno del open */
				syslog(LOG_ERR, "Error interno");
				return INTERNAL_SERVER_ERROR;
			}
			/* Enviamos la cabecera con la respuesta de error */
			send(connfd, mensaje, num_bytes, 0);
			/* Enviamos los datos del formulario html */ 
			while (num_bytes > 0) {
				memset(mensaje, 0, sizeof(mensaje));
				num_bytes = read(descr, mensaje, sizeof(mensaje));
				if (num_bytes == 0) {	/* Final del fichero */
					strcat(mensaje, "\n");
					send(connfd, mensaje, 1, 0);
					break;
				}
				send(connfd, mensaje, num_bytes, 0);
			}
			close(descr);
			return OK;
		}

		/* Preparamos la llamada al script y la realizamos */
		p_ret = ejecutar_script(content_type, real_path, args, server_root);
		/* Comprobamos que se haya ejecutado el script correctamente */
		if (p_ret == NULL){
			syslog(LOG_ERR, "Error ejecutando el script (GET)");
			return INTERNAL_SERVER_ERROR;
		}
		/* Cerramos la tuberia formada por el popen */
		pclose(p_ret);

		/* Si llegamos hasta aqui, el script ha ido bien asi que debemos leer del fichero de salida, el cual sabemos que existe */
 		bzero(real_path, sizeof(char)*TAM);
 		strcpy(real_path, server_root);
		strcat(real_path, FICHERO_SALIDA);
		/* La extension del fichero de salida se ha decidido que siempre sea HTML */
		bzero(content_type, sizeof(char)*TAM);
		strcpy(content_type, "text/html");
		/* Obtenemos campos importantes del archivo */
		stat(real_path, &c);
		total_bytes = c.st_size;
		aux = strtok(ctime(&c.st_mtime), "\n");
		bzero(ult_mod, sizeof(char)*TAM);
		strcpy(ult_mod, aux);
		strcat(ult_mod, "\r\n");
		
	}	
	
	/* Abrimos el fichero que se nos pide */
	descr = open (real_path, O_RDONLY);
	/* Control de error sobre la apertura del fichero */
	if (descr < 0) {	/* El fichero ya sabemos que existe, asi que este error seria interno del open */
		syslog(LOG_ERR, "Error interno");
		return INTERNAL_SERVER_ERROR;
	}

	num_bytes = formar_respuesta(mensaje, "GET", OK, "OK", fecha, server_signature, ult_mod, total_bytes, content_type);

	/* Enviamos la cabecera con la respuesta de confirmacion */
	send(connfd, mensaje, num_bytes, 0);
	/* Enviamos los datos del fichero en cuestion */
	while (num_bytes > 0) {
		memset(mensaje, 0, sizeof(mensaje));
		num_bytes = read(descr, mensaje, sizeof(mensaje));
		if (num_bytes == 0) {	/* Final del fichero */
			strcat(mensaje, "\n");
			send(connfd, mensaje, 1, 0);
			break;
		}
		send(connfd, mensaje, num_bytes, 0);
	}
	close(descr);
	return OK;
}


/********
* FUNCIÓN: Status_Code procesar_peticion_head (int connfd, char *real_path, char *content_type, char *fecha, char *ult_mod, \
									int total_bytes, char *server_root, char *server_signature)
* ARGS_IN:  int connfd: descriptor de la conexion
*			char *real_path: ruta del fichero que se solicita
*			char *content_type: MIME del fichero que se solicita
*			char *fecha: fecha en la que se realiza la peticion 
*			char *ult_mod: fecha de ultima modificacion del fichero que se solicita
*			int total_bytes: numero de bytes del fichero que se solicita
*			char *server_root: path del servidor
*			char *server_signature: nombre del servidor 
* DESCRIPCIÓN: Funcion que se encarga de dar respuesta a la peticion HEAD del cliente
* ARGS_OUT: Status_Code - codigo de retorno correspondiente al 
*						  resultado de la peticion
*********/
Status_Code procesar_peticion_head (int connfd, char *real_path, char *content_type, char *fecha, char *ult_mod, \
									int total_bytes, char *server_root, char *server_signature){
	int descr, num_bytes;
	char mensaje[TAM] = "", path[TAM] = ""; 
	struct stat c;
	

	/* Vamos a abrir el fichero para comprobar si existe realmente */
	descr = open (real_path, O_RDONLY);
	/* Control de error sobre la apertura del fichero */
	if (descr < 0) {
		syslog(LOG_ERR, "Error abriendo el archivo");
		strcpy(path, server_root);
		strcat(path, "/errors/404_error.html");
		stat(path, &c);
		total_bytes = c.st_size;
		num_bytes = formar_respuesta(mensaje, "", NOT_FOUND, "Not Found", fecha, server_signature, ult_mod, total_bytes, "text/html");
		descr = open (path, O_RDONLY);
		if (descr < 0) {			/* El fichero de error sabemos que existe, asi que este error seria interno del open */
			syslog(LOG_ERR, "Error interno");
			return INTERNAL_SERVER_ERROR;
		}
		/* Enviamos la cabecera con la respuesta de error */
		send(connfd, mensaje, num_bytes, 0);
		/* Enviamos los datos del fichero html de error */ 
		while (num_bytes > 0) {
			memset(mensaje, 0, sizeof(mensaje));
			num_bytes = read(descr, mensaje, sizeof(mensaje));
			if (num_bytes == 0) {	/* Final del fichero */
				strcat(mensaje, "\n");
				send(connfd, mensaje, 1, 0);
				break;
			}
			send(connfd, mensaje, num_bytes, 0);
		}
		close(descr);
		return NOT_FOUND;
	} else {
		/* En caso de que exista, lo cerramos */
		close(descr);
	}

	num_bytes = formar_respuesta(mensaje, "HEAD", OK, "OK", fecha, server_signature, ult_mod, total_bytes, content_type);
	
	/* Enviamos la cabecera con la respuesta de confirmacion */
	send(connfd, mensaje, num_bytes, 0);
	
	return OK;
}


/********
* FUNCIÓN: Status_Code procesar_peticion_post (char *buffer, int salto, int connfd, char *real_path, char *content_type, \
									char *fecha, char *ult_mod, int total_bytes, char *server_root, char *server_signature)
* ARGS_IN:  int connfd: descriptor de la conexion
*			char *buffer: peticion del cliente
*			int salto: numero de bytes para pasar la cabecera de la peticion y llegar al cuerpo
*			char *real_path: ruta del fichero que se solicita
*			char *content_type: MIME del fichero que se solicita
*			char *fecha: fecha en la que se realiza la peticion 
*			char *ult_mod: fecha de ultima modificacion del fichero que se solicita
*			int total_bytes: numero de bytes del fichero que se solicita
*			char *server_root: path del servidor
*			char *server_signature: nombre del servidor 
* DESCRIPCIÓN: Funcion que se encarga de dar respuesta a la peticion POST del cliente
* ARGS_OUT: Status_Code - codigo de retorno correspondiente al 
*						  resultado de la peticion
*********/
Status_Code procesar_peticion_post (int connfd, char *buffer, int salto, char *real_path, char *content_type, \
									char *fecha, char *ult_mod, int total_bytes, char *server_root, char *server_signature){
	char aux[TAM] = "", ubicacion[TAM] = "";
	struct stat c;
    char *auxiliar;
	int descr, num_bytes;
	FILE *p_ret;

	/* En caso de que el fichero NO sea un script python o php */
	if (strcmp(content_type, "application/py") && strcmp(content_type, "application/php")) {
		strcpy(ubicacion, server_root);
		strcat(ubicacion, "/errors/405_error.html");
		stat(ubicacion, &c);
		total_bytes = c.st_size;
		num_bytes = formar_respuesta(aux, "", METHOD_NOT_ALLOWED, "Method Not Allowed", fecha, server_signature, ult_mod, total_bytes, "text/html");
		descr = open (ubicacion, O_RDONLY);
		if (descr < 0) {			/* El fichero de error sabemos que existe, asi que este error seria interno del open */
			syslog(LOG_ERR, "Error interno");
			return INTERNAL_SERVER_ERROR;
		}
		/* Enviamos la cabecera con la respuesta de error */
		send(connfd, aux, num_bytes, 0);
		/* Enviamos los datos del fichero html de error */ 
		while (num_bytes > 0) {
			memset(aux, 0, sizeof(aux));
			num_bytes = read(descr, aux, sizeof(aux));
			if (num_bytes == 0) {	/* Final del fichero */
				strcat(aux, "\n");
				send(connfd, aux, 1, 0);
				break;
			}
			send(connfd, aux, num_bytes, 0);
		}
		close(descr);
		return METHOD_NOT_ALLOWED;
	}

	/* Parseamos el body de la peticion POST del cliente */
	strcpy(aux, buffer + salto);
	if (strlen(aux) == 0) {
		/* Caso en el que no hay argumentos */
		syslog(LOG_ERR, "No hay argumentos para el script");
		strcpy(ubicacion, server_root);
		strcat(ubicacion, FICHERO_SIN_ARGS);
		stat(ubicacion, &c);
		total_bytes = c.st_size;		
		auxiliar = strtok(ctime(&c.st_mtime), "\n");
		bzero(ult_mod, sizeof(char)*TAM);
		strcpy(ult_mod, auxiliar);
		strcat(ult_mod, "\r\n");

		num_bytes = formar_respuesta(aux, "POST", OK, "OK", fecha, server_signature, ult_mod, total_bytes, "text/html");
		descr = open (ubicacion, O_RDONLY);
		if (descr < 0) {			/* El fichero del formulario sabemos que existe, asi que este error seria interno del open */
			syslog(LOG_ERR, "Error interno");
			return INTERNAL_SERVER_ERROR;
		}
		/* Enviamos la cabecera con la respuesta de error */
		send(connfd, aux, num_bytes, 0);
		/* Enviamos los datos del formulario html */ 
		while (num_bytes > 0) {
			memset(aux, 0, sizeof(aux));
			num_bytes = read(descr, aux, sizeof(aux));
			if (num_bytes == 0) {	/* Final del fichero */
				strcat(aux, "\n");
				send(connfd, aux, 1, 0);
				break;
			}
			send(connfd, aux, num_bytes, 0);
		}
		close(descr);
		return OK;
	}

	/* Preparamos la llamada al script y la realizamos */
	p_ret = ejecutar_script(content_type, real_path, aux, server_root);
	/* Comprobamos que se haya ejecutado el script correctamente */
	if (p_ret == NULL){
		syslog(LOG_ERR, "Error ejecutando el script (POST)");
		return INTERNAL_SERVER_ERROR;
	}
	/* Cerramos la tuberia formada por el popen */
	pclose(p_ret);

	/* Si llegamos hasta aqui, el script ha ido bien asi que debemos leer del fichero de salida, el cual sabemos que existe */
	strcpy(ubicacion, server_root);
	strcat(ubicacion, FICHERO_SALIDA);

	/* La extension del fichero de salida se ha decidido que siempre sea HTML */
	bzero(content_type, sizeof(char)*TAM);
	strcpy(content_type, "text/html");
	/* Obtenemos campos importantes del archivo */
	stat(ubicacion, &c);
	total_bytes = c.st_size;
	auxiliar = strtok(ctime(&c.st_mtime), "\n");
	bzero(ult_mod, sizeof(char)*TAM);
	strcpy(ult_mod, auxiliar);
	strcat(ult_mod, "\r\n");

	/* Abrimos el fichero HTML de salida */
	descr = open (ubicacion, O_RDONLY);
	/* Control de error sobre la apertura del fichero */
	if (descr < 0) {	/* El fichero ya sabemos que existe, asi que este error seria interno del open */
		syslog(LOG_ERR, "Error interno");
		return INTERNAL_SERVER_ERROR;
	}

	bzero(aux, sizeof(aux));
	num_bytes = formar_respuesta(aux, "POST", OK, "OK", fecha, server_signature, ult_mod, total_bytes, content_type);

	/* Enviamos la cabecera con la respuesta de confirmacion */
	send(connfd, aux, num_bytes, 0);
	/* Enviamos los datos del fichero en cuestion */
	while (num_bytes > 0) {
		memset(aux, 0, sizeof(aux));
		num_bytes = read(descr, aux, sizeof(aux));
		if (num_bytes == 0) {	/* Final del fichero */
			strcat(aux, "\n");
			send(connfd, aux, 1, 0);
			break;
		}
		send(connfd, aux, num_bytes, 0);
	}
	close(descr);
	return OK;
}



/********
* FUNCIÓN: Status_Code procesar_peticion_options (int connfd, char *content_type, char *fecha, int total_bytes, char *server_signature)
* ARGS_IN:  int connfd: descriptor de la conexion
*			char *content_type: MIME del fichero que se solicita
*			char *fecha: fecha en la que se realiza la peticion 
*			int total_bytes: numero de bytes del fichero que se solicita
*			char *server_signature: nombre del servidor 
* DESCRIPCIÓN: Funcion que se encarga de dar respuesta a la peticion OPTIONS del cliente
* ARGS_OUT: Status_Code - codigo de retorno correspondiente al 
*						  resultado de la peticion (OK en este caso)
*********/
Status_Code procesar_peticion_options (int connfd, char *content_type, char *fecha, int total_bytes, char *server_signature){
	int num_bytes;
	char mensaje[TAM] = ""; 

	num_bytes = formar_respuesta(mensaje, "OPTIONS", OK, "OK", fecha, server_signature, NULL, total_bytes, content_type);
	
	send(connfd, mensaje, num_bytes, 0);
	
	return OK;
}



/********
* FUNCIÓN: int formar_respuesta(char *respuesta, char *method, Status_Code status_code, char *status_msg,  \
						char *fecha, char *servidor, char *ult_mod, int total_bytes, char *content_type) 
* ARGS_IN:  char *respuesta: cadena donde se escribira la respuesta
*			char *method: metodo del que formar la respuesta (solo util para OPTIONS)
*			Status_Code status_code: codigo de estado del procesado de la peticion
*			char *status_msg: mensaje del codigo de estado
*			char *fecha: fecha en la que se realiza la peticion 
*			char *servidor: nombre del servidor 
*			char *ult_mod: fecha de ultima modificacion del fichero que se solicita
*			int total_bytes: numero de bytes del fichero que se solicita
*			char *content_type: MIME del fichero que se solicita
* DESCRIPCIÓN: Funcion que forma la respuesta del servidor a una peticion
* ARGS_OUT: int - numero de bytes de la respuesta
*********/
int formar_respuesta(char *respuesta, char *method, Status_Code status_code, char *status_msg,  \
						char *fecha, char *servidor, char *ult_mod, int total_bytes, char *content_type) {
	int num_bytes;

	num_bytes = sprintf(respuesta, "HTTP/1.1 %d %s\r\nDate: %sServer: %s\r\n", status_code, status_msg, fecha, servidor);
	
	if (status_code == METHOD_NOT_IMPLEMENTED || strcmp(method, "OPTIONS") == 0) {
		if (strcmp(content_type, "application/py") == 0 || strcmp(content_type, "application/php") == 0 || 
			strcmp(content_type, "*") == 0 || status_code == METHOD_NOT_IMPLEMENTED) {
			num_bytes += sprintf(respuesta + num_bytes, "Allow: GET HEAD POST OPTIONS\r\n");
		} else {
			num_bytes += sprintf(respuesta + num_bytes, "Allow: GET HEAD OPTIONS\r\n");
		}
	}
	if (status_code == OK  && strcmp(method, "OPTIONS")) {
		num_bytes += sprintf(respuesta + num_bytes, "Last-Modified: %s", ult_mod);
	} 

	if (strcmp(content_type, "*") == 0) {
		/* Esto debera ser un tipo de archivo valido en funcion del cuerpo de la peticion */
		strcpy(content_type, "text/plain");  /* Escogemos text/plain pero en esta version de HTTP/1.1 todavia no se emplea para nada */
	}

	num_bytes += sprintf(respuesta + num_bytes, "Content-Length: %d bytes\r\nContent-Type: %s\r\n\r\n", total_bytes, content_type);

	return num_bytes;
}

/********
* FUNCIÓN: FILE *ejecutar_script(char *content_type, char *path, char *argumentos, char *server_root)
* ARGS_IN:  char *content_type: MIME del script (application/py o application/php)
*		    char *path: ruta donde se encuentra el script
*		   	char *argumentos: argumentos de entrada para el script
* 			char *server_root: ruta del servidor
* DESCRIPCIÓN: Prepara la cadena de texto que debera ser pasada al
*              shell para ejecutar el script y realiza dicha ejecucion 
*			   mediante popen, pasando argumentos de entrada
* ARGS_OUT: FILE * - devuelve un puntero al fichero que devuelve el popen o NULL 
*					 en caso de extension no valida del script
*********/
FILE * ejecutar_script(char *content_type, char *path, char *argumentos, char *server_root){
	FILE *p_ret = NULL;
	char command[TAM]="";
	char aux[TAM]="";

	if (strcmp(content_type, "application/py") == 0) {
		strcpy(command, "python ");
	} else if (strcmp(content_type, "application/php") == 0) {
		strcpy(command, "php ");
	} else {
		return NULL;
	}

	strcpy(aux, server_root); 
	strcat(aux, FICHERO_SALIDA);

	strcat(command, path);
	strcat(command, " ");
	strcat(command, argumentos);
	strcat(command, " ");
	strcat(command, aux);
	syslog(LOG_ERR, "%s", command);
	p_ret = popen(command, "w");

	return p_ret;
}

