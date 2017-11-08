
awk '{contador[$1]+=1;suma+=1} END{for (i in contador) print i,contador[i]/suma*100"%"}' tipos.txt 
