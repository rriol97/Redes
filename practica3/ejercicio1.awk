BEGIN {suma=0;i = 0}
	{contador[$1]+=1;suma+=1}
END {for (i in contador)print i, contador[i]/suma*100"%"}
