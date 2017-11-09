#!/bin/bash

gnuplot << EOF
#set data style points
set title "$1"
set xlabel "$2"
set ylabel "$3"
plot "$4" using 1:2 with steps title "Datos"

#Para salida a un archivo tipo portable network graphics
#set term jpeg
#set output "$5"
#replot

set term pngcairo
set output "$5"
replot

#set term svg
#set output "$5.svg"
#replot

# Cierra el archivo de salida
set output

quit
EOF
