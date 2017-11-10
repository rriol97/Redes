{
	indice = int($1);
	contador[indice]+=$2; 
}
END {
	for (i in contador) {
		print i,contador[i]*8;
	}
}
