##############################################################################
# enc_sig.py
#
# Implementacion de las funciones referentes al cifrado y firmado de mensajes
#
# Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
##############################################################################


''' Importacion de modulos '''

from Cryptodome.Signature import pss
from Cryptodome.Hash import SHA256
from Cryptodome.PublicKey import RSA
from Cryptodome.Cipher import AES, PKCS1_OAEP
from Cryptodome.Random import get_random_bytes
from Cryptodome.Util.Padding import pad, unpad
from constantes import LF_DIR
import os


''' Implementacion de funciones '''

########
# FUNCIÓN: generar_clave_rsa(fichero)
# ARGS_IN:  fichero: nombre del fichero donde escribir la clave privada
# DESCRIPCIÓN: Funcion que genera una clave RSA (par publica-privada) 
#			   y escribe la privada en un fichero
# ARGS_OUT: public_key - devuelve la clave publica en formato PEM
#########
def generar_clave_rsa(fichero):
	# generamos clave RSA de longitud 2048 bits
	key = RSA.generate(2048)
	
	private_key = key.exportKey() # Clave privada en formato PEM
	public_key = key.publickey().exportKey() # Clave publica en formato PEM

	file_out = open(fichero, 'wb')
	file_out.write(private_key)
	file_out.close()

	return public_key


########
# FUNCIÓN: firmar(mensaje, fichero, path)
# ARGS_IN:  mensaje: texto a firmar
#			fichero: nombre del fichero donde se encuentra la clave privada 
#					 del usuario que firma
#			path: nombre del fichero que contiene el texto a firmar
# DESCRIPCIÓN: Funcion que firma un mensaje con la clave privada del usuario
# ARGS_OUT: nombre - devuelve el nombre del fichero firmado
#########
def firmar(mensaje, fichero, path):
	# Comprobamos si el directorio de ficheros locales existe
	if not os.path.exists(LF_DIR): 
		os.makedirs(LF_DIR)	# Lo creamos si no existe

	# Abrimos el fichero donde firmaremos
	nombre = 'sig_' + path
	fichero_firmado = open(LF_DIR+'/'+nombre, 'wb')
	# Obtenemos la clave privada del usuario
	try:
		key = RSA.import_key(open(fichero, 'rb').read())
	except (ValueError, IndexError, TypeError):
		print('Error parseando la clave a importar')
		return 
	# Pasamos el texto por la funcion hash SHA256
	h = SHA256.new(mensaje)
	# Obtenemos la firma y la escribimos al comienzo del fichero resultado
	try:
		fichero_firmado.write(pss.new(key).sign(h))
	except (ValueError):
		print('Error firmando el mensaje: La clave RSA no es suficientemente larga para el algoritmo hash utilizado')
		return 
	except (TypeError):
		print('Error firmando el mensaje: La clave RSA no tiene una mitad privada')
		return
	# Escribimos el mensaje original despues de la firma
	fichero_firmado.write(mensaje)
	fichero_firmado.close()

	return nombre


	

########
# FUNCIÓN: cifrar_aes(mensaje, clave_publica, path)
# ARGS_IN:  mensaje: texto a cifrar
#			clave_publica: clave publica del usuario al que enviamos
#			path: nombre del fichero que contiene el texto a cifrar
# DESCRIPCIÓN: Funcion que cifra un mensaje usando AES con modo de encadenamiento 
#			   CBC, con IV de 16 bytes, y longitud de clave de 256 bits;
#			   y tambien cifra la clave simetrica con RSA
# ARGS_OUT: nombre - devuelve el nombre del fichero cifrado
#########
def cifrar_aes(mensaje, clave_publica, path):
	# Comprobamos si el directorio de ficheros locales existe
	if not os.path.exists(LF_DIR): 
		os.makedirs(LF_DIR)  # Lo creamos si no existe
	
	# Abrimos el fichero donde cifraremos
	nombre = 'enc_' + path
	fichero_cifrado = open(LF_DIR+'/'+nombre, 'wb')
	# Obtenemos una clave simetrica aleatoria de 32 bytes
	session_key = get_random_bytes(32)

	# Ciframos la clave simetrica con RSA usando la clave publica del receptor
	# (obtenemos por tanto, el sobre digital)
	cipher_rsa = PKCS1_OAEP.new(clave_publica)
	try:
		sobre = cipher_rsa.encrypt(session_key)
	except (ValueError):
		print('Error cifrando la clave simetrica')
		return 

	# Obtenemos un vector de inicializacion de 16 bytes
	iv = get_random_bytes(16)
	cipher_aes = AES.new(session_key, AES.MODE_CBC, iv)

	# Escribimos el sobre primero y luego el vector en el fichero cifrado
	fichero_cifrado.write(sobre)
	fichero_cifrado.write(iv)
	# Hacemos padding del mensaje a 16 bytes para poder emplear el modo CBC
	mensaje16 = pad(mensaje, 16, style='pkcs7')
	# Ciframos con AES y escribimos el mensaje cifrado en el fichero que devolveremos
	try:
		fichero_cifrado.write(cipher_aes.encrypt(mensaje16))
	except (ValueError):
		print('Error cifrando el mensaje')
		return 
	fichero_cifrado.close()
	return nombre


########
# FUNCIÓN: descifrar_aes(mensaje, fichero)
# ARGS_IN:  mensaje: texto a descifrar
#			fichero: nombre del fichero donde esta la clave privada del usuario
# DESCRIPCIÓN: Funcion que descifra un mensaje cifrado con AES con modo de encadenamiento 
#			   CBC, con IV de 16 bytes, y longitud de clave de 256 bits;
# 			   y tambien descifra la clave de sesion cifrada con RSA
# ARGS_OUT: texto_claro - devuelve el texto descifrado (con la firma al comienzo)
#########
def descifrar_aes(mensaje, fichero):
	# Obtengo mi clave privada 
	clave_privada = RSA.import_key(open(fichero, 'rb').read())
	
	# Saco el sobre (sabemos que ocupa 256 bytes)
	sobre = mensaje[:256]

	# Obtengo la clave simetrica descifrando el sobre
	cipher_rsa = PKCS1_OAEP.new(clave_privada)
	try:
		session_key = cipher_rsa.decrypt(sobre)
	except (ValueError):
		print('Error descifrando el sobre digital: La longitud del texto cifrado es incorrecta')
		return 	
	except (TypeError):
		print('Error descifrando el sobre digital: La clave RSA no tiene una mitad privada')
		return

	# Obtengo el vector de inicializacion (16 bytes) y descifro el mensaje
	iv = mensaje[256:256+16]
	cipher_aes = AES.new(session_key, AES.MODE_CBC, iv)
	try:
		texto16 = cipher_aes.decrypt(mensaje[256+16:])
	except (ValueError, TypeError):
		print('Error descifrando el mensaje')
		return 

	# Deshago el padding hecho en el mensaje descifrado 
	try:
		texto_claro = unpad(texto16, 16, style='pkcs7')
	except (ValueError):
		print('Error eliminando el padding del texto descifrado')
		return 

	return texto_claro


########
# FUNCIÓN: comprobar_firma(mensaje, clave_publica)
# ARGS_IN:  mensaje: texto con la firma al comienzo
#			clave_publica: clave publica del emisor del mensaje
# DESCRIPCIÓN: Funcion que comprueba si la firma de un mensaje es correcta
# ARGS_OUT: True si la firma es correcta, False si no
#########
def comprobar_firma(mensaje, clave_publica):
	# Saco la firma (sabemos que es de 256 bytes)
	firma = mensaje[:256]

	# Calculo el hash del mensaje
	h = SHA256.new(mensaje[256:])

	# Compruebo si la firma es correcta o no
	try:
		pss.new(clave_publica).verify(h, firma)
	except (ValueError):
	    return False

	return True
