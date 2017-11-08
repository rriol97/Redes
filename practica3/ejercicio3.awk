	{tot+=$1; contador[$2]=tot;} 
END {for (i in contador) print i, contador[i]/tot}