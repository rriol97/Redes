#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# cliente_p2p.py
#
# Implementacion de las funciones para comunicarse con otros clientes
#
# Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
##############################################################################

#importando librerias
from socket import *
import argparse
import codecs
import time

def iniciar_llamada(control_socket, nick, ip, puerto_origen, puerto_destino, protocolo):
    control_socket.settimeout(15)
    control_socket.connect((ip, int(puerto_destino)))
    arg = 'CALLING '+nick+ ' '+puerto_origen
    control_socket.send(arg.encode('utf-8'))
    control_socket.settimeout(None)
  

def pausar_llamada(control_socket, nick, ip, puerto_destino):
    control_socket.connect((ip, int(puerto_destino)))
    arg = 'CALL_HOLD '+nick
    control_socket.send(arg.encode('utf-8'))
    

def reanudar_llamada(control_socket, nick, ip, puerto_destino):
    control_socket.connect((ip, int(puerto_destino)))
    arg = 'CALL_RESUME '+nick
    control_socket.send(arg.encode('utf-8'))
    

def finalizar_llamada(control_socket, nick, ip, puerto_destino):
    control_socket.connect((ip, int(puerto_destino)))
    arg = 'CALL_END '+nick
    control_socket.send(arg.encode('utf-8'))
    

