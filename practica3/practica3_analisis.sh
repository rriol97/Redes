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

if [ ! -d figuras ]
then
	mkdir figuras
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
TEMP1_LV4=./ecdf/tiempos_origen_tcp.txt
TEMP2_LV4=./ecdf/tiempos_destino_tcp.txt
TEMP3_LV4=./ecdf/tiempos_origen_udp.txt
TEMP4_LV4=./ecdf/tiempos_destino_udp.txt
SERIE1=./figuras/caudal_origen.txt
SERIE2=./figuras/caudal_destino.txt
ECDF1_LV2=./ecdf/ecdf_tamanios_origen_lv2.png
ECDF2_LV2=./ecdf/ecdf_tamanios_destino_lv2.png
ECDF1_LV3=./ecdf/ecdf_tamanios_origen_http.png
ECDF2_LV3=./ecdf/ecdf_tamanios_destino_http.png
ECDF3_LV3=./ecdf/ecdf_tamanios_origen_dns.png
ECDF4_LV3=./ecdf/ecdf_tamanios_destino_dns.png
ECDF1_LV4=./ecdf/ecdf_tiempos_origen_tcp.png
ECDF2_LV4=./ecdf/ecdf_tiempos_destino_tcp.png
ECDF3_LV4=./ecdf/ecdf_tiempos_origen_udp.png
ECDF4_LV4=./ecdf/ecdf_tiempos_destino_udp.png
FIG1=./figuras/fig_caudal_origen.png
FIG2=./figuras/fig_caudal_destino.png

# Obtenemos los porcentajes de cada protocolo de nivel 3
echo -e "Porcentaje de protocolos de red"
if [ ! -f $TIPOS ]
then
	tshark -r traza_1302_09.pcap -T fields -e eth.type -e ip.proto -e vlan.etype > $TIPOS
fi
awk -f ejercicio1.awk $TIPOS

# Obtenemos el top 10 de cada una de las categorias que se nos piden
echo -e "\nTOP 10: IP Origen por paquetes"
if [ ! -f $TOPS1 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.src > $TOPS1
fi
sort -n < $TOPS1 | uniq -c | sort -nr | head

echo -e "\nTOP 10: IP Destino por paquetes"
if [ ! -f $TOPS2 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.dst > $TOPS2
fi
sort -n < $TOPS2 | uniq -c | sort -nr | head

echo -e "\nTOP 10: IP Origen por bytes"
if [ ! -f $TOPS3 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.src -e frame.len > $TOPS3
fi
awk -f ejercicio2.awk $TOPS3 | sort -nr | head

echo -e "\nTOP 10: IP Destino por bytes"
if [ ! -f $TOPS4 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.dst -e frame.len > $TOPS4
fi
awk -f ejercicio2.awk $TOPS4 | sort -nr | head

echo -e "\nTOP 10: Puerto origen TCP por paquetes"
if [ ! -f $TOPS5 ]
then
	tshark -r traza_1302_09.pcap -T fields -e tcp.srcport -Y 'tcp' > $TOPS5
fi
sort -n < $TOPS5 | uniq -c | sort -nr | head

echo -e "\nTOP 10: Puerto destino TCP por paquetes"
if [ ! -f $TOPS6 ]
then
	tshark -r traza_1302_09.pcap -T fields -e tcp.dstport -Y 'tcp' > $TOPS6
fi
sort -n < $TOPS6 | uniq -c | sort -nr | head

echo -e "\nTOP 10: Puerto origen TCP por bytes"
if [ ! -f $TOPS7 ]
then
	tshark -r traza_1302_09.pcap -T fields -e tcp.srcport -e frame.len -Y 'tcp' > $TOPS7
fi
awk -f ejercicio2.awk $TOPS7 | sort -nr | head

echo -e "\nTOP 10: Puerto destino TCP por bytes"
if [ ! -f $TOPS8 ]
then
	tshark -r traza_1302_09.pcap -T fields -e tcp.dstport -e frame.len -Y 'tcp' > $TOPS8
fi
awk -f ejercicio2.awk $TOPS8 | sort -nr | head

echo -e "\nTOP 10: Puerto origen UDP por paquetes"
if [ ! -f $TOPS9 ]
then
	tshark -r traza_1302_09.pcap -T fields -e udp.srcport -Y 'udp' > $TOPS9
fi
sort -n < $TOPS9 | uniq -c | sort -nr | head

echo -e "\nTOP 10: Puerto destino UDP por paquetes"
if [ ! -f $TOPS10 ]
then
	tshark -r traza_1302_09.pcap -T fields -e udp.dstport -Y 'udp' > $TOPS10
fi
sort -n < $TOPS10 | uniq -c | sort -nr | head

echo -e "\nTOP 10: Puerto origen UDP por bytes"
if [ ! -f $TOPS11 ]
then
	tshark -r traza_1302_09.pcap -T fields -e udp.srcport -e frame.len -Y 'udp' > $TOPS11
fi
awk -f ejercicio2.awk $TOPS11 | sort -nr | head

echo -e "\nTOP 10: Puerto destino UDP por bytes"
if [ ! -f $TOPS12 ]
then
	tshark -r traza_1302_09.pcap -T fields -e udp.dstport -e frame.len -Y 'udp' > $TOPS12
fi
awk -f ejercicio2.awk $TOPS12 | sort -nr | head


# Obtenemos las ECDF de tamanios que se nos piden
chmod +x realizar_graficas.sh

echo -e "\nECDF de los tamanios de los paquetes (Direccion Mac origen)(Ver grafica)"
if [ ! -f $TAM1_LV2 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.len -Y 'eth.src eq 00:11:88:CC:33:1' > $TAM1_LV2
fi
sort -n < $TAM1_LV2 | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt
./realizar_graficas.sh TamaniosOrigen Tamanios_B Probabilidades temp2.txt $ECDF1_LV2

echo -e "\nECDF de los tamanios de los paquetes (Direccion Mac destino)(Ver grafica)"
if [ ! -f $TAM2_LV2 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.len -Y 'eth.dst eq 00:11:88:CC:33:1' > $TAM2_LV2
fi
sort -n < $TAM2_LV2 | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt
./realizar_graficas.sh TamaniosDestino Tamanios_B Probabilidades temp2.txt $ECDF2_LV2

echo -e "\nECDF de los tamanios de nivel 3 de los paquetes HTTP (Puerto TCP origen)(Ver grafica)"
if [ ! -f $TAM1_LV3 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.len -Y 'tcp.srcport eq 80' > $TAM1_LV3
fi
sort -n < $TAM1_LV3 | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt
./realizar_graficas.sh TamaniosOrigenHTTP Tamanios_B Probabilidades temp2.txt $ECDF1_LV3

echo -e "\nECDF de los tamanios de nivel 3 de los paquetes HTTP (Puerto TCP destino)(Ver grafica)"
if [ ! -f $TAM2_LV3 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.len -Y 'tcp.dstport eq 80' > $TAM2_LV3
fi
sort -n < $TAM2_LV3 | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt
./realizar_graficas.sh TamaniosDestinoHTTP Tamanios_B Probabilidades temp2.txt $ECDF2_LV3

echo -e "\nECDF de los tamanios de nivel 3 de los paquetes DNS (Puerto UDP origen)(Ver grafica)"
if [ ! -f $TAM3_LV3 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.len -Y 'udp.srcport eq 53' > $TAM3_LV3
fi
sort -n < $TAM3_LV3 | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt
./realizar_graficas.sh TamaniosOrigenDNS Tamanios_B Probabilidades temp2.txt $ECDF3_LV3

echo -e "\nECDF de los tamanios de nivel 3 de los paquetes DNS (Puerto UDP destino)(Ver grafica)"
if [ ! -f $TAM4_LV3 ]
then
	tshark -r traza_1302_09.pcap -T fields -e ip.len -Y 'udp.dstport eq 53' > $TAM4_LV3
fi
sort -n < $TAM4_LV3 | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt
./realizar_graficas.sh TamaniosDestinoDNS Tamanios_B Probabilidades temp2.txt $ECDF4_LV3

# Obtenemos la serie temporal del caudal
echo -e "\nSerie del caudal en b/s (Direccion MAC origen)(Ver grafica)"
if [ ! -f $SERIE1 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.time_relative -e frame.len -Y 'eth.src eq 00:11:88:CC:33:1' > $SERIE1
fi
awk -f ejercicio4.awk $SERIE1 | sort -n > temp1.txt
./realizar_graficas.sh CaudalOrigen Tiempos_s Tamanio_b temp1.txt $FIG1

echo -e "\nSerie del caudal en b/s (Direccion MAC destino)(Ver grafica)"
if [ ! -f $SERIE2 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.time_relative -e frame.len -Y 'eth.dst eq 00:11:88:CC:33:1' > $SERIE2
fi
awk -f ejercicio4.awk $SERIE2 | sort -n > temp1.txt
./realizar_graficas.sh CaudalDestino Tiempos_s Tamanio_b temp1.txt $FIG2

# Obtenemos las ECDF de los tiempos entre llegadas que se nos piden
echo -e "\nECDF de los tiempos entre llegadas del flujo TCP (Direccion IP origen)(Ver grafica)"
if [ ! -f $TEMP1_LV4 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.time_relative -Y 'ip.src eq 81.84.126.202' > $TEMP1_LV4
fi
awk -f ejercicio5.awk $TEMP1_LV4 | sort -n | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt

./realizar_graficas.sh TiemposOrigenTCP Tiempos_s Probabilidades temp2.txt $ECDF1_LV4 x

echo -e "\nECDF de los tiempos entre llegadas del flujo TCP (Direccion IP destino)(Ver grafica)"
if [ ! -f $TEMP2_LV4 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.time_relative -Y 'ip.dst eq 81.84.126.202' > $TEMP2_LV4
fi
awk -f ejercicio5.awk $TEMP2_LV4 | sort -n | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt

./realizar_graficas.sh TiemposDestinoTCP Tiempos_s Probabilidades temp2.txt $ECDF2_LV4 x

echo -e "\nECDF de los tiempos entre llegadas del flujo UDP (Puerto UDP origen)(Ver grafica)"
if [ ! -f $TEMP3_LV4 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.time_relative -Y 'udp.srcport eq 54771' > $TEMP3_LV4
fi
awk -f ejercicio5.awk $TEMP3_LV4 | sort -n | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt
#./realizar_graficas.sh TiemposOrigenUDP Tiempos_s Probabilidades temp2.txt $ECDF3_LV4 x

echo -e "\nECDF de los tiempos entre llegadas del flujo UDP (Puerto UDP destino)(Ver grafica)"
if [ ! -f $TEMP4_LV4 ]
then
	tshark -r traza_1302_09.pcap -T fields -e frame.time_relative -Y 'udp.dstport eq 54771' > $TEMP4_LV4
fi
awk -f ejercicio5.awk $TEMP4_LV4 | sort -n | uniq -c > temp1.txt
awk -f ejercicio3.awk temp1.txt | sort -n > temp2.txt
./realizar_graficas.sh TiemposDestinoUDP Tiempos_s Probabilidades temp2.txt $ECDF4_LV4 x


#rm -f temp?.txt
