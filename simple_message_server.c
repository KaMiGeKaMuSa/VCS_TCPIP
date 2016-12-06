/*
socket (AF_INET, SOCK_STREAM, 0)
bind
listen
	accept
	fork
			dup2 für stdin und stdout
	exec (businesslogic ausführen)
			close für stdin und stdout
	close

getopt für port
bind bsp in manpage von ip(7) 
struct sockaddr_in:
	- sa_family = AF_INET;
	- sin_port = htans(portnummer)
	- sin_addr = INADDR_ANY
	
setsockopt(SOL_REUSEADDR,1)

*/