/***************************************************************************
 practica2.h
 Fichero cabecera del ejercicio de la practica 2 de Redes I. 
 Contiene las constantes que se emplean.

 Autor: Alejandro Sánchez Sanz y Ricardo Riol González (Pareja 9)
 2017
***************************************************************************/

#ifndef PRACTICA2_H
#define PRACTICA2_H

/*Definicion de constantes *************************************************/
#define ETH_ALEN      6      /* Tamanio de la direccion ethernet           */
#define ETH_HLEN      14     /* Tamanio de la cabecera ethernet            */
#define ETH_TLEN      2      /* Tamanio del campo tipo ethernet            */
#define ETH_FRAME_MAX 1514   /* Tamanio maximo la trama ethernet (sin CRC) */
#define ETH_FRAME_MIN 60     /* Tamanio minimo la trama ethernet (sin CRC) */
#define ETH_DATA_MAX  (ETH_FRAME_MAX - ETH_HLEN) /* Tamano maximo y minimo de los datos de una trama ethernet*/
#define ETH_DATA_MIN  (ETH_FRAME_MIN - ETH_HLEN)
#define IP_ALEN 4           /* Tamanio de la direccion IP                   */
#define OK 0
#define ERROR 1
#define PACK_READ 1
#define PACK_ERR -1
#define TRACE_END -2
#define NO_FILTER 0

/* Cabeceras de las funciones a utilizar */
void analizar_paquete(const struct pcap_pkthdr *hdr, const uint8_t *pack);

void handleSignal(int nsignal);


#endif