/*
socket (AF_INET, SOCK_STREAM, 0)
bind
listen
	accept
	fork
			dup2 f�r stdin und stdout
	exec (businesslogic ausf�hren)
			close f�r stdin und stdout
	close

getopt f�r port
bind bsp in manpage von ip(7) 
struct sockaddr_in:
	- sa_family = AF_INET;
	- sin_port = htans(portnummer)
	- sin_addr = INADDR_ANY
	
setsockopt(SOL_REUSEADDR,1)

*/