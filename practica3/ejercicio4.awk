{indice = int($1); contador[indice]+=$2; }
END {for (i in contador) printf "%d %d\n", i, contador[i]*8}
