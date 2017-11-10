{
	tot+=$1;
	contador[$2]=tot;
} 
END {
	for (i in contador) {
		printf "%f %f\n", i,contador[i]/tot;
	}
}