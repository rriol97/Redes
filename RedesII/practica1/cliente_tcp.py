# Cliente para servidor TCP en Python

from socket import *

serverName = '10.0.2.15'
serverPort = 12001
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName,serverPort))
sentence = raw_input ('Ingrese texto en minusculas:')
clientSocket.send(sentence)
modifiedSentence = clientSocket.recv(1024)
print ('Desde el servidor:', modifiedSentence)
clientSocket.close()