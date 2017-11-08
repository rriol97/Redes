
	{contador[$1]+=$2; }
END {for (i in contador) printf "%10d %s\n", contador[i],i}
