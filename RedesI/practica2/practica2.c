/***************************************************************************
 practica2.c
 Muestra las direciones Ethernet de la traza que se pasa como primer parametro.
 Debe complatarse con mas campos de niveles 2, 3, y 4 tal como se pida en el enunciado.
 Debe tener capacidad de dejar de analizar paquetes de acuerdo a un filtro.

 Compila: gcc -Wall -o practica2 practica2.c -lpcap, make
 Autor: Jose Luis Garcia Dorado, Jorge E. Lopez de Vergara Mendez, Rafael Leira
 2017 EPS-UAM
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <pcap.h>
#include <string.h>
#include <netinet/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include <inttypes.h>
#include "practica2.h"


/* Variables globales */
pcap_t *descr = NULL;
uint64_t contador = 0;
uint8_t ipsrc_filter[IP_ALEN] = {NO_FILTER};
uint8_t ipdst_filter[IP_ALEN] = {NO_FILTER};
uint16_t sport_filter= NO_FILTER;
uint16_t dport_filter = NO_FILTER;

void handleSignal(int nsignal)
{
    (void) nsignal; // indicamos al compilador que no nos importa que nsignal no se utilice

    printf("Control C pulsado (%"PRIu64" paquetes leidos)\n", contador);
    pcap_close(descr);
    exit(OK);
}

int main(int argc, char **argv)
{
    uint8_t *pack = NULL;
    struct pcap_pkthdr *hdr;
    char errbuf[PCAP_ERRBUF_SIZE];
    char entrada[256];
    int long_index = 0, retorno = 0;
    char opt;
    
    (void) errbuf; //indicamos al compilador que no nos importa que errbuf no se utilice. Esta linea debe ser eliminada en la entrega final.

    if (signal(SIGINT, handleSignal) == SIG_ERR) {
        printf("Error: Fallo al capturar la senal SIGINT.\n");
        exit(ERROR);
    }

    if (argc > 1) {
        if (strlen(argv[1]) < 256) {
            strcpy(entrada, argv[1]);
        }

    } else {
        printf("Ejecucion: %s <-f traza.pcap / -i eth0> [-ipo IPO] [-ipd IPD] [-po PO] [-pd PD]\n", argv[0]);
        exit(ERROR);
    }

    static struct option options[] = {
        {"f", required_argument, 0, 'f'},
        {"i",required_argument, 0,'i'},
        {"ipo", required_argument, 0, '1'},
        {"ipd", required_argument, 0, '2'},
        {"po", required_argument, 0, '3'},
        {"pd", required_argument, 0, '4'},
        {"h", no_argument, 0, '5'},
        {0, 0, 0, 0}
    };

    //Simple lectura por parametros por completar casos de error, ojo no cumple 100% los requisitos del enunciado!
    while ((opt = getopt_long_only(argc, argv, "f:i:1:2:3:4:5", options, &long_index)) != -1) {
        switch (opt) {
        case 'i' :
            if(descr) { // comprobamos que no se ha abierto ninguna otra interfaz o fichero
                printf("Ha seleccionado más de una fuente de datos\n");
                pcap_close(descr);
                exit(ERROR);
            }
            
            if ( (descr = pcap_open_live(optarg, ETH_FRAME_MAX, 0, 100, errbuf)) == NULL){
                printf("Error: pcap_open_live(): Interface: %s, %s %s %d.\n", optarg,errbuf,__FILE__,__LINE__);
                exit(ERROR);
            }
            break;

        case 'f' :
            if(descr) { // comprobamos que no se ha abierto ninguna otra interfaz o fichero
                printf("Ha seleccionado más de una fuente de datos\n");
                pcap_close(descr);
                exit(ERROR);
            }

            if ((descr = pcap_open_offline(optarg, errbuf)) == NULL) {
                printf("Error: pcap_open_offline(): File: %s, %s %s %d.\n", optarg, errbuf, __FILE__, __LINE__);
                exit(ERROR);
            }

            break;

        case '1' :
            if (sscanf(optarg, "%"SCNu8".%"SCNu8".%"SCNu8".%"SCNu8"", &(ipsrc_filter[0]), &(ipsrc_filter[1]), &(ipsrc_filter[2]), &(ipsrc_filter[3])) != IP_ALEN) {
                printf("Error ipo_filtro. Ejecucion: %s /ruta/captura_pcap [-ipo IPO] [-ipd IPD] [-po PO] [-pd PD]: %d\n", argv[0], argc);
                exit(ERROR);
            }

            break;

        case '2' :
            if (sscanf(optarg, "%"SCNu8".%"SCNu8".%"SCNu8".%"SCNu8"", &(ipdst_filter[0]), &(ipdst_filter[1]), &(ipdst_filter[2]), &(ipdst_filter[3])) != IP_ALEN) {
                printf("Error ipd_filtro. Ejecucion: %s /ruta/captura_pcap [-ipo IPO] [-ipd IPD] [-po PO] [-pd PD]: %d\n", argv[0], argc);
                exit(ERROR);
            }

            break;

        case '3' :
            if ((sport_filter = atoi(optarg)) == 0) {
                printf("Error po_filtro.Ejecucion: %s /ruta/captura_pcap [-ipo IPO] [-ipd IPD] [-po PO] [-pd PD]: %d\n", argv[0], argc);
                exit(ERROR);
            }

            break;

        case '4' :
            if ((dport_filter = atoi(optarg)) == 0) {
                printf("Error pd_filtro. Ejecucion: %s /ruta/captura_pcap [-ipo IPO] [-ipd IPD] [-po PO] [-pd PD]: %d\n", argv[0], argc);
                exit(ERROR);
            }

            break;

        case '5' :
            printf("Ayuda. Ejecucion: %s <-f traza.pcap / -i eth0> [-ipo IPO] [-ipd IPD] [-po PO] [-pd PD]: %d\n", argv[0], argc);
            exit(ERROR);
            break;

        case '?' :
        default:
            printf("Error. Ejecucion: %s <-f traza.pcap / -i eth0> [-ipo IPO] [-ipd IPD] [-po PO] [-pd PD]: %d\n", argv[0], argc);
            exit(ERROR);
            break;
        }
    }

    if (!descr) {
        printf("No selecciono ningún origen de paquetes.\n");
        return ERROR;
    }

    //Simple comprobacion de la correcion de la lectura de parametros
    printf("Filtro:");
    if(ipsrc_filter[0]!=0)
        printf("ipsrc_filter:%"PRIu8".%"PRIu8".%"PRIu8".%"PRIu8"\t", ipsrc_filter[0], ipsrc_filter[1], ipsrc_filter[2], ipsrc_filter[3]);
    if(ipdst_filter[0]!=0)
        printf("ipdst_filter:%"PRIu8".%"PRIu8".%"PRIu8".%"PRIu8"\t", ipdst_filter[0], ipdst_filter[1], ipdst_filter[2], ipdst_filter[3]);

    if (sport_filter!= NO_FILTER) {
        printf("po_filtro=%"PRIu16"\t", sport_filter);
    }

    if (dport_filter != NO_FILTER) {
        printf("pd_filtro=%"PRIu16"\t", dport_filter);
    }

    printf("\n\n");

    do {
        retorno = pcap_next_ex(descr, &hdr, (const u_char **)&pack);

        if (retorno == PACK_READ) { //Todo correcto
            analizar_paquete(hdr, pack);
        
        } else if (retorno == PACK_ERR) { //En caso de error
            printf("Error al capturar un paquetes %s, %s %d.\n", pcap_geterr(descr), __FILE__, __LINE__);
            pcap_close(descr);
            exit(ERROR);

        }
    } while (retorno != TRACE_END);

    printf("Se procesaron %"PRIu64" paquetes.\n\n", contador);
    pcap_close(descr);
    return OK;
}



void analizar_paquete(const struct pcap_pkthdr *hdr, const uint8_t *pack){

    int i = 0;
    uint16_t offset;
    const uint8_t *pack_aux;
    uint8_t version, ihl, protocolo;

    contador++;
    printf ("\nPaquete número %"PRIu64" a las %s", contador, ctime((const time_t*) & (hdr->ts.tv_sec)));

    printf("Direccion ETH destino= ");
    printf("%02X", pack[0]);

    for (i = 1; i < ETH_ALEN; i++) {
        printf(":%02X", pack[i]);
    }
    pack += ETH_ALEN;

    printf("\n");


    printf("Direccion ETH origen = ");
    printf("%02X", pack[0]);

    for (i = 1; i < ETH_ALEN; i++) {
        printf(":%02X", pack[i]);
    }
    pack+=ETH_ALEN;

    printf("\n");

    printf("Tipo Ethernet = 0x%04X\n",htons(*(uint16_t *)pack));

    if (htons(*(uint16_t *)pack)!= 0x0800){
        printf("Ethernet type no es IPv4.\n");
        return;
    }
    pack += ETH_TLEN;

    // Ahora imprimimos los campos de la cabecera de nivel 3 : IPv4
    pack_aux = pack;
    version = (*pack_aux >> 4);
    printf("Version IP = %d\n", version);
    ihl = (*pack_aux & 0x0F)*4; 
    printf("Longitud de cabecera IP = %d bytes\n", ihl);

    pack_aux += 2; // Avanzamos el puntero auxiliar para pasar la version, el ihl y el tipo de servicio
    printf("Longitud total = %d\n", htons(*(uint16_t *)pack_aux));

    pack_aux += 4; // Avanzamos el puntero auxiliar para pasar la longitud total y la identificacion
    offset = (htons(*(uint16_t *)pack_aux) & 0x1FFF);
    printf("Posicion/Desplazamiento = %d\n", offset);

    pack_aux += 2; // Avanzamos el puntero auxiliar para pasar los flags y la posicion
    printf("Tiempo de vida = %d\n", *pack_aux);

    pack_aux += 1; // Avanzamos el puntero auxiliar para pasar el tiempo de vida
    protocolo = *pack_aux;
    printf("Protocolo = %d\n", protocolo);

    pack_aux += 3; // Avanzamos el puntero auxiliar para pasar el protocolo, y la suma de control de cabecera
    printf("Direccion IP Origen = %"PRIu8".%"PRIu8".%"PRIu8".%"PRIu8"\n", pack_aux[0], pack_aux[1], pack_aux[2], pack_aux[3]);

    if (*(uint32_t *)ipsrc_filter != 0x0000){
        for (i=0; i<IP_ALEN; i++) {
            if (ipsrc_filter[i] != pack_aux[i]) {
                printf("La dirección IP de origen no cumple el filtro introducido.\n");
                return;
            } 
        }
    }

    pack_aux += 4; // Avanzamos el puntero auxiliar para pasar la direccion ip de origen 
    printf("Direccion IP Destino = %"PRIu8".%"PRIu8".%"PRIu8".%"PRIu8"\n", pack_aux[0], pack_aux[1], pack_aux[2], pack_aux[3]);

    if (*(uint32_t *)ipdst_filter != 0x0000){
        for (i=0; i<IP_ALEN; i++) {
            if (ipdst_filter[i] != pack_aux[i]) {
                printf("La dirección IP de destino no cumple el filtro introducido.\n");
                return;
            } 
        }
    }

    if (offset != 0x0000) {
        printf("El desplazamiento no es 0 así que este paquete no es el primer fragmento. Pasamos al siguiente paquete.\n");
        return;
    }

    if (protocolo != 0x06 && protocolo != 0x11) {
        printf("El protocolo no es TCP (6) ni UDP (17) así que no nos interesa. Pasamos al siguiente paquete.\n");
        return;
    }

    pack += ihl;

    printf ("Puerto de origen = %"PRIu16"\n", htons(*(uint16_t *)pack));
    if (sport_filter != *pack && sport_filter != NO_FILTER) {
        printf("El puerto de origen no cumple el filtro introducido.\n");
        return;
    }

    pack += 2; //Avanzamos el puntero para pasar el puerto de origen
    printf ("Puerto de destino = %"PRIu16"\n", htons(*(uint16_t *)pack));
    if (dport_filter != *pack && dport_filter != NO_FILTER) {
        printf("El puerto de destino no cumple el filtro introducido.\n");
        return;
    }

    if (protocolo == 0x06){ // El protocolo de nivel 4 es TCP
        pack += 11; //Avanzamos el puntero para leer las banderas del protocolo TCP
        printf ("Bandera ACK de TCP = %x\n", (*pack & 0x10) >> 4);
        printf ("Bandera SYN de TCP = %x\n", (*pack & 0x02) >> 1);
    } else {
        pack += 2; //Avanzamos el puntero para pasar al campo longitud del protocolo UDP
        printf ("Longitud UDP = %"PRIu16"", htons(*(uint16_t *)pack));
    }


    printf("\n\n");
}

