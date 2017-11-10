{
	contador[$1]+=1;
	total+=1; 
	if (($1==2048) || ($2==2048)) {
		suma+=1;
	}
}
END {
	for (i in contador) {
		print i,contador[i]/total*100"%"; 
	}
	print "El porcentaje de paquetes IP es", suma/total*100"%";
}
