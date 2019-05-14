############################################################################
# conversor.py
#
# Script que recibe una temperatura en grados Celsius y la devuelve
# en Farenheit.
#
# Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
############################################################################

#!/usr/bin/python

import sys
dirFichero = sys.argv[2]
fichero = open(dirFichero, 'w')
if len(sys.argv) != 3:
	fichero.write("Error, se debe introducir solo dos argumento de entrada\n")
else:	
	campos = []
	campos = sys.argv[1].split('=')
	if (len(campos) != 2) or (campos[0] != "temp"):
		fichero.write("Error, mal introducidos los argumentos: temp=valor\n")
	else:
		fichero.write("Grados Farenheit: " + str(float(campos[1])*1.8+32)+ '\n')

fichero.close()
