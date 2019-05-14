#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# cliente_ds.py
#
# Implementacion de las funciones para comunicarse con el
# servidor de descubrimiento
#
# Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
##############################################################################

#importando librerias
from socket import *
import argparse
import codecs

def registrar_cliente (cliente_socket, nick, ip, puerto, psw, protos):
	print ('Registrando en el servidor...')
	# Comprobacion de argumentos de entrada
	if '#' in nick:
		return 'WRONG_NICK'
	if 65335 < int(puerto) or int(puerto) <= 1024:
		return 'WRONG_PORT'
		
	serverName = 'vega.ii.uam.es'
	serverPort = 8000
	sentence = 'REGISTER '+nick+' '+ip+' '+puerto+' '+psw+' '+protos
	cliente_socket.connect((serverName, serverPort))
	cliente_socket.send(sentence.encode('utf-8'))
	modifiedSentence = cliente_socket.recv(1024)
	res = modifiedSentence.decode('utf-8')
	ret = res.split(' ')

	if ret[0] == 'NOK':
		b = 'QUIT'
		cliente_socket.send(b.encode('utf-8'))
		aux = cliente_socket.recv(1024)
		return ret[1]
	else:
		return ret[1]+' '+ret[2]

def consulta (cliente_socket, nick):
	print ('Buscando a '+ nick+ ' en el servidor ...')
	arg = 'QUERY '+nick
	cliente_socket.send(arg.encode('utf-8'))
	resp = cliente_socket.recv(1024)
	res = resp.decode('utf-8')
	lista = res.split(' ')

	if lista[0] == 'NOK':
		return 'Usuario no encontrado'
	else:
		return lista[2], lista[3], lista[4], lista[5]

def contar_usu (cadena,n_usu):
	for c in cadena:
		if c == '#':
			n_usu += 1
	return n_usu		

def lista_clientes (cliente_socket):
	comando = 'LIST_USERS'
	cliente_socket.send(comando.encode('utf-8'))
	resp = cliente_socket.recv(1024).decode('utf-8')
	#Cogemos el número de usurios totales
	l_usu = resp.split(' ')
	if l_usu[0] == 'NOK':
		print ('Imposible coger la lista de usuarios')
		return 

	nUsuarios = int (l_usu[2])
	#Contamos cuantos ha leido hasta ahora
	nLeidos = contar_usu(resp, 0)
	lista = resp.split('OK USERS_LIST '+str(nUsuarios)+' ')

	while nLeidos < (nUsuarios):
		resp = cliente_socket.recv(1024).decode('utf-8')
		nLeidos = contar_usu(resp, nLeidos)
		lista[1] += resp
	#Ya tenemos una cadena con todos los usuarios separados por #
	lista_final = lista[1].split('#')
	lista_final.remove("")
	return lista_final	

def cerrar_conexion(cliente_socket):	
	b = 'QUIT'
	cliente_socket.send(b.encode('utf-8'))
	aux = cliente_socket.recv(1024)
	cliente_socket.close()

