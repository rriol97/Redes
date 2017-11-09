#!/bin/bash

# Creacion de directorios para guardar los ficheros de texto en caso necesario
if [ ! -d tipos ]
then
	mkdir tipos
fi

if [ ! -d tops ]
then
	mkdir tops
fi

if [ ! -d ecdf ]
then
	mkdir ecdf
fi

# Guardamos el nombre de cada fichero de texto en una variable
TIPOS=./tipos/tipos.txt
TOPS1=./tops/ip_src_p.txt
TOPS2=./tops/ip_dst_p.txt
TOPS3=./tops/ip_src_b.txt
TOPS4=./tops/ip_dst_b.txt
TOPS5=./tops/src_port_tcp_p.txt
TOPS6=./tops/dst_port_tcp_p.txt
TOPS7=./tops/src_port_tcp_b.txt
TOPS8=./tops/dst_port_tcp_b.txt
TOPS9=./tops/src_port_udp_p.txt
TOPS10=./tops/dst_port_udp_p.txt
TOPS11=./tops/src_port_udp_b.txt
TOPS12=./tops/dst_port_udp_b.txt
TAM1_LV2=./ecdf/tamanios_origen_lv2.txt
TAM2_LV2=./ecdf/tamanios_destino_lv2.txt
TAM1_LV3=./ecdf/tamanios_origen_http.txt
TAM2_LV3=./ecdf/tamanios_destino_http.txt
TAM3_LV3=./ecdf/tamanios_origen_dns.txt
TAM4_LV3=./ecdf/tamanios_destino_dns.txt
ECDF1_LV2=./ecdf/tamanios_origen_lv2_ecdf.png
ECDF2_LV2=./ecdf/tamanios_destino_lv2_ecdf.png
ECDF1_LV3=./ecdf/tamanios_origen_http_ecdf.png
ECDF2_LV3=./ecdf/tamanios_destino_http_ecdf.png
ECDF3_LV3=./ecdf/tamanios_origen_dns_ecdf.png
ECDF4_LV3=./ecdf/tamanios_destino_dns_ecdf.png

# Obtenemos los porcentajes de cada protocolo de nivel 3
echo -e "Porcentaje de protocolos de red"
if [ ! -e $TIPOS ]
then
	tshark -r traza_1302_09.pcap -T fields -e eth.type -e vlan.etype > $TIPOS
fi
awk -f ejercicio1.awk $TIPOS
awk -f ejer1.awk $TIPOS

# Obtenemos el top 10 de cada una de las categorias que se nos piden
echo -e "\nTOP 10: IP Origen por paquetes"
if [ ! -e $TOPS1 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.src > $TOPS1
fi
sort -n < $TOPS1 | uniq -c | sort -nr | head

echo -e "\nTOP 10: IP Destino por paquetes"
if [ ! -e $TOPS2 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.dst > $TOPS2
fi
sort -n < $TOPS2 | uniq -c | sort -nr | head

echo -e "\nTOP 10: IP Origen por bytes"
if [ ! -e $TOPS3 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.src -e frame.len > $TOPS3
fi
awk -f ejercicio2.awk $TOPS3 | sort -nr | head

echo -e "\nTOP 10: IP Destino por bytes"
if [ ! -e $TOPS4 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.dst -e frame.len > $TOPS4
fi
awk -f ejercicio2.awk $TOPS4 | sort -nr | head

echo -e "\nTOP 10: Puerto origen TCP por paquetes"
if [ ! -e $TOPS5 ]
then
	tshark -r traza_1302_09.pcap -T fields -e tcp.srcport -Y 'tcp' > $TOPS5
fi
sort -n < $TOPS5 | uniq -c | sort -nr | head

echo -e "\nTOP 10: Puerto destino TCP por paquetes"
if [ ! -e $TOPS6 ]
then
	tshark -r traza_1302_09.pcap -T fields -e tcp.dstport -Y 'tcp' > $TOPS6
fi
sort -n < $TOPS6 | uniq -c | sort -nr | head

echo -e "\nTOP 10: Puerto origen TCP por bytes"
if [ ! -e $TOPS7 ]
then
	tshark -r traza_1302_09.pcap -T fields -e tcp.srcport -e frame.len -Y 'tcp' > $TOPS7
fi
awk -f ejercicio2.awk $TOPS7 | sort -nr | head

echo -e "\nTOP 10: Puerto destino TCP por bytes"
if [ ! -e $TOPS8 ]
then
	tshark -r traza_1302_09.pcap -T fields -e tcp.dstport -e frame.len -Y 'tcp' > $TOPS8
fi
awk -f ejercicio2.awk $TOPS8 | sort -nr | head

echo -e "\nTOP 10: Puerto origen UDP por paquetes"
if [ ! -e $TOPS9 ]
then
	tshark -r traza_1302_09.pcap -T fields -e udp.srcport -Y 'udp' > $TOPS9
fi
sort -n < $TOPS9 | uniq -c | sort -nr | head

echo -e "\nTOP 10: Puerto destino UDP por paquetes"
if [ ! -e $TOPS10 ]
then
	tshark -r traza_1302_09.pcap -T fields -e udp.dstport -Y 'udp' > $TOPS10
fi
sort -n < $TOPS10 | uniq -c | sort -nr | head

echo -e "\nTOP 10: Puerto origen UDP por bytes"
if [ ! -e $TOPS11 ]
then
	tshark -r traza_1302_09.pcap -T fields -e udp.srcport -e frame.len -Y 'udp' > $TOPS11
fi
awk -f ejercicio2.awk $TOPS11 | sort -nr | head

echo -e "\nTOP 10: Puerto destino UDP por bytes"
if [ ! -e $TOPS12 ]
then
	tshark -r traza_1302_09.pcap -T fields -e udp.dstport -e frame.len -Y 'udp' > $TOPS12
fi
awk -f ejercicio2.awk $TOPS12 | sort -nr | head


# Obtenemos las ECDF que se nos piden
echo -e "\nECDF de los tamanios de los paquetes (Mac origen)(Ver grafica)"
if [ ! -e $TAM1_LV2 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.len -Y 'eth.src eq 00:11:88:CC:33:1' > $TAM1_LV2
fi
sort -n < $TAM1_LV2 | uniq -c | sort -nk 2 > temp.txt
awk -f ejercicio3.awk temp.txt | sort -n > tamanios_origen.txt
chmod +x realizar_graficas.gp
./realizar_graficas.gp TamaniosOrigen Tamanios Probabilidades tamanios_origen.txt $ECDF1_LV2
rm -f tamanios_origen.txt

echo -e "\nECDF de los tamanios de los paquetes (Mac destino)(Ver grafica)"
if [ ! -e $TAM2_LV2 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.len -Y 'eth.dst eq 00:11:88:CC:33:1' > $TAM2_LV2
fi
sort -n < $TAM2_LV2 | uniq -c | sort -nk 2 > temp.txt
awk -f ejercicio3.awk temp.txt | sort -n > tamanios_destino.txt
chmod +x realizar_graficas.gp
./realizar_graficas.gp TamaniosDestino Tamanios Probabilidades tamanios_destino.txt $ECDF2_LV2
rm -f tamanios_destino.txt
rm -f temp.txt
