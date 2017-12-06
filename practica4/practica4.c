/***************************************************************************
 practica4.c
 Inicio, funciones auxiliares y modulos de transmision implmentados y a implementar de la practica 4.
Compila con warning pues falta usar variables y modificar funciones
 
 Compila: make
 Autores: Alejandro Sanchez Sanz y Ricardo Riol Gonzalez
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "interface.h"
#include "practica4.h"

/***************************Variables globales utiles*************************************************/
pcap_t* descr, *descr2; //Descriptores de la interface de red
pcap_dumper_t * pdumper;//y salida a pcap
uint64_t cont=0;	//Contador numero de mensajes enviados
char interface[10];	//Interface donde transmitir por ejemplo "eth0"
uint16_t ID=1;		//Identificador IP


void handleSignal(int nsignal){
	printf("Control C pulsado (%"PRIu64")\n", cont);
	pcap_close(descr);
	exit(OK);
}

int main(int argc, char **argv){	

	char errbuf[PCAP_ERRBUF_SIZE];
	char fichero_pcap_destino[CADENAS];
	uint8_t IP_destino_red[IP_ALEN];
	uint16_t MTU;
	uint16_t datalink;
	uint16_t puerto_destino;
	char data[IP_DATAGRAM_MAX];
	uint16_t pila_protocolos[CADENAS];
	FILE *fichero_a_transmitir = NULL;
	Parametros parametros_udp, parametros_icmp; 

	int long_index=0;
	char opt;
	char flag_iface = 0, flag_ip = 0, flag_port = 0, flag_file = 0;

	static struct option options[] = {
		{"if",required_argument,0,'1'},
		{"ip",required_argument,0,'2'},
		{"pd",required_argument,0,'3'},
		{"f",required_argument,0,'4'},
		{"h",no_argument,0,'5'},
		{0,0,0,0}
	};

		//Dos opciones: leer de stdin o de fichero, adicionalmente para pruebas si no se introduce argumento se considera que el mensaje es "Payload "
	while ((opt = getopt_long_only(argc, argv,"1:2:3:4:5", options, &long_index )) != -1) {
		switch (opt) {

			case '1' :

				flag_iface = 1;
					//Por comodidad definimos interface como una variable global
				sprintf(interface,"%s",optarg);
				break;

			case '2' : 

				flag_ip = 1;
					//Leemos la IP a donde transmitir y la almacenamos en orden de red
				if (sscanf(optarg,"%"SCNu8".%"SCNu8".%"SCNu8".%"SCNu8"",
				                   &(IP_destino_red[0]),&(IP_destino_red[1]),&(IP_destino_red[2]),&(IP_destino_red[3])) != IP_ALEN){
					printf("Error: Fallo en la lectura IP destino %s\n", optarg);
					exit(ERROR);
				}

				break;

			case '3' :

				flag_port = 1;
					//Leemos el puerto a donde transmitir y lo almacenamos en orden de hardware
				puerto_destino = atoi(optarg);
				break;

			case '4' :

				if (strcmp(optarg, "stdin") == 0) {
					if (fgets(data, sizeof data, stdin) == NULL) {
						printf("Error leyendo desde stdin: %s %s %d.\n",errbuf,__FILE__,__LINE__);
						exit(ERROR);
					}
					sprintf(fichero_pcap_destino,"%s%s","stdin",".pcap");
				} else {
					sprintf(fichero_pcap_destino,"%s%s",optarg,".pcap");
					fichero_a_transmitir = fopen(optarg, "r");
					if (fichero_a_transmitir == NULL) {
						printf("Error leyendo desde el fichero a transmitir: %s %s %d.\n",errbuf,__FILE__,__LINE__);
						exit(ERROR);
					}
					while (fgets(data, sizeof data, fichero_a_transmitir) != NULL);
					if (strlen(data)%2 != 0) {  // Comprobamos que los datos son pares 
						strcat(data," ");
					}
					if (strlen(data)%2 == 0) {  // Pura comprobacion, luego habra que quitarlo
						printf("AHORA YA FUNCIONA \n");
					}
					fclose(fichero_a_transmitir);
				}
				flag_file = 1;

				break;

			case '5' : printf("Ayuda. Ejecucion: %s -if interface -ip IP -pd Puerto <-f /ruta/fichero_a_transmitir o stdin>: %d\n",argv[0],argc); exit(ERROR);
				break;

			case '?' : printf("Error. Ejecucion: %s -if interface -ip IP -pd Puerto <-f /ruta/fichero_a_transmitir o stdin>: %d\n",argv[0],argc); exit(ERROR);
				break;

			default: printf("Error. Ejecucion: %s -if interface -ip IP -pd Puerto <-f /ruta/fichero_a_transmitir o stdin>: %d\n",argv[0],argc); exit(ERROR);
				break;
        }
    }

	if ((flag_iface == 0) || (flag_ip == 0) || (flag_port == 0)){
		printf("Error. Ejecucion: %s -if interface -ip IP -pd Puerto <-f /ruta/fichero_a_transmitir o stdin>: %d\n",argv[0],argc);
		exit(ERROR);
	} else {
		printf("Interface:\n\t%s\n",interface);
		printf("IP:\n\t%"PRIu8".%"PRIu8".%"PRIu8".%"PRIu8"\n",IP_destino_red[0],IP_destino_red[1],IP_destino_red[2],IP_destino_red[3]);
		printf("Puerto destino:\n\t%"PRIu16"\n",puerto_destino);
	}

	if (flag_file == 0) {
		sprintf(data,"%s","Payload "); //Deben ser pares!
		sprintf(fichero_pcap_destino,"%s%s","debugging",".pcap");
	}

	if(signal(SIGINT,handleSignal)==SIG_ERR){
		printf("Error: Fallo al capturar la senal SIGINT.\n");
		return ERROR;
	}
		//Inicializamos las tablas de protocolos
	if(inicializarPilaEnviar()==ERROR){
      	printf("Error iniciando las tablas de protocolos: %s %s %d.\n",errbuf,__FILE__,__LINE__);
		return ERROR;
	}

		//Leemos el tamano maximo de transmision del nivel de enlace
	if(obtenerMTUInterface(interface, &MTU)==ERROR)
		return ERROR;

		//Descriptor de la interface de red donde inyectar trafico
	if ((descr = pcap_open_live(interface,MTU+ETH_HLEN,0, 0, errbuf)) == NULL){
		printf("Error: pcap_open_live(): %s %s %d.\n",errbuf,__FILE__,__LINE__);
		return ERROR;
	}

	datalink=(uint16_t)pcap_datalink(descr); //DLT_EN10MB==Ethernet

		//Descriptor del fichero de salida pcap para debugging
	descr2=pcap_open_dead(datalink,MTU+ETH_HLEN);
	pdumper=pcap_dump_open(descr2,fichero_pcap_destino);

		//Formamos y enviamos el trafico, debe enviarse un unico segmento por llamada a enviar() aunque luego se traduzca en mas de un datagrama
		//Primero un paquete UDP
		//Definimos la pila de protocolos que queremos seguir
	pila_protocolos[0]=UDP_PROTO; 
	pila_protocolos[1]=IP_PROTO; 
	pila_protocolos[2]=ETH_PROTO;
		//Rellenamos los parametros necesarios para enviar el paquete a su destinatario y proceso
	memcpy(parametros_udp.IP_destino,IP_destino_red,IP_ALEN); 
	parametros_udp.puerto_destino=puerto_destino;
		//Enviamos
	if (enviar((uint8_t*)data,strlen(data),pila_protocolos,&parametros_udp)==ERROR){
		printf("Error: enviar(): %s %s %d.\n",errbuf,__FILE__,__LINE__);
		return ERROR;
	}
	else	cont++;
	printf("Enviado mensaje %"PRIu64", almacenado en %s\n\n\n", cont,fichero_pcap_destino);

		//Luego, un paquete ICMP en concreto un ping
	pila_protocolos[0]=ICMP_PROTO; 
	pila_protocolos[1]=IP_PROTO;
	parametros_icmp.tipo=PING_TIPO; 
	parametros_icmp.codigo=PING_CODE; 
	memcpy(parametros_icmp.IP_destino,IP_destino_red,IP_ALEN);
	if(enviar((uint8_t*)"Probando a hacer un ping",pila_protocolos,strlen("Probando a hacer un ping"),&parametros_icmp)==ERROR ){
		printf("Error: enviar(): %s %s %d.\n",errbuf,__FILE__,__LINE__);
		return ERROR;
	}
	else	cont++;
	printf("Enviado mensaje %"PRIu64", ICMP almacenado en %s\n\n", cont,fichero_pcap_destino);

		//Cerramos descriptores
	pcap_close(descr);
	pcap_dump_close(pdumper);
	pcap_close(descr2);
	return OK;
}


/****************************************************************************************
* Nombre: enviar 									*
* Descripcion: Esta funcion envia un mensaje						*
* Argumentos: 										*
*  -mensaje: mensaje a enviar								*
*  -pila_protocolos: conjunto de protocolos a seguir					*
*  -longitud: bytes que componen mensaje						*
*  -parametros: parametros necesario para el envio (struct parametros)			*
* Retorno: OK/ERROR									*
****************************************************************************************/

uint8_t enviar(uint8_t* mensaje, uint64_t longitud,uint16_t* pila_protocolos,void *parametros){
	uint16_t protocolo=pila_protocolos[0];
	printf("Enviar(%"PRIu16") %s %d.\n",protocolo,__FILE__,__LINE__);
	if(protocolos_registrados[protocolo]==NULL){
		printf("Protocolo %"PRIu16" desconocido\n",protocolo);
		return ERROR;
	}
	else {
		return protocolos_registrados[protocolo](mensaje,longitud,pila_protocolos,parametros);
	}
	return ERROR;
}


/***************************TODO Pila de protocolos a implementar************************************/

/****************************************************************************************
* Nombre: moduloUDP 									*
* Descripcion: Esta funcion implementa el modulo de envio UDP				*
* Argumentos: 										*
*  -mensaje: mensaje a enviar								*
*  -longitud: bytes que componen mensaje						*
*  -pila_protocolos: conjunto de protocolos a seguir					*
*  -parametros: parametros necesario para el envio este protocolo			*
* Retorno: OK/ERROR									*
****************************************************************************************/

uint8_t moduloUDP(uint8_t* mensaje, uint64_t longitud, uint16_t* pila_protocolos,void *parametros){
	uint8_t segmento[UDP_SEG_MAX]={0};
	uint16_t puerto_origen = 0, puerto_destino = 0, suma_control=0;
	uint16_t aux16;
	uint32_t pos=0;
	uint16_t protocolo_inferior=pila_protocolos[1];
	Parametros udpdatos; 
	printf("modulo UDP(%"PRIu16") %s %d.\n",protocolo_inferior,__FILE__,__LINE__);

	if (longitud>(UDP_SEG_MAX-IP_HLEN-UDP_HLEN)){
		printf("Error: mensaje demasiado grande para UDP (%f).\n",(UDP_SEG_MAX-IP_HLEN-UDP_HLEN));
		return ERROR;
	}

	udpdatos=*((Parametros*)parametros);
	puerto_destino=udpdatos.puerto_destino;

		//Puerto origen
	obtenerPuertoOrigen(&puerto_origen);
	aux16 = htons(puerto_origen);
	memcpy(segmento+pos,&aux16,sizeof(uint16_t));
	pos += sizeof(uint16_t);

		//Puerto destino
	aux16 = htons(puerto_destino);
	memcpy(segmento+pos,&aux16,sizeof(uint16_t));
	pos += sizeof(uint16_t);

		//Longitud
	aux16 = htons((uint16_t)longitud + UDP_HLEN);
	memcpy(segmento+pos, &aux16, sizeof(uint16_t));
	pos += sizeof(uint16_t);

		//CheckSum (campo a 0)
	memcpy(segmento+pos, 0, sizeof(uint16_t));
	pos += sizeof(uint16_t);

		//Mensaje
	memcpy(segmento+pos, mensaje, longitud);


		//Se llama al protocolo definido de nivel inferior a traves de los punteros registrados en la tabla de protocolos registrados
	return protocolos_registrados[protocolo_inferior](segmento,longitud+pos,pila_protocolos,parametros);
}


/****************************************************************************************
* Nombre: moduloIP 									*
* Descripcion: Esta funcion implementa el modulo de envio IP				*
* Argumentos: 										*
*  -segmento: segmento a enviar								*
*  -longitud: bytes que componen el segmento						*
*  -pila_protocolos: conjunto de protocolos a seguir					*
*  -parametros: parametros necesario para el envio este protocolo			*
* Retorno: OK/ERROR									*
*
* ***************************************************************************************/

uint8_t moduloIP(uint8_t* segmento, uint64_t longitud, uint16_t* pila_protocolos,void *parametros){
	int i, flag = 1;
	uint8_t datagrama[IP_DATAGRAM_MAX]={0};
	uint32_t aux32;
	uint16_t aux16;
	uint8_t aux8;
	uint32_t pos=0,pos_control=0;
	uint8_t IP_origen[IP_ALEN];
	uint16_t protocolo_superior=pila_protocolos[0];
	uint16_t protocolo_inferior=pila_protocolos[2];
	uint8_t mascara[IP_ALEN],IP_rango_origen[IP_ALEN],IP_rango_destino[IP_ALEN];
	uint8_t* IP_destino = NULL;
	uint8_t checksum[2]={0};
	Parametros ipdatos;

	printf("modulo IP(%"PRIu16") %s %d.\n",protocolo_inferior,__FILE__,__LINE__);
	if (longitud>(IP_DATAGRAM_MAX-IP_HLEN)){
		printf("Error: mensaje demasiado grande para IP (%f).\n",(IP_DATAGRAM_MAX-IP_HLEN));
		return ERROR;
	}

	ipdatos = *((Parametros*)parametros);
	IP_destino = ipdatos.IP_destino;

	obtenerIPInterface(interface, IP_origen); // Obtenemos la direccion IP origen
	obtenerMascaraInterface(interface, mascara); //Obtenemos la mascara de red de la interfaz

	//IP_rango_origen = IP_origen and mascara; IP_rango_destino = IP_destino and mascara;
	for (i = 0; i < IP_ALEN; i++){
		IP_rango_origen[i] = IP_origen[i] & mascara[i];
		IP_rango_destino[i] = IP_destino[i] & mascara[i];
		if (IP_rango_origen != IP_rango_destino){
			flag = 0;
		}
	}

	if (flag == 1){ 
		// El terminal esta en nuestra misma subred.
		ARPrequest(interface, IP_destino, ipdatos.ETH_destino); //Obtenemos la mac del terminal (misma subred)
	} 
	else {
		obtenerGateway(interface, &aux8); //Cogemos la IP de la interfaz del router
		ARPrequest(interface, aux8, ipdatos.ETH_destino); //Pedimos la mac de la interfaz del router
	}

	aux16 = 0x4500; // Escribimos en el datagrama los siguientes campos: version, IHL, Tipo de servicio.
	memcpy (datagrama+pos, &aux16, sizeof(uint16_t));
	pos += sizeof(uint16_t);

	//Calculamos la longitud total (sin fragmentacion)

	obtenerMTUInterface(interface, &aux16); //Obtenemos la MTU del nivel fisico
	aux16 = htons(aux16);
	aux16 = aux16 - IP_HLEN; //Restamos a la MTU el tamanio de la cabecera IP
	aux16 = (aux16 / 8) * 8; //Calculamos el menor multiplo de 8 de la cantidad MTU - longitudCabeceraIP

	if ((uint16_t)longitud > aux16){
		printf ("FRAGMENTAR.\n");
	}

	// Campo longitud total
	aux16 = (uint16_t)longitud + IP_HLEN;
	memcpy(datagrama+pos, &aux16, sizeof(uint16_t));
	pos += sizeof(uint16_t);

	// Campo identificador (cuidado, el mismo para todos los fragmentos del mismo paquete)
	aux16 = rand(); // srand??
	memcpy(datagrama+pos, &aux16, sizeof(uint16_t));
	pos += sizeof(uint16_t);

	aux16 = 0x4000; // Importante: Son valores especificos porque asumo no fragmentacion
	memcpy(datagrama+pos, &aux16, sizeof(uint16_t)); //Flags y posicion
	pos += sizeof(uint16_t);

	aux8 = 0x40; // Tiempo de vida
	memcpy(datagrama+pos, &aux8, sizeof(uint8_t));
	pos += sizeof(uint8_t);

	// Protocolo
	aux8 = protocolo_superior;
	memcpy(datagrama+pos, &aux8, sizeof(uint8_t));
	pos += sizeof(uint8_t);

 	//Checksum se calcula al final. Ponemos un 0. Una vez calculado se sustituira por el valor correspondiente
 	memcpy(datagrama+pos, 0, sizeof(uint16_t));
 	pos_control = pos;
 	pos += sizeof(uint16_t);

 	//Direccion IP origen
 	for (i = 0; i < IP_ALEN; i++){
 		memcpy(datagrama+pos, &(IP_origen[i]), sizeof(uint8_t));
 		pos += sizeof(uint8_t);
 	}

 	//Direccion IP destino
 	for (i = 0; i < IP_ALEN; i++){
 		memcpy(datagrama+pos, &(IP_destino[i]), sizeof(uint8_t));
 		pos += sizeof(uint8_t);
 	}

 	//Calculamos el CheckSum y lo escribimos en el lugar correspondiente
 	calcularChecksum(pos, datagrama, checksum);
 	//datagrama[pos_control] = checksum[0]; //no se como sobrescribir los 0.
 	//datagrama[pos_control+1] = checksum[1];
 	memcpy(datagrama+pos_control,checksum,sizeof(uint16_t));

 	//Segmento
	memcpy(datagrama+pos, segmento, longitud);

 	// No se hacia pila_protocolos++ o algo asi??
	return protocolos_registrados[protocolo_inferior](datagrama,longitud+pos,pila_protocolos,parametros);

//TODO
//Llamar a ARPrequest(·) adecuadamente y usar ETH_destino de la estructura parametros
//[...] 
//TODO A implementar el datagrama y fragmentación (en caso contrario, control de tamano)
//[...] 
//llamada/s a protocolo de nivel inferior [...]

	//version un 4, ihl un 5, tipo de servicio a 0, longitud total calculando con los datos
	// identificador aleatorio con rand() de un uint16_t, con cuidado de que sea el mismo para todos
	// los fragmentos del mismo paquete fragmentado; flag y offset(cuidado porque hay que ponerlo dividido entre 8) 
	// en funcion de si es fragmento o no; ttl un valor parecido a uno de una captura que veamos (64 o 128); protocolo (1 si es ICMP
	// y 17 si es UDP); checksum se genera con funcion; ip origen se calcula con funcion y la ip destino es parametro

}


/****************************************************************************************
* Nombre: moduloETH 									*
* Descripcion: Esta funcion implementa el modulo de envio Ethernet			*
* Argumentos: 										*
*  -datagrama: datagrama a enviar							*
*  -longitud: bytes que componen el datagrama						*
*  -pila_protocolos: conjunto de protocolos a seguir					*
*  -parametros: Parametros necesario para el envio este protocolo			*
* Retorno: OK/ERROR									*
****************************************************************************************/

uint8_t moduloETH(uint8_t* datagrama, uint64_t longitud, uint16_t* pila_protocolos,void *parametros){
//TODO
//[...] Variables del modulo
	uint16_t aux_16;
	uint8_t eth_src[ETH_ALEN], eth_dst[ETH_ALEN];
	uint16_t pos = 0;
	uint8_t trama[ETH_FRAME_MAX]={0};
	uint16_t protocolo_superior=pila_protocolos[1];
	Parametros ethdatos;

	printf("modulo ETH(fisica) %s %d.\n",__FILE__,__LINE__);	

//TODO
//[...] Control de tamano
	if (longitud > ETH_FRAME_MAX-ETH_HLEN){
		printf("Error: mensaje demasiado grande para Ethernet (%f).\n",(ETH_FRAME_MAX-ETH_HLEN));
		return ERROR;
	}

	ethdatos = *(Parametros *)parametros;
	eth_dst = ethdatos.ETH_destino;

//TODO
		// Direccion Ethernet de origen
	obtenerMACdeInterface(interface, eth_src);
	memcpy(trama+pos, eth_src, sizeof(uint8_t)*ETH_ALEN);
	pos += sizeof(uint8_t)*ETH_ALEN;

		// Direccion Ethernet de destino
	memcpy(trama+pos, eth_dst, sizeof(uint8_t)*ETH_ALEN);
	pos += sizeof(uint8_t)*ETH_ALEN;

	//Tipo Ethernet
	aux_16 = 0x0800;
	memcpy (trama+pos, &aux_16, sizeof(uint16_t));
	pos+=sizeof(uint16_t);


//TODO
//Enviar a capa fisica [...]  
//TODO
//Almacenamos la salida por cuestiones de debugging [...]
	
	return OK;
}


/****************************************************************************************
* Nombre: moduloICMP 									*
* Descripcion: Esta funcion implementa el modulo de envio ICMP				*
* Argumentos: 										*
*  -mensaje: mensaje a anadir a la cabecera ICMP					*
*  -longitud: bytes que componen mensaje						*
*  -pila_protocolos: conjunto de protocolos a seguir					*
*  -parametros: parametros necesario para el envio este protocolo			*
* Retorno: OK/ERROR									*
****************************************************************************************/

uint8_t moduloICMP(uint8_t* mensaje,uint64_t longitud, uint16_t* pila_protocolos,void *parametros){
	uint8_t segmento[UDP_SEG_MAX]={0};
	uint16_t aux16;
	uint32_t pos=0, pos_control=0;	
	uint8_t checksum[2]={0};

	if (longitud > (ICMP_DATAGRAM_MAX-ICMP_HLEN)){
		printf("Error: mensaje demasiado grande para ICMP (%f).\n",(ICMP_DATAGRAM_MAX-ICMP_HLEN));
		return ERROR;
	}
	// Tipo (8) y Codigo (0)
	aux16 = 0x0800;
	memcpy(segmento, &aux16, sizeof(uint16_t));
	pos += sizeof(uint16_t);

	//Checksum provisional
	memcpy(segmento, 0, sizeof(uint16_t));
	pos_control = pos;
	pos += sizeof(uint16_t);

	//Identificador
	aux16 = rand();
	memcpy(segmento, aux16, sizeof(uint16_t));
	pos += sizeof(uint16_t);

	//Numero de secuencia
	aux16 = rand();
	memcpy(segmento, aux16, sizeof(uint16_t));
	pos += sizeof(uint16_t);

	//Mensaje
	memcpy(segmento+pos, mensaje, longitud);

	//Calculamos checksum
	calcularChecksum(longitud+pos, segmento, checksum);
	memcpy(segmento+pos_control,checksum,sizeof(uint16_t));

	return protocolos_registrados[protocolo_inferior](segmento,longitud+pos,pila_protocolos,parametros);
	//htons para los campos de 16 bits como son id y n_secuencia

}


/***************************Funciones auxiliares a implementar***********************************/

/****************************************************************************************
* Nombre: aplicarMascara 								*
* Descripcion: Esta funcion aplica una mascara a una vector				*
* Argumentos: 										*
*  -IP: IP a la que aplicar la mascara en orden de red					*
*  -mascara: mascara a aplicar en orden de red						*
*  -longitud: bytes que componen la direccion (IPv4 == 4)				*
*  -resultado: Resultados de aplicar mascara en IP en orden red				*
* Retorno: OK/ERROR									*
****************************************************************************************/

uint8_t aplicarMascara(uint8_t* IP, uint8_t* mascara, uint32_t longitud, uint8_t* resultado){
//TODO
//[...]
}


/***************************Funciones auxiliares implementadas**************************************/

/****************************************************************************************
* Nombre: mostrarPaquete 								*
* Descripcion: Esta funcion imprime por pantalla en hexadecimal un vector		*
* Argumentos: 										*
*  -paquete: bytes que conforman un paquete						*
*  -longitud: Bytes que componen el mensaje						*
* Retorno: OK/ERROR									*
****************************************************************************************/

uint8_t mostrarPaquete(uint8_t * paquete, uint32_t longitud){
	uint32_t i;
	printf("Paquete:\n");
	for (i=0;i<longitud;i++){
		printf("%02"PRIx8" ", paquete[i]);
	}
	printf("\n");
	return OK;
}


/****************************************************************************************
* Nombre: calcularChecksum							     	*
* Descripcion: Esta funcion devuelve el ckecksum tal como lo calcula IP/ICMP		*
* Argumentos:										*
*   -longitud: numero de bytes de los datos sobre los que calcular el checksum		*
*   -datos: datos sobre los que calcular el checksum					*
*   -checksum: checksum de los datos (2 bytes) en orden de red! 			*
* Retorno: OK/ERROR									*
****************************************************************************************/

uint8_t calcularChecksum(uint16_t longitud, uint8_t *datos, uint8_t *checksum) {
    uint16_t word16;
    uint32_t sum=0;
    int i;
    // make 16 bit words out of every two adjacent 8 bit words in the packet
    // and add them up
    for (i=0; i<longitud; i=i+2){
        word16 = (datos[i]<<8) + datos[i+1];
        sum += (uint32_t)word16;       
    }
    // take only 16 bits out of the 32 bit sum and add up the carries
    while (sum>>16) {
        sum = (sum & 0xFFFF)+(sum >> 16);
    }
    // one's complement the result
    sum = ~sum;      
    checksum[0] = sum >> 8;
    checksum[1] = sum & 0xFF;
    return OK;
}


/***************************Funciones inicializacion implementadas*********************************/

/****************************************************************************************
* Nombre: inicializarPilaEnviar     							*
* Descripcion: inicializar la pila de red para enviar registrando los distintos modulos *
* Retorno: OK/ERROR									*
****************************************************************************************/

uint8_t inicializarPilaEnviar() {
	bzero(protocolos_registrados,MAX_PROTOCOL*sizeof(pf_notificacion));
	if(registrarProtocolo(ETH_PROTO, moduloETH, protocolos_registrados)==ERROR)
		return ERROR;
	if(registrarProtocolo(IP_PROTO, moduloIP, protocolos_registrados)==ERROR)
		return ERROR;
	if(registrarProtocolo(UDP_PROTO, moduloUDP, protocolos_registrados)==ERROR)
		return ERROR;
	if(registrarProtocolo(ICMP_PROTO, moduloICMP, protocolos_registrados)==ERROR)
		return ERROR;
	return OK;
}


/****************************************************************************************
* Nombre: registrarProtocolo 								*
* Descripcion: Registra un protocolo en la tabla de protocolos 				*
* Argumentos:										*
*  -protocolo: Referencia del protocolo (ver RFC 1700)					*
*  -handleModule: Funcion a llamar con los datos a enviar				*
*  -protocolos_registrados: vector de funciones registradas 				*
* Retorno: OK/ERROR 									*
*****************************************************************************************/

uint8_t registrarProtocolo(uint16_t protocolo, pf_notificacion handleModule, pf_notificacion* protocolos_registrados){
	if(protocolos_registrados==NULL ||  handleModule==NULL){		
		printf("Error: registrarProtocolo(): entradas nulas.\n");
		return ERROR;
	}
	else
		protocolos_registrados[protocolo]=handleModule;
	return OK;
}


