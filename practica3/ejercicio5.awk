{
	i+=1; 
	tiempos[i]=$1;
} 
END {
	for (j=2; j<=i; ++j) {
		printf "%f\n", tiempos[j]-tiempos[j-1];
	}
}