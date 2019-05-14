############################################################################
# saludo.py
#
# Script que recibe un nombre y responde con la cadena "Hola <nombre>!"
#
# Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
############################################################################


#!/usr/bin/python
import sys 

dirFichero = sys.argv[2]
fichero = open(dirFichero, 'w')
if len(sys.argv) != 3:
	fichero.write("Error, se debe introducir dos un argumento de entrada\n")
else:
	campos = []
	campos = sys.argv[1].split('=')
	if (len(campos) != 2) or (campos[0] != "nombre"):
		fichero.write("Error, mal introducidos los argumentos: nombre=valor\n")
	else:
		fichero.write("Hola")
		separados = campos[1].split("+")
		for palabra in separados:
			fichero.write(" " + palabra);
		fichero.write("!\n")
fichero.close()
