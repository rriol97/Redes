/* Servidor iterativo TCP */

int tcp_listen(char * arg1, char * arg2, socklen_t * addrlen);

int main(int argc, char **argv) {
	int listenfd, connfd;
	socklen_t clilen, addrlen;
	struct sockaddr *cliaddr;

	/* Contiene las llamadas a socket(), bind() y listen() */
	listenfd = tcp_listen(argv[1], argv[2], &addrlen);

	for ( ; ; ) {
		connfd = accept(listenfd, cliaddr, &clilen);
		process_request(connfd);

		Close(connfd);
	}
}

int tcp_listen(char * arg1, char * arg2, socklen_t * addrlen) {

}

