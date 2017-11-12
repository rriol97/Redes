{
	contador[$1]+=1;
	total+=1;
	if (($1==2048) || (($3==2048))) {
		acumulador[$2]+=1;
		suma+=1;
	}
}
END {
	for (i in contador) {
		print i,contador[i]/total*100"%";
	}
	print "El porcentaje de paquetes IP es", suma/total*100"%";
	printf "\nEl porcentaje de protocolos sobre IP es el siguiente:\n";
	for (i in acumulador) {
		print i,acumulador[i]/total*100"%";
	}
}
