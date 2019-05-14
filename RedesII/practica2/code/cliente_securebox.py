##############################################################################
# cliente_securebox.py
#
# Implementacion de la interfaz de un cliente que consume el 
# servicio SecureBox de un modo seguro
#
# Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
##############################################################################
 
''' Importacion de modulos '''

import argparse
import enc_sig
import json
import requests
import os
from constantes import PATH, TOKEN, LF_DIR, D_DIR
from Cryptodome.PublicKey import RSA 

''' Definicion de clases '''

##########
###
### Clase que permite controlar los errores devueltos por 
### el servidor
###
##########
class RequestError(Exception):
	def __init__(self, valor,codigo, descr):
		self.valor = valor
		self.codigo = codigo
		self.descr = descr


''' Implementacion de funciones '''

########
# FUNCIÓN: create_id(nombre, correo, token)
# ARGS_IN:  nombre: nombre del usuario que se crea
#			correo: correo del usuario que se crea
#			token: token de autenticacion del usuario loggeado
# DESCRIPCIÓN: Funcion que registra un usuario en el servidor
# ARGS_OUT: -
#########
def create_id (nombre, correo, token):
	#Generamos la clave publica del cliente
	clave = enc_sig.generar_clave_rsa('rsa_key_emisor.bin')  
	print('Generando par de claves RSA de 2048 bits...', end='')
	url = PATH+'/users/register'
	args = {'nombre':nombre, 'email':correo, 'publicKey': clave}
	head = {'Authorization': 'Bearer ' + token}
	#Hacemos la peticion POST al servidor
	r = requests.post(url, headers=head, json=args)
	d = json.loads(r.text)
	#Controlamos los posibles errores devueltos por el servidor
	if (r.status_code!= 200):
		try:
			raise RequestError(d['http_error_code'], d['error_code'],  d['description'])
		except RequestError as e:
			print('RequestError: ' + str(e.valor)+' '+e.codigo+' '+e.descr)
			return 
	print('OK')
	#Buscamos el usuario en el servidor para encontrar su ID
	url = PATH+'/users/search'
	data_search = {'data_search': correo}
	head = {'Authorization': 'Bearer ' + token}
	rest= requests.post(url, headers=head, json=data_search)	
	dic = json.loads(rest.text)
	for usu in dic:
		if (str(d['ts']).split('.')[0] == str(usu['ts']).split('.')[0]):
			id =  usu['userID']
	print('Identidad #'+id+ ' creada correctemente')

########
# FUNCIÓN: search_id(cadena, token)
# ARGS_IN:  cadena: texto a buscar en nombre o correo de los usuarios
#			token: token de autenticacion del usuario loggeado
# DESCRIPCIÓN: Funcion que busca usuarios del servidor
# ARGS_OUT: -
#########
def search_id (cadena, token):
	print('Buscando usuario '+cadena+' en el servidor...', end='')
	url = PATH+'/users/search'
	data_search = {'data_search': cadena}
	head = {'Authorization': 'Bearer ' + token}
	r = requests.post(url, headers=head, json=data_search)
	d = json.loads(r.text)

	if (r.status_code!=200):
		try:
			raise RequestError(d['http_error_code'], d['error_code'],  d['description'])
		except RequestError as e:
			print('RequestError: ' + str(e.valor)+' '+e.codigo+' '+e.descr)
			return 

	if (len(d) == 0):
		try:
			raise RequestError(401, 'USER_ID2', 'No se ha encontrado el usuario con los datos proporcionados para la busqueda con la funcion search')
		except RequestError as e:
			print('RequestError: ' + str(e.valor)+' '+e.codigo+' '+e.descr)
			return
	i = 1
	print('OK')
	print(str(len(d))+' usuarios encontrados:')
	for usu in d:
		q = '[%d] %s, %s, ID: %s' % (i, usu['nombre'], usu['email'], usu['userID'])
		i = i + 1
		print(q)


########
# FUNCIÓN: obtener_clave_publica(id, token)
# ARGS_IN:  id: identificador del usuario del que queremos su clave publica
#			token: token de autenticacion del usuario loggeado
# DESCRIPCIÓN: Funcion que obtiene la clave publica de un cliente
# ARGS_OUT: clave publica del usuario buscado
#########
def obtener_clave_publica (id, token):
	print('-> Recuperando clave publica de ID '+id+'...', end='')
	url = PATH+'/users/getPublicKey'
	public_key = {'userID': id}
	head = {'Authorization': 'Bearer ' + token}
	r = requests.post(url, headers=head, json=public_key)
	d = json.loads(r.text)

	if (r.status_code != 200):
		try:
			raise RequestError(d['http_error_code'], d['error_code'],  d['description'])
		except RequestError as e:
			print('RequestError: ' + str(e.valor)+' '+e.codigo+' '+e.descr)
			return 	

	print('OK')
	return RSA.import_key(d['publicKey'])

########
# FUNCIÓN: delete_id(id, token)
# ARGS_IN:  id: identificador del usuario que queremos eliminar
#			token: token de autenticacion del usuario loggeado
# DESCRIPCIÓN: Funcion que elimina un cliente del servidor
# ARGS_OUT: -
#########
def delete_id(id, token):
	print('Solicitud de borrado del usuario '+id+'...', end='')
	url = PATH+'/users/delete'
	identificador = {'userID': id}
	head = {'Authorization': 'Bearer ' + token}
	r = requests.post(url, headers=head, json=identificador)

	d = json.loads(r.text)
	if (r.status_code != 200):
		
		try:
			raise RequestError(d['http_error_code'], d['error_code'],  d['description'])
		except RequestError as e:
			print('RequestError: ' + str(e.valor)+' '+e.codigo+' '+e.descr)
			return 	

	print('OK')
	print('El usuario', d['userID'], 'ha sido eliminado correctamente')



########
# FUNCIÓN: upload(fichero, destinatario, token)
# ARGS_IN:  fichero: nombre del fichero que se quiere subir al servidor
#			destinatario: identificador del usuario que queremos que reciba el fichero
#			token: token de autenticacion del usuario loggeado
# DESCRIPCIÓN: Funcion que sube un fichero firmado y cifrado al servidor
# ARGS_OUT: -
#########
def upload(fichero, destinatario, token):
	print('Solicitado envio de fichero a SecureBox')
	fichero_firmado, fichero_cifrado = firmar_cifrar_fichero(fichero, destinatario, token, upload=True)
	print('Subiendo fichero a servidor...', end='')
	url = PATH+'/files/upload'
	head = {'Authorization': 'Bearer ' + token}
	r = requests.post(url, headers=head, files={'ufile':open(LF_DIR+'/'+fichero_cifrado, 'rb')})
	d = json.loads(r.text)
	if (r.status_code != 200):
		try:
			raise RequestError(d['http_error_code'], d['error_code'],  d['description'])
		except RequestError as e:
			print('RequestError: ' + str(e.valor)+' '+e.codigo+' '+e.descr)
			return 	

	# Despues de subir el fichero, eliminamos los intermedios
	os.remove(LF_DIR+'/'+fichero_firmado)
	os.remove(LF_DIR+'/'+fichero_cifrado)
	print('OK')
	print('Subida realizada correctamente, ID del fichero:', d['file_id'])


########
# FUNCIÓN: download(id_fichero, token, emisor='')
# ARGS_IN:  id_fichero: identificador del fichero que se quiere descargar del servidor
#			token: token de autenticacion del usuario loggeado
#			emisor: identificador del emisor (por defecto sera el nuestro si no)
# DESCRIPCIÓN: Funcion que descarga un fichero, lo descifra y comprueba la firma
# ARGS_OUT: -
#########
def download(id_fichero, token, emisor=''):
	print('Descargando fichero de SecureBox...', end='')
	url = PATH+'/files/download'
	head = {'Authorization': 'Bearer ' + token}
	args = {'file_id': id_fichero} 
	r = requests.post(url, headers=head, json=args)
	descarga = r.content
	if (r.status_code != 200):
		try:
			raise RequestError(r.headers['http_error_code'], r.headers['error_code'],  r.headers['description'])
		except RequestError as e:
			print('RequestError: ' + str(e.valor)+' '+e.codigo+' '+e.descr)
			return
	print('OK')
	print('->', r.headers['content-length'], 'bytes descargados correctamente')

	print('Descifrando fichero...', end='')
	mensaje_con_firma = enc_sig.descifrar_aes(descarga, 'rsa_key_emisor.bin')
	print('OK')

	if emisor == '':  # Caso en el que no se introduce --source_id, usamos el nuestro
		try:
			clave_publica = RSA.import_key(open('rsa_key_emisor.bin', 'rb').read()).publickey()
		except (ValueError, IndexError, TypeError):
			print('Error parseando la clave a importar')
			return 
	else:
		clave_publica = obtener_clave_publica(emisor, token)

	print('-> Verificando firma...', end='')
	if enc_sig.comprobar_firma(mensaje_con_firma, clave_publica):
		print('OK')
		# Creamos la copia con el contenido del fichero ya descifrado
		nombre = extraer_nombre_fichero(r.headers['content-disposition'])
		if not os.path.exists(D_DIR): 
			os.makedirs(D_DIR)
		open(D_DIR+'/'+nombre, 'wb').write(mensaje_con_firma[256:])
		print('Fichero descargado y verificado correctamente')
	else:
		print('Error de autenticacion: La firma del fichero no es valida')
	


########
# FUNCIÓN: list_files(token)
# ARGS_IN:  token: token de autenticacion del usuario loggeado
# DESCRIPCIÓN: Funcion que lista los ficheros del servidor subidos por nosotros
# ARGS_OUT: -
#########
def list_files(token):
	print('Buscando lista de ficheros...', end='')
	url = PATH+'/files/list'
	head = {'Authorization': 'Bearer ' + token}
	r = requests.post(url, headers=head,json={})
	d = d = json.loads(r.text)
	if (r.status_code != 200):
		try:
			raise RequestError(d['http_error_code'], d['error_code'],  d['description'])
		except RequestError as e:
			print('RequestError: ' + str(e.valor)+' '+e.codigo+' '+e.descr)
			return 	
	i = 1	
	print('OK')	
	print('Numero de ficheros:', d['num_files'])
	for file in d['files_list']:
		q = '[%d] %s, %s' % (i, file['fileID'], file['fileName'])
		print(q)
		i = i + 1



########
# FUNCIÓN: delete_file(id_fichero, token)
# ARGS_IN:  id_fichero: identificador del fichero que queremos eliminar
#			token: token de autenticacion del usuario loggeado
# DESCRIPCIÓN: Funcion que elimina un fichero del servidor
# ARGS_OUT: -
#########
def delete_file (id_fichero, token):	
	print('Borrando fichero: '+id_fichero+'...', end='')
	url = PATH+'/files/delete'
	args = {'file_id': id_fichero}
	head = {'Authorization': 'Bearer ' + token}
	r = requests.post(url, headers=head,json=args)

	if (r.status_code != 200):
		d = d = json.loads(r.text)
		try:
			raise RequestError(d['http_error_code'], d['error_code'],  d['description'])
		except RequestError as e:
			print('RequestError: ' + str(e.valor)+' '+e.codigo+' '+e.descr)
			return 	
	print('OK')
	print('Eliminado el fichero', id_fichero)



########
# FUNCIÓN: firmar_fichero(fichero, upload=False)
# ARGS_IN:  fichero: nombre del fichero que queremos firmar
#			upload: indicador de si estamos firmando en medio de un upload o no
# DESCRIPCIÓN: Funcion que firma un fichero
# ARGS_OUT: fichero_firmado: nombre del fichero donde se ha firmado
#########
def firmar_fichero(fichero, upload=False):
	if not upload:
		print('Firmado local de fichero')
	print('-> Firmando fichero...', end='')
	mensaje = open(fichero, 'rb').read()
	if upload:
		fichero_firmado = enc_sig.firmar(mensaje, 'rsa_key_emisor.bin', 'u_'+fichero)
	else:
		fichero_firmado = enc_sig.firmar(mensaje, 'rsa_key_emisor.bin', fichero)
	print('OK')
	if not upload:
		print('Firmado local realizado correctamente en '+LF_DIR+'/'+fichero_firmado)
	return fichero_firmado

########
# FUNCIÓN: cifrar_fichero(fichero, upload=False)
# ARGS_IN:  fichero: nombre del fichero que queremos cifrar
#			clave_publica: clave publica del usuario al que enviamos 
#			upload: indicador de si estamos cifrando en medio de un upload o no
#			firmado: indicador de si estamos cifrando tras haber firmado o no
# DESCRIPCIÓN: Funcion que cifra un fichero
# ARGS_OUT: fichero_cifrado: nombre del fichero donde se ha cifrado
#########
def cifrar_fichero(fichero, clave_publica, upload=False, firmado=False):
	if not upload:
		print('Cifrado local de fichero')
	print('-> Cifrando fichero...', end='')
	if firmado:
		mensaje = open(LF_DIR+'/'+fichero, 'rb').read()
	else:
		mensaje = open(fichero, 'rb').read()

	if upload:
		fichero_cifrado = enc_sig.cifrar_aes(mensaje, clave_publica, 'u_'+fichero)
	else:
		fichero_cifrado = enc_sig.cifrar_aes(mensaje, clave_publica, fichero)
	print('OK')
	if not upload:
		print('Cifrado local realizado correctamente en '+LF_DIR+'/'+fichero_cifrado)
	return fichero_cifrado


########
# FUNCIÓN: firmar_cifrar_fichero(fichero, destinatario, token, upload=False)
# ARGS_IN:  fichero: nombre del fichero que queremos firmar y cifrar
#			destinatario: identificador del usuario del que queremos su clave publica
#			token: token de autenticacion del usuario loggeado
#			upload: indicador de si estamos cifrando en medio de un upload o no
# DESCRIPCIÓN: Funcion que firma y cifra un fichero
# ARGS_OUT: fichero_firmado: nombre del fichero donde se ha firmado
#			fichero_cifrado: nombre del fichero donde se ha cifrado
#########
def firmar_cifrar_fichero(fichero, destinatario, token, upload=False):
	clave_publica = obtener_clave_publica(destinatario, token)
	fichero_firmado = firmar_fichero(fichero, upload)
	fichero_cifrado = cifrar_fichero(fichero_firmado, clave_publica, upload, firmado=True)
	return [fichero_firmado, fichero_cifrado]


########
# FUNCIÓN: leer_token(fichero)
# ARGS_IN:  fichero: nombre del fichero que contiene el token del usuario
# DESCRIPCIÓN: Funcion que obtiene el token del usuario desde un fichero
# ARGS_OUT: token: token del usuario loggeado
#########
def leer_token(fichero):
	f = open(fichero)
	token = f.read()	
	f.close()
	return token


########
# FUNCIÓN: extraer_nombre_fichero(hdr)
# ARGS_IN:  hdr: cabecera del tipo 'Content-Disposition'
# DESCRIPCIÓN: Funcion que extrae el nombre del fichero que se descarga
# ARGS_OUT: cortes[1]: nombre del fichero que esta subido al servidor 
#					   y queremos descargar
#########
def extraer_nombre_fichero (hdr):
	# Cortamos la cabecera content-disposition por comillas dobles
	cortes = hdr.split('"')
	# El nombre del fichero esta entre las dos primeras comillas
	return cortes[1] 




''' FUNCIONALIDAD COMO TAL DEL CLIENTE '''
token = leer_token(TOKEN)
parser = argparse.ArgumentParser()
parser.add_argument('-c_id', '--create_id', help='Inicializa un cliente en el sistema: (nombre, correo)', nargs = 2)
parser.add_argument('-s_id', '--search_id', help='Busca un el ID asociado a un cliente', nargs = 1)
parser.add_argument('-dlt_id', '--delete_id', help='Elimina el cliente con la ID determinada', nargs = 1)
parser.add_argument('-u', '--upload', help='Sube el fichero con al receptor especificado (indicar receptor)', nargs = 1)
parser.add_argument('-dst_id', '--dest_id', help='Especifica el id del recpetor', nargs = 1)
parser.add_argument('-src_id', '--source_id', help='Especifica el id del emisor', nargs = 1)
parser.add_argument('-l', '--list_files', help='Lista de ficheros del usuario', action='store_true')
parser.add_argument('-d', '--download', help='Recupera el fichero especificado', nargs = 1)
parser.add_argument('-dlt_f', '--delete_file', help='Elimina el fichero especifcado', nargs = 1)
parser.add_argument('-enc', '--encrypt', help='Cifra un fichero (indicar destinatario)', nargs = 1)
parser.add_argument('-s', '--sign', help='Firma un fichero', nargs = 1)
parser.add_argument('-e_s', '--enc_sign', help='Cifra y firma un fichero (indicar destinatario)', nargs = 1)

args = parser.parse_args()
 
# Aqui procesamos lo que se tiene que hacer con cada argumento
if args.create_id: 
	create_id (args.create_id[0], args.create_id[1], token)
elif args.search_id:
	search_id (args.search_id[0], token)
elif args.delete_id:
	delete_id(args.delete_id[0], token)
elif args.upload and args.dest_id:
	upload(args.upload[0], args.dest_id[0], token)
elif args.list_files:
	list_files(token)
elif args.download:
	if args.source_id: 
		download(args.download[0], token, emisor=args.source_id[0])
	else: 
		download(args.download[0], token)	
elif args.delete_file: 
	delete_file(args.delete_file[0], token)
elif args.encrypt and args.dest_id:
	clave = obtener_clave_publica(args.dest_id[0], token)
	cifrar_fichero(args.encrypt[0], clave)
elif args.sign:
	firmar_fichero(args.sign[0])
elif args.enc_sign and args.dest_id:
	firmar_cifrar_fichero(args.enc_sign[0], args.dest_id[0], token)
else:
	parser.print_help()	

	 