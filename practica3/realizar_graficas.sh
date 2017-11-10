#!/bin/bash

if [ -e $6 ]; then
  gnuplot << EOF
  set title "$1"
  set xlabel "$2"
  set ylabel "$3"
  set logscale "$6"
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
