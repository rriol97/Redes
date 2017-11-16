#!/bin/bash

if [ ! -s $4 ]; then
  echo -e "El fichero no contiene datos\n"
#Para los tiempos entre llegadas
elif [[ $6 = x* ]]; then
  gnuplot << EOF
  set title "$1"
  set xlabel "$2"
  set ylabel "$3"
  set xrange [0.0001:*]
  set logscale $6
  plot "$4" using 1:2 with steps title "Datos"

  #Para salida a un archivo tipo portable network graphics
  set term jpeg
  set output "$5"
  replot

  set term pngcairo
  set output "$5"
  replot

  # Cierra el archivo de salida
  set output

  quit
EOF
#Para las series temporales
elif [[ $6 = f* ]];then
  gnuplot << EOF
  set title "$1"
  set xlabel "$2"
  set ylabel "$3"
  plot "$4" using 1:2 with lines title "Datos"

  #Para salida a un archivo tipo portable network graphics
  set term jpeg
  set output "$5"
  replot

  set term pngcairo
  set output "$5"
  replot

  # Cierra el archivo de salida
  set output

  quit
EOF
else
  gnuplot << EOF
  set title "$1"
  set xlabel "$2"
  set ylabel "$3"
  plot "$4" using 1:2 with steps title "Datos"

  #Para salida a un archivo tipo portable network graphics
  set term jpeg
  set output "$5"
  replot

  set term pngcairo
  set output "$5"
  replot

  # Cierra el archivo de salida
  set output

  quit
EOF
fi
