/**
 * @file simple_message_client.c
 * TCP/IP Server-Client project
 *
 * @author Karin Kalman <karin.kalman@technikum-wien.at>
 * @author Michael Mueller <michael.mueller@technikum-wien.at>
 * @author Gerhard Sabeditsch <gerhard.sabeditsch@technikum-wien.at>
 * @date 2016/12/14
 *
 * @version $Revision: 1 $
 *
 *
 * URL: $HeadURL$
 *
 * Last Modified: $Author: Michael $
 */
 
 /**
 * -------------------------------------------------------------- includes --
 */
#include "simple_message_client_commandline_handling.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
 
/**
 * -------------------------------------------------------------- defines --
 */
#define MAX_BUF 1024 
 
 /**
 * -------------------------------------------------------------- global variables --
 */
const char *cpServer, *cpPort, *cpUser, *cpMessage, *cpImage, *cpFilename;
int iVerbose = 0;
int save_errno = 0;

/**
 * --------------------------------------------------- function prototypes --
 */
void usage(FILE * stream, const char * message, int exitcode);
void verbose(const char * message);
void openSocket(int *paramISocketFD);
void sendRequest(int *paramISocketFD, FILE* fpWriteSocket);
void readResponse(int *paramISocketFD, FILE* fpReadSocket);

 /**
 * ------------------------------------------------------------- main --
 */
int main(int argc, const char* argv[])
{	
    int iSocketFD;
	cpFilename = argv[0];
	FILE* fpWriteSocket = NULL;
	FILE* fpReadSocket = NULL;
	
	/* function to parse parameter provided by Thomas M. Galla, Christian Fibich*/
	smc_parsecommandline(argc, argv, &usage, &cpServer, &cpPort, &cpUser, &cpMessage, &cpImage, &iVerbose);

	/* function to connect to server */
	openSocket(&iSocketFD);   
	
	/* function to write request into stream socket */
	sendRequest(&iSocketFD, fpWriteSocket);

	/* function to parse response into files */
	readResponse(&iSocketFD, fpReadSocket);
	
	/* everthing fine - close file pointer if they were already open*/
    if ( fpWriteSocket != NULL) fclose(fpWriteSocket);
    if ( fpReadSocket != NULL) fclose(fpReadSocket);
	
	close(iSocketFD);
    
	return 0;
}

/**
 * \brief function needed as error message in smc_parsecommandline
 *
 * \param stream - stream where error message gets printed
 * \param message - error message print before exiting
 * \param errcode - int number which is used at exit
 */
void usage(FILE * stream, const char * message, int errcode)
{
    //reset errno for checking fprintf()
    errno = 0;
    if (fprintf(stream,
            "\n usage: %s options\n"
            "options:\n"
			"        -s, --server  <server>       IP address (IPv4 or IPv6) of server\n"
            "        -p, --port <port>       port of the server [0 to 65535]\n"
			"        -u, --user <user>		 username for the message submission\n"
			"        -i, --image <image URL>       image url for the submitting user\n"
			"        -m, --message <message>	   message to submit to bulletin board\n"
			"        -v, --verbose	   trace information to stdout\n"
            "        -h, --help\n", message) < 0) {
        /*When fprintf fails, the new exit value is the errno value from the failed fprintf()*/
		errcode = errno; 
    }
   
    exit(errcode);
}

void verbose(const char * message) 
{	
	if(iVerbose) {
		if(!printf("%s: %s\n", cpFilename, message)) {
			fprintf(stderr,"Error on printf.\n");
		}
	}	
}

/**
 * \brief function to get socket information and establish connection
 *
 * \param paramISocketFD - socket file descriptor to the address
 */
void openSocket(int *paramISocketFD) 
{
	struct addrinfo hints, *socket_address, *rp;
	int iRetValue;
	
	verbose("Try to connect to socket");
	
	/* set memory for struct hints */
	memset(&hints, 0, sizeof hints); //fill a byte string with a byte value
	
	/* 
	*	AI_FAMILY - valid values:
	*	AF_INET = IPv4
	*	AF_INET6 = IPv6
	*	AF_UNSPEC = Allow IPv4 AND IPv6 sockets
	*/
	hints.ai_family = AF_UNSPEC;
	/*	
	*	AI_SOCKTYPE - valid values:
	*	SOCK_STREAM  = TCP socket
	*	SOCK_DGRAM = UDP socket
	*	0 = Allow TCP AND UDP sockets
	*/
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;
	/* Any protocol */
    hints.ai_protocol = 0;          
	
	/* get address info of socket */
	if ((iRetValue = getaddrinfo(cpServer, cpPort, &hints, &socket_address)) != 0) {
        
        //RESET save_errno
        save_errno = 0;
        
        //ERROR MESSAGE
        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "getaddrinfo()", strerror(errno)) < 0) save_errno= errno;
		
        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
        exit(EXIT_FAILURE);
	}
	
	/* loop over socket address linked list */
	for (rp = socket_address; rp != NULL; rp = rp->ai_next) {
	   *paramISocketFD = socket(rp->ai_family, rp->ai_socktype,
					rp->ai_protocol);
	   if (*paramISocketFD == -1)
		   continue;
	
	   if (connect(*paramISocketFD, rp->ai_addr, rp->ai_addrlen) != -1)
		   break;                 
	
	   close(*paramISocketFD);
    }
	
	if (rp == NULL || paramISocketFD == NULL) {
		
        //RESET save_errno
        save_errno = 0;
        
        //ERROR MESSAGE
        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),connect()", "no address succeeded") < 0) save_errno= errno;

        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
		exit(EXIT_FAILURE);
        
        
	}
	
	/* socket address info are no longer needed */
	freeaddrinfo(socket_address);   
	
	verbose("Successful connected to socket");
}

/**
 * \brief function to open stream as write stream and write/send request to server socket
 *
 * \param paramISocketFD - socket file descriptor to the address
 */
void sendRequest(int *paramISocketFD, FILE* fpWriteSocket) 
{
	
	verbose("Try to send request to server");
	
	/* open socket file descriptor */
	verbose("Open stream for writing");
	if ((fpWriteSocket = fdopen(*paramISocketFD, "w")) == NULL) {
        
        //RESET save_errno
        save_errno = 0;
        
        //ERROR MESSAGE
        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fdopen()", strerror(errno)) < 0) save_errno= errno;

        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
		exit(EXIT_FAILURE);
        
	}
	
	/* write user to stream */
	verbose("Write user into stream");
	if (fprintf(fpWriteSocket,"user=%s\n",cpUser) < 0) {

        //RESET save_errno
        save_errno = 0;

        //ERROR MESSAGE
        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fprintf()", strerror(errno)) < 0) save_errno= errno;

    //close write file pointer
        if (fclose(fpWriteSocket) < 0) {
            //ERROR MESSAGE
            if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Write File Pointer", strerror(errno)) < 0) save_errno= errno;
        }
        
        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
		exit(EXIT_FAILURE);
	}
	
    
	/* if img exist: write image to sream */
	if (cpImage != NULL) {
		verbose("Write image into stream");
		if (fprintf(fpWriteSocket,"img=%s\n", cpImage) < 0) {
			
            //RESET save_errno
            save_errno = 0;

            //ERROR MESSAGE
            if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fprintf()", strerror(errno)) < 0) save_errno= errno;

    //close write file pointer
            if (fclose(fpWriteSocket) < 0) {
                
                //ERROR MESSAGE
                if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Write File Pointer", strerror(errno)) < 0) save_errno= errno;
            
            }
            
            //EXIT LOGIC
            if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
			exit(EXIT_FAILURE);
		}
	}
	
	/* write message to stream */
	verbose("Write message into stream");
	if (fprintf(fpWriteSocket,"%s\n", cpMessage) < 0) {
		
        //RESET save_errno
        save_errno = 0;
     
        //ERROR MESSAGE
        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fprintf()", strerror(errno)) < 0) save_errno= errno;

        
    //close write file pointer
        if (fclose(fpWriteSocket) < 0) {
            
            //ERROR MESSAGE
            if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Write File Pointer", strerror(errno)) < 0) save_errno= errno;
            
        }
        
        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
        exit(EXIT_FAILURE);

	}
	
	/* flush all unwritten data to the stream */
	if (fflush(fpWriteSocket) != 0) {
		
        //RESET save_errno
        save_errno = 0;
       
        //ERROR MESSAGE
        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fflush()", strerror(errno)) < 0) save_errno= errno;


    //close write file pointer
        if (fclose(fpWriteSocket) < 0) {
            
            //ERROR MESSAGE
            if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Write File Pointer", strerror(errno)) < 0) save_errno= errno;
            
        }
        
        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
        exit(EXIT_FAILURE);
	}
	
	/* after writing: disable write operations for socket */
	verbose("Close write direction of stream");
	if (shutdown(*paramISocketFD,SHUT_WR) != 0) {
		
        //RESET save_errno
        save_errno = 0;
        
        //ERROR MESSAGE
        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "shutdown()", strerror(errno)) < 0) save_errno= errno;
        
        
    //close write file pointer
        if (fclose(fpWriteSocket) < 0) {
            
            //ERROR MESSAGE
            if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Write File Pointer", strerror(errno)) < 0) save_errno= errno;
            
        }
        
        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
        exit(EXIT_FAILURE);
        
	}
	
	verbose("Succesful sent request to server");
}

/**
 * \brief function to open stream as read stream and parse response into files
 *
 * \param paramISocketFD - socket file descriptor to the address
 */
void readResponse(int *paramISocketFD, FILE* fpReadSocket) 
{
	FILE* fpInputFile = NULL;
	char cBuf[MAX_BUF];
	int iRecStatus, iRecLength, iReadlen = 0, iBufLen = 0, iCurrentlyRead = 0;
	char *cpResponseFilename = NULL;
	
	verbose("Try to parse response of server");
	
	/* open socket file descriptor in read operation */
	verbose("Open stream for reading");
	if ((fpReadSocket = fdopen(*paramISocketFD, "r")) == NULL) {
		
        //RESET save_errno
        save_errno = 0;
        
        //ERROR MESSAGE
        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fdopen()", strerror(errno)) < 0) save_errno= errno;

        //EXIT LOGIC
        if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
		exit(EXIT_FAILURE);
        
	}
	
	/* loop over response */
	while(fgets(cBuf, MAX_BUF, fpReadSocket)) {
		//reset errno
        errno = 0;
			
		/* check if part of buffered response is status part */
		if (strncmp(cBuf, "status=", 7) == 0) 
		{
			/* get status of response */
			verbose("Parse status of response");
			if (sscanf(cBuf,"status=%d",&iRecStatus) == 0) {
				if (errno != 0) {
					
                    //RESET save_errno
                    save_errno = 0;

                    //ERROR MESSAGE
                    if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "sscanf()", /*strerror(errno)*/"status could not be scanned") < 0) save_errno= errno;

                    
                //close read file pointer
                    if (fclose(fpReadSocket) < 0) {
                        
                        //ERROR MESSAGE
                        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Read File Pointer", strerror(errno)) < 0) save_errno= errno;
                        
                    }

                    //EXIT LOGIC
                    if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
                    exit(EXIT_FAILURE);
				}
			}
		}
		
		/* check if part of buffered response is filename part */
		if (strncmp(cBuf, "file=", 5) == 0) 
		{
			/* malloc for length of cbuf - "file=" and \n char at the end of the line */
			if ((cpResponseFilename = malloc((strlen(cBuf) - 6) * sizeof(char))) == NULL)
			{
                
                
                //RESET save_errno
                save_errno = 0;
                
                //ERROR MESSAGE
                if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "malloc()", strerror(errno)) < 0) save_errno= errno;
                
                
            //close read file pointer
                if (fclose(fpReadSocket) < 0) {
                    
                    //ERROR MESSAGE
                    if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Read File Pointer", strerror(errno)) < 0) save_errno= errno;
                    
                }
                
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
                exit(EXIT_FAILURE);

			}
			
			/* get filename of response */
			verbose("Parse filename of response");
			if (sscanf(cBuf,"file=%s",cpResponseFilename) == 0) {
				if (errno != 0) {
					
                    
                    //RESET save_errno
                    save_errno = 0;
                    
                    //ERROR MESSAGE
                    if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "sscanf()", /*strerror(errno)*/"file could not be scanned") < 0) save_errno= errno;
                    
                    
                //close read file pointer
                    if (fclose(fpReadSocket) < 0) {
                        
                        //ERROR MESSAGE
                        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Read File Pointer", strerror(errno)) < 0) save_errno= errno;
                        
                    }
                    
                    //EXIT LOGIC
                    if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
                    exit(EXIT_FAILURE);

				}
			}
		}
		
		/* check if part of buffered response is length part */
		if (strncmp(cBuf, "len=", 4) == 0) 
		{
			/* get bytes-length of file */
			verbose("Parse length of response file");
			if (sscanf(cBuf,"len=%d",&iRecLength) == 0) {
				if (errno != 0) {
					
                    //RESET save_errno
                    save_errno = 0;
                    
                    //ERROR MESSAGE
                    if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "sscanf()", /*strerror(errno)*/"file could not be scanned") < 0) save_errno= errno;
                    
                    
                    //close read file pointer
                    if (fclose(fpReadSocket) < 0) {
                        
                        //ERROR MESSAGE
                        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Read File Pointer", strerror(errno)) < 0) save_errno= errno;
                        
                    }
                    
                    //EXIT LOGIC
                    if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
                    exit(EXIT_FAILURE);

				}
			}
			
			/* open file in (w)rite mode -> create file if not exist or clean file and begin at null */
			verbose("Open response file in write mode");
			if ((fpInputFile = fopen(cpResponseFilename,"w")) ==  NULL)
			{
				
                
                //RESET save_errno
                save_errno = 0;
                
                //ERROR MESSAGE
                if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fopen()", strerror(errno)) < 0) save_errno= errno;
                
                
                //close read file pointer
                if (fclose(fpReadSocket) < 0) {
                    
                    //ERROR MESSAGE
                    if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Read File Pointer", strerror(errno)) < 0) save_errno= errno;
                    
                }
                
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
                exit(EXIT_FAILURE);

			}
			
			/* reset int variables for each file */
			iReadlen = 0;
			iBufLen = 0;
			
			/* loop until byte-length of file is reached and processed */
			while (iReadlen < iRecLength)
			{
				/* set length of bytes which are need to be read */
				if ((iRecLength-iReadlen) > MAX_BUF) {
					iBufLen = MAX_BUF;
				} else {
					iBufLen = iRecLength-iReadlen;
				}
			//
            /* read from stream... */
            //
				iCurrentlyRead = (int) fread(cBuf, sizeof(char),iBufLen,fpReadSocket);
				
				/* network problems because no bytes read */
				if (iCurrentlyRead == 0) {
					
                    //RESET save_errno
                    save_errno = 0;
                    
                    //ERROR MESSAGE
                    if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fread()", "Cannot read from socket") < 0) save_errno= errno;
                    
                
                //close inputfile file pointer
                    if (fclose(fpInputFile) < 0) {
                        
                        //ERROR MESSAGE
                        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close fpInputFile File Pointer", strerror(errno)) < 0) save_errno= errno;
                        
                    }

                    
                //close read file pointer
                    if (fclose(fpReadSocket) < 0) {
                        
                        //ERROR MESSAGE
                        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Read File Pointer", strerror(errno)) < 0) save_errno= errno;
                        
                    }
                    
                    //EXIT LOGIC
                    if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
                    exit(EXIT_FAILURE);

				}
			
            //
            /* ...and write into file */
            //
				if (((int)fwrite(cBuf, sizeof(char), iCurrentlyRead,fpInputFile)) != iCurrentlyRead)
				{
                    //RESET save_errno
                    save_errno = 0;
                    
                    //ERROR MESSAGE
                    if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fwrite()", strerror(errno)) < 0) save_errno= errno;

                    
                //close inputfile file pointer
                    if (fclose(fpInputFile) < 0) {
                        
                        //ERROR MESSAGE
                        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close fpInputFile File Pointer", strerror(errno)) < 0) save_errno= errno;
                        
                    }
                    
                    
                //close read file pointer
                    if (fclose(fpReadSocket) < 0) {
                        
                        //ERROR MESSAGE
                        if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close Read File Pointer", strerror(errno)) < 0) save_errno= errno;
                        
                    }
                    
                    //EXIT LOGIC
                    if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
                    exit(EXIT_FAILURE);

				}
				
				/* add processed bytes to counter */
				iReadlen += iCurrentlyRead;
			}
			
			/* flush all unwritten data to the stream */
			if (fflush(fpInputFile) != 0) {
				
                
                //RESET save_errno
                save_errno = 0;
                
                //ERROR MESSAGE
                if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fflush()", strerror(errno)) < 0) save_errno= errno;

               
            //close inputfile file pointer
                if (fclose(fpInputFile) < 0) {
                    
                    //ERROR MESSAGE
                    if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close fpInputFile File Pointer", strerror(errno)) < 0) save_errno= errno;
                    
                }
                
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
                exit(EXIT_FAILURE);
			}
			
            
            
			/* everything fine - close file pointer */
			verbose("Successful processed response file");
		
            
        //close inputfile file pointer
            if (fclose(fpInputFile) < 0) {
                
                //ERROR MESSAGE
                if (fprintf(stderr,"%s - %s: %s\n", cpFilename, "fclose() - could not close fpInputFile File Pointer", strerror(errno)) < 0) save_errno= errno;
                
                
                //EXIT LOGIC
                if (save_errno != 0) exit (save_errno); //If save_errno is not 0, than exit with save_errno -> otherwise exit with normal failure
                exit(EXIT_FAILURE);
            }
            
		}
	}
	
	verbose("Successful processed response");
}
