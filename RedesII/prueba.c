#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main (){
	int d;
	char buffer[1024] = "fichero.txt";

	d = open (buffer, O_RDONLY);

	if (d < 0){
		printf ("A la mierda\n");
	} else{
		printf ("guaaaauuu\n");
	}
	close (d);
	return 0;
}