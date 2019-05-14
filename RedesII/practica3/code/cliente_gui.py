#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# cliente_gui.py
#
# Implementacion de la interfaz del cliente de la plataforma de video
#
# Autores : Ricardo Riol Gonzalez y Alejandro Sanchez Sanz
##############################################################################

from appJar import gui
import cliente_ds, cliente_p2p
import socket
from PIL import Image, ImageTk
import numpy as np
import cv2
import threading
import urllib.request
import time
import queue
import itertools




class GuiClient(object):

    def __init__(self, tam_ventana):


        #Interfaz gráfica de la aplicación
        self.app = gui("Redes II - P2P", tam_ventana)
        self.app.setGuiPadding(10,10)
        self.app.setLocation("CENTER")
        self.app.setResizable(canResize=False)
        self.app.setGuiPadding(5,5)
        self.app.setBg("LightYellow")

        
    
        #Asociamos los sockets que necesitara la aplicacion   
        self.socket_ds = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # Socket para comunicarnos con el servidor de descubrimiento
        self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # Socket para enviar comandos de control a otro usuario
        self.socket_escucha = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # Socket para escuchar los comandos que nos envien otros usuarios
        self.socket_llamada = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # Socket para enviar y recibir video
        self.socket_llamada.settimeout(5)  # 5 segundos de espera a los frames de la llamada

        #Subventana para que el usuario se conecte al servidor
        self.app.startSubWindow("Login", title = "Inicio sesión", modal = True)
        self.app.setStopFunction(self.check_stop_login)
        self.app.setGeometry("390x250")
        self.app.setBg("LightYellow") 
        self.app.setGuiPadding(10,10)
        self.app.addLabelEntry("Username")
        self.app.addLabelSecretEntry("Password")
        self.app.addLabelEntry("Puerto Nº")
        self.app.addLabelOptionBox("Protocolo/Protocolos", ["V1"])
        self.app.addCheckBox("Registrar IP pública")
        self.app.addButton("Aceptar", self.login)
        self.app.enableEnter(self.login)
        self.app.stopSubWindow()

        #Interfaz de la aplicación de usario
        #Parte de arriba a la derecha
        self.app.setSticky("n")
        self.app.addLabel("title", "ZOEWIE 11.0",0,1)

        # Añadir los botones
        self.app.setSticky("ne")
        self.app.addButtons(["Logout", "Salir"], self.callbacks_botones, 0, 2)

        #Añadimos software para buscar usuarios
        self.app.setSticky("w")
        self.app.addLabelEntry("Buscar : ", 1, 0)
        self.app.setSticky("nw")
        self.app.addListBox("busq", [""],2,0) 
        self.app.setSticky("sw")
        self.app.addButtons(["Buscar", "Listar"], self.callbacks_botones, 2,0)

        #Añadimos una variable de clase para guardarnos el último usuario buscado
        self.last_usu = []

        #Añadimos botones para interacturar con el video
        self.app.setSticky("s")
        self.app.addButtons(["Llamar", "Play/Pause", "Colgar"], self.callbacks_botones, 2 ,1)

        # Registramos la función de captura de video
        # Esta misma función también sirve para enviar un vídeo
        self.pathWCam = "imgs/webcam.gif"
        self.app.setSticky("n")
        self.app.addImage("video", self.pathWCam, 2,1)
        self.cap = cv2.VideoCapture(0)
        self.hilo_video_propio = threading.Thread(target=self.video_propio, name='Hilo video propio')
        self.event = threading.Event()
        self.lock = threading.Lock()
        self.app.hideImage("video")

        self.pathUser = "imgs/user.gif"
        self.app.setSticky("s")
        self.app.addImage("user", self.pathUser, 2,2)
        self.hilo_video_externo = threading.Thread(target=self.video_externo, name='Hilo video externo')
        self.app.hideImage("user")

        # Hilo para escuchar comandos de control de conexion
        self.hilo_escucha = threading.Thread(target=self.control_video, name='Hilo escucha')

        #Añadimos barra statusBar
        self.app.addStatusbar(fields=3)
        self.app.setStatusbar("Alejandro Sánchez Sanz | Ricardo Riol González", 0)
        self.app.setStatusbarWidth(50, 0)
        self.app.setStatusbar("Duración: 0", 1)
        self.app.setStatusbar("FPS: 0", 2)

        # Buffer necesario para la lectura del video
        self.buffer = queue.PriorityQueue(60) 

        # Variables globales necesarias
        self.lanzar_hilo_video = True
        self.empezado = False
        self.pausado_propio = True
        self.pausado_externo = True
        self.puerto_origen = 6000
        self.control = False
        self.nick = ""
        self.ip_destino = ""
        self.ip_origen = ""
        self.ip_origen_privada = ""
        self.puerto_destino = 8080
        self.primero = False
        self.inicio = 0
        self.control_buffer = 0
        self.loggeado = False
        self.counter = itertools.count()
        self.protocolos = ""
        self.delay = 0
        self.app.registerEvent(self.mostrar_video)

    def start(self):
        self.app.go(startWindow = "Login")  

    def salir(self):
        self.app.stop()

    def check_stop_login(self):
        print("Cerrando desde el login")
        ret = self.app.yesNoBox("Confirmar Salida", "¿Está seguro de que desea cerrar la aplicación?")
        if ret:    
            self.buffer.join()
            self.control = False
            self.socket_control.close()
            self.socket_escucha.close()
            self.socket_llamada.close()
            self.socket_ds.close()
            if self.loggeado:
                self.app.setStopFunction(None)
            self.event.set()
            self.app.stop()



    def check_stop(self):
        print('Saliendo de la aplicación')
        ret = self.app.yesNoBox("Confirmar Salida", "¿Está seguro de que desea cerrar la aplicación?")
        if ret:           
            self.control = False
            self.lock.acquire()
            self.lanzar_hilo_video = True
            self.lock.release()
            self.event.set() 
            try:           
                
                self.socket_control.connect((self.ip_origen_privada, int(self.puerto_origen)))
                self.empezado = False
                self.cap.release()
                self.event.set()
                self.socket_control.close()
                self.socket_escucha.close()
                self.socket_llamada.close()
                cliente_ds.cerrar_conexion(self.socket_ds)
            except OSError:
                print('Problema cerrando todo para salir de la aplicación')

        else: 
            self.control = True
            self.lock.acquire()
            self.lanzar_hilo_video = False
            self.lock.release()

        
        self.event.set()

        return ret
 
    # POR DEFECTO, LA LOCAL, PERO SE DA OPCION A LA PUBLICA
    def get_ip_publica(self):
        """Obtiene la ip publica del sistema, mediante una lectura a la pagina
        web http://ip.42.pl/raw extrayendo de ella la ip publica mostrada"""
        
        self.ip_origen = urllib.request.urlopen('http://ip.42.pl/raw').read().decode('utf-8') 

    def get_ip_privada(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            s.connect(('10.255.255.255', 1))
            IP = s.getsockname()[0]
        except:
            IP = '127.0.0.1'
        finally:
            s.close()
        return IP
    

    def login(self, name):
        self.ip_origen_privada = self.get_ip_privada()
        tipo_ip = self.app.getCheckBox("Registrar IP pública")
        if tipo_ip:
            self.get_ip_publica()
        else:
            self.ip_origen = self.ip_origen_privada

        self.nick = self.app.getEntry("Username")
        psw = self.app.getEntry("Password")
        self.puerto_origen = self.app.getEntry("Puerto Nº")
        self.protocolos = self.app.getOptionBox("Protocolo/Protocolos")
        res = ""
        try:
            res = cliente_ds.registrar_cliente(self.socket_ds, self.nick, self.ip_origen, self.puerto_origen, psw, self.protocolos)
        except:
            print('NO HAY CONEXION A INTERNET')
        if res == "":
            self.app.errorBox("Error","No hay conexión a internet", parent=None)
            self.socket_ds.close()
            self.socket_ds = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        elif res == "WRONG_PASS":
            self.app.errorBox("Error","Contraseña incorrecta", parent=None)
            self.socket_ds.close()
            self.socket_ds = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        elif res == "WRONG_PORT":
            self.app.errorBox("Error","Puerto incorrecto\nDebe estar entre 1025 y 65335", parent=None)
        elif res == "WRONG_NICK":
            self.app.errorBox("Error","Nombre de usuario incorrecto\nNo puede contener almohadillas", parent=None)
        else:    
            self.socket_escucha.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                self.socket_escucha.bind(('0.0.0.0', int(self.puerto_origen)))
            except OSError:
                self.app.errorBox("Error","Error vinculando la dirección de escucha", parent=None)
                self.socket_ds.close()
                self.socket_ds = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.socket_escucha.close()
                self.socket_escucha = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                return
            self.socket_escucha.listen(1)
            if not self.control:
                self.hilo_escucha.start()
                self.control = True

            else:
                self.event.set()
            self.app.openSubWindow("Login")
            self.app.setStopFunction(None)
            self.loggeado = True
            self.app.hideSubWindow("Login")
            self.app.disableEnter()
            self.app.showImage("video")
            self.app.showImage("user")
            self.app.setStopFunction(self.check_stop)
            self.app.show()

    def logout(self):
        print("Saliendo del servicio, volviendo al login")
        self.nick = ""
        try:
            self.socket_control.connect((self.ip_origen_privada, int(self.puerto_origen)))
            cliente_ds.cerrar_conexion(self.socket_ds)
            self.socket_ds = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket_escucha.close()
            self.socket_escucha = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket_control.close()
            self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except OSError:
            print('Problema cerrando todo para hacer logout')
        self.app.clearAllEntries()
        self.app.clearAllListBoxes()
        self.app.setCheckBox("Registrar IP pública", False)
        self.app.enableEnter(self.login)
        self.control = True
        self.app.hide()
        self.app.openSubWindow("Login")
        self.app.setStopFunction(self.check_stop_login)
        self.app.showSubWindow("Login")

    def buscar(self):
        usu = self.app.getEntry("Buscar : ")
        if usu == '':
            self.app.errorBox("Error","Debe introducir el nick del usuario que desea buscar", parent=None)
            return
        try:
            ret = cliente_ds.consulta(self.socket_ds, usu)
        except:
            self.app.errorBox("Error","El servidor de descubrimiento no funciona correctamente", parent=None)
            return
        if ret == "Usuario no encontrado":
            self.app.errorBox("Error", ret, parent=None)
            return 
        self.last_usu = ret    
        self.app.clearListBox("busq", callFunction=False)
        self.app.addListItem("busq", ret[0])    

    def listar(self):
        try:
            ret = cliente_ds.lista_clientes(self.socket_ds)
        except:
            self.app.errorBox("Error","El servidor de descubrimiento no funciona correctamente", parent=None)
            return
        for usu in ret:
            l = usu.split(" ")
            self.app.addListItem("busq", l[0])
        self.last_usu = []

    def actualizar_usu(self):
        self.app.clearListBox("busq", callFunction=False)
        self.listar()        

    def llamar(self):
        if self.empezado:
            return
        if self.last_usu == []:
            res = self.app.getListBox("busq")
            if res == []:
                self.app.errorBox("Error","Debe seleccionar un usuario al que llamar", parent=None)
                return
            nick_usu = res[0]
            try:
                self.last_usu = cliente_ds.consulta(self.socket_ds, nick_usu)
            except:
                self.app.errorBox("Error","El servidor de descubrimiento no funciona correctamente", parent=None)
                return
        nick = self.nick
        print ('Estableciendo llamada con '+ self.last_usu[0]+ ' ...')
        self.ip_destino = self.last_usu[1]
        self.puerto_destino = self.last_usu[2]
        protocolos_destino = self.last_usu[3]
        lista = protocolos_destino.split('#')
        if not self.protocolos in lista:
            self.app.errorBox("Error","El usuario "+self.last_usu[0]+ " no tiene protocolos comunes a nosotros", parent=None)
            return


        try:
            cliente_p2p.iniciar_llamada(self.socket_control, nick, self.ip_destino, self.puerto_origen, self.puerto_destino, protocolos_destino)
        except socket.timeout:
            self.socket_control.settimeout(None)
            self.app.infoBox("Usuario no responde","El usuario "+self.last_usu[0]+ " no ha respondido a tiempo a su llamada", parent=None)
        except OSError:
            self.socket_control.settimeout(None)
            self.app.infoBox("Usuario no conectado","El usuario "+self.last_usu[0]+ " no está conectado en estos momentos", parent=None)

        self.socket_control.close()
        self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        
    

    def pausar(self):
        print('Peticion de pausado de llamada')
        self.pausado_propio = True
        self.pausado_externo = True

    def reanudar(self):
        print('Peticion de reanudar la llamada')
        self.pausado_propio = False
        self.pausado_externo = False

    def colgar(self):
        print('Finalizar la llamada')
        self.control_buffer = 0
        self.pausado_propio = True
        self.pausado_externo = True
        self.empezado = False
        self.app.setStatusbar("Duración: 0", 1)
        self.app.setStatusbar("FPS: 0", 2)
        self.socket_llamada.close()
        self.socket_llamada = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            self.socket_control.connect((self.ip_origen_privada, int(self.puerto_origen)))
        except OSError:
            self.app.errorBox("Error","Error cerrando todo para colgar", parent=None)

        self.socket_control.close()
        self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        
        
        


    def callbacks_botones(self, button):    
        if button == "Salir":
            self.salir()
        elif button == "Logout":
            self.logout()
        elif button == "Play/Pause":
            if self.empezado:
                if self.pausado_propio and self.pausado_externo:
                    try: 
                        cliente_p2p.reanudar_llamada(self.socket_control, self.nick, self.ip_destino, self.puerto_destino)
                    except:
                        self.app.errorBox("Usuario no conectado","El usuario de la llamada no está conectado en estos momentos", parent=None)
                        self.socket_control.close()
                        self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        return
                    self.socket_control.close()
                    self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.reanudar()
                else:
                    try:
                        cliente_p2p.pausar_llamada(self.socket_control, self.nick, self.ip_destino, self.puerto_destino)
                    except:
                        self.app.errorBox("Usuario no conectado","El usuario de la llamada no está conectado en estos momentos", parent=None)
                        self.socket_control.close()
                        self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        return
                    self.socket_control.close()
                    self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.pausar()
        elif button == "Colgar":
            try:
                cliente_p2p.finalizar_llamada(self.socket_control, self.nick, self.ip_destino, self.puerto_destino)
            except:
                self.app.errorBox("Usuario no conectado","El usuario de la llamada no está conectado en estos momentos", parent=None)
                self.socket_control.close()
                self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                return
            self.socket_control.close()
            self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.colgar() 
        elif button == "Llamar":
            self.llamar() 
        elif button == "Buscar":
            self.buscar()
        elif button == "Listar":
            self.actualizar_usu() 



    ################################################
    ## VIDEO
    ################################################
    def mostrar_video(self):
        if not self.control:
            return
        if self.control_buffer == 1:
            # INTENTO DE CONTROLAR FLUJO AUTOMATICAMENTE
            # if self.buffer.qsize() < 20:
            #     self.delay /= 2
            # elif self.buffer.qsize() > 50:
            #     self.delay += 4
            # if self.pausado_externo:
            #     self.delay = 0
            # self.app.setPollTime(self.delay)

            if self.buffer.empty():
                return

            img_tk = self.buffer.get_nowait()
            self.app.setImageData("video", img_tk[2], fmt = 'PhotoImage')
        else:
            if not self.buffer.empty() and not self.empezado:
                self.buffer.get_nowait()  # Vaciamos el buffer cuando no estamos en llamada (despues de colgar)
        

    # Función que trata el video que proviene de la webcam de la persona con la que estemos 
    def video_externo(self):
    
        while(True):
            # Bucle de espera activa mientras el video esta pausado
            debug = True
            while(self.pausado_externo):
                if debug:
                    print('ACABAMOS DE MOSTRAR VIDEO EXTERNO')
                    debug = False
                if self.lanzar_hilo_video:
                    print('Se ha salido de la aplicacion')
                    return
                if not self.empezado:
                    print('Se ha colgado')
                    self.app.reloadImage("video", self.pathWCam)
                    self.event.clear()
                    self.event.wait()

            if not self.empezado:
                print('Se ha colgado')
                self.app.reloadImage("video", self.pathWCam)
                self.event.clear()
                self.event.wait()   # Espera hasta una nueva llamada
                self.primero = True 

            if self.lanzar_hilo_video:
                print('Se ha salido de la aplicacion')
                return

            self.event.clear()

            # Recibimos los datos de video 
            try:
                (mensaje, ip) = self.socket_llamada.recvfrom(65536)
            except socket.timeout:
                print('Llegan lentos los frames')
                continue
            recibido = mensaje.split(b'#');

            orden_frame = recibido[0].decode('utf-8')
            timestamp = recibido[1].decode('utf-8')
            resolucion = recibido[2].decode('utf-8')
            fps = recibido[3].decode('utf-8')
 
            
            # Juntamos los bytes del frame recibido
            mensaje_bytes = b'#'.join(recibido[4:]);    # Posicion 4 hasta el final de la lista

            # Descompresión de los datos, una vez recibidos
            decimg = cv2.imdecode(np.frombuffer(mensaje_bytes, np.uint8), 1)
            

            # Conversión de formato para su uso en el GUI
            decimg = cv2.resize(decimg, (400,380))
            cv2_im = cv2.cvtColor(decimg,cv2.COLOR_BGR2RGB)
            img_tk = ImageTk.PhotoImage(Image.fromarray(cv2_im))            

            
            count = next(self.counter)
            self.delay = int(float(fps))
            self.buffer.put((orden_frame, count, img_tk), block=True)
            if self.buffer.full():  
                self.app.setPollTime(self.delay)
                self.control_buffer = 1
             

    # Función que trata el video que proviene de nuestra camara
    def video_propio(self):
        while(True):
            self.event.clear()

            if self.primero:
                total_frames = 1
                self.primero = False
            else:
                total_frames += 1

            # Capturamos un frame de la cámara o del vídeo
            ret, frame = self.cap.read()
            # Compresión JPG al 50% de resolución (se puede variar)
            encode_param = [cv2.IMWRITE_JPEG_QUALITY,50]
            result,encimg = cv2.imencode('.jpg', frame, encode_param)
            if result == False: 
                print('Error al codificar imagen')
            encimg = encimg.tobytes()
            frame_mio = cv2.resize(frame, (96,96))
            cv2_im = cv2.cvtColor(frame_mio,cv2.COLOR_BGR2RGB)
            img_tk = ImageTk.PhotoImage(Image.fromarray(cv2_im))            

            # Lo mostramos en el GUI
            self.app.setImageData("user", img_tk, fmt = 'PhotoImage')

            # Aquí tendría que el código que envia el frame a la red
            # Cabecera
            cabecera = []
            timestamp = time.time()
            duracion = timestamp - self.inicio
            fps = self.cap.get(cv2.CAP_PROP_FPS)
            width = self.cap.get(cv2.CAP_PROP_FRAME_WIDTH)
            height = self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
            resolucion = str(width) + "x" + str(height)
            
            self.app.setStatusbar("Duración: "+str(duracion//1), 1)
            self.app.setStatusbar("FPS: "+str(fps), 2)

            cabecera.append(str(total_frames))
            cabecera.append(str(timestamp))
            cabecera.append(resolucion)
            cabecera.append(str(fps))
            separador = '#'
            mensaje = separador.join(cabecera) + separador
            mensaje = bytearray(mensaje, 'utf-8')
            mensaje = mensaje + encimg

            # Los datos ya están listos para su envío por la red
            try:
                self.socket_llamada.sendto(mensaje, (self.ip_destino, int(self.puerto_destino)))
            except BrokenPipeError:
                print('Problema con la conexión con el otro usuario')
                try:
                    cliente_p2p.finalizar_llamada(self.socket_control, self.nick, self.ip_destino, self.puerto_destino)
                except:
                    self.app.errorBox("Usuario no conectado","El usuario de la llamada no está conectado en estos momentos", parent=None)
                    self.socket_control.close()
                    self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.socket_control.close()
                self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.colgar()

            # Bucle de espera activa mientras el video esta pausado
            debug = True
            while(self.pausado_propio):
                if debug:
                    print('ACABAMOS DE MOSTRAR VIDEO PROPIO')
                    debug = False
                if self.lanzar_hilo_video:
                    print('Se ha salido de la aplicacion')
                    return
                if not self.empezado:
                    print('Se ha colgado')
                    self.app.reloadImage("user", self.pathUser)
                    self.event.clear()
                    self.event.wait()

            if not self.empezado:
                print('Se ha colgado')
                self.app.reloadImage("user", self.pathUser)
                self.event.clear()
                self.event.wait()   # Espera hasta una nueva llamada
                self.primero = True

            if self.lanzar_hilo_video:
                print('Se ha salido de la aplicacion')
                return

                
    def control_video(self):
        while(True):
            

            connfd, addr = self.socket_escucha.accept() # Se bloquea aqui hasta recibir un comando

            # Para el logout
            if self.nick == "":
                self.event.clear()
                self.event.wait()


            ret = self.control

            if not ret:
                break
            
            resp = connfd.recv(1024).decode('utf-8').split(' ')
            if resp[0] == 'CALL_HOLD':
                self.pausar()
            elif resp[0] == 'CALL_RESUME':
                self.reanudar()
            elif resp[0] == 'CALL_END':
                connfd.close()
                self.colgar()
            elif resp[0] == 'CALL_DENIED':
                self.app.infoBox("Llamada rechazada","El usuario "+resp[1]+ " ha rechazado la llamada", parent=None)
                self.socket_control.close()
                self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            elif resp[0] == 'CALL_BUSY':
                self.app.infoBox("Linea ocupada","El usuario "+resp[1]+ " esta en otra llamada actualmente", parent=None)
                self.socket_control.close()
                self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            elif resp[0] == 'CALL_ACCEPTED':
                self.app.infoBox("Llamada aceptada","El usuario "+resp[1]+ " ha aceptado su llamada", parent=None)
                self.pausado_propio = False
                self.pausado_externo = False
                self.primero = True
                self.socket_llamada.close()
                self.socket_llamada = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                try:
                    self.socket_llamada.bind((self.ip_origen_privada, int(self.puerto_origen)))
                except OSError:
                    self.app.errorBox("Error","Error vinculando la dirección de llamada", parent=None)
                    self.socket_llamada.close()
                    self.socket_llamada = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    continue
                self.inicio = time.time()
                if self.lanzar_hilo_video:
                    self.lanzar_hilo_video = False
                    self.empezado = True
                    self.hilo_video_propio.start()
                    self.hilo_video_externo.start()
                else:
                    if not self.empezado:
                        print("estoy aquiii")
                        self.empezado = True
                        self.event.set()
                

                
            elif resp[0] == 'CALLING':
                try:
                    self.ip_destino = cliente_ds.consulta(self.socket_ds, resp[1])[1]
                except:
                    self.app.errorBox("Error","El servidor de descubrimiento no funciona correctamente", parent=None)
                    continue
                self.puerto_destino = int(resp[2])
                try:
                    self.socket_control.connect((self.ip_destino, self.puerto_destino))
                except OSError:
                    self.app.errorBox("Error","Error conectando con el usuario que quiere llamarnos", parent=None)
                    self.socket_control.close()
                    self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    continue
                if self.empezado:
                    arg = 'CALL_BUSY '+self.nick
                else:
                    
                    ret = self.app.yesNoBox("LLamada entrante", "¿Desea aceptar la llamada del usuario "+resp[1]+"?", parent=None)
                    if ret:
                        # LLAMADA COMIENZA
                        
                        arg = 'CALL_ACCEPTED '+self.nick+' '+self.puerto_origen
                        self.socket_llamada.close()
                        self.socket_llamada = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                        try:
                            self.socket_llamada.bind((self.ip_origen_privada, int(self.puerto_origen)))
                        except OSError:
                            self.app.errorBox("Error","Error vinculando la dirección de llamada", parent=None)
                            self.socket_llamada.close()
                            self.socket_llamada = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                            continue
                        self.inicio = time.time()
                        self.pausado_propio = False
                        self.pausado_externo = False
                        self.primero = True
                        if self.lanzar_hilo_video:
                            self.lanzar_hilo_video = False
                            self.empezado = True
                            self.hilo_video_propio.start()
                            self.hilo_video_externo.start()
                        else:
                            if not self.empezado:
                                self.empezado = True
                                self.event.set()
                        
                    else:
                        arg = 'CALL_DENIED '+self.nick

                self.socket_control.send(arg.encode('utf-8'))
                self.socket_control.close()
                self.socket_control = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        return
        
                

if __name__ == '__main__':

    gui = GuiClient("900x600")
    gui.start()







