# Servidor iterativo TCP en Python


from socket import *

serverName = '192.168.1.45'
serverPort = 12001
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind((serverName, serverPort))
serverSocket.listen(1)
print "Servidor preparado para recibir"
while 1:
	connectionSocket, addr = serverSocket.accept()
	sentence = connectionSocket.recv(1024)
	capitalizedSentence = sentence.upper()
	print "Voy a enviar el mensaje", capitalizedSentence
	connectionSocket.send(capitalizedSentence)
	connectionSocket.close()