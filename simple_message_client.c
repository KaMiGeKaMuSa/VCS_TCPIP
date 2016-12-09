/**
 * @file simple_message_client.c
 * TCP/IP Server-Client project
 *
 * @author Karin Kalman <karin.kalman@technikum-wien.at>
 * @author Michael Mueller <michael.mueller@technikum-wien.at>
 * @author Gerhard Sabeditsch <gerhard.sabeditsch@technikum-wien.at>
 * @date 2016/12/02
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

/*
 * --------------------------------------------------- function prototypes --
 */
void usage(FILE * stream, const char * message, int exitcode);
void openSocket(int *paramISocketFD);
void sendRequest(int *paramISocketFD);
void readResponse(int *paramISocketFD);

 /*
 * ------------------------------------------------------------- main --
 */
int main(int argc, const char* argv[])
{	
	int iSocketFD = NULL;
	cpFilename = argv[0];
	
	/* function to parse parameter provided by Thomas M. Galla, Christian Fibich*/
	smc_parsecommandline(argc, argv, &usage, &cpServer, &cpPort, &cpUser, &cpMessage, &cpImage, &iVerbose);

	
	openSocket(&iSocketFD);   
	
	sendRequest(&iSocketFD);

	readResponse(&iSocketFD);
	
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
	fprintf(stream, message);
	exit(errcode);
}

void openSocket(int *paramISocketFD) 
{
	struct addrinfo hints, *socket_address, *rp;
	int iRetValue;
		
	/* set memory for struct hints */
	memset(&hints, 0, sizeof hints);
	
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
    hints.ai_protocol = 0;          /* Any protocol */
	
	/* get address info of socket */
	if ((iRetValue = getaddrinfo(cpServer, cpPort, &hints, &socket_address)) != 0) {
		fprintf(stderr,"%s - %s: %s\n", cpFilename, "getaddrinfo()", /*strerror(errno)*/"");
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
	
	if (rp == NULL) {
		fprintf(stderr,"%s - %s: %s\n", cpFilename, "socket(),connect()", "no address succeeded");
		exit(EXIT_FAILURE);
	}
	
	/* socket address info are no longer needed */
	freeaddrinfo(socket_address);   
}

void sendRequest(int *paramISocketFD) 
{
	FILE* fpWriteSocket = NULL;
	
	/* open socket file descriptor */
	if ((fpWriteSocket = fdopen(*paramISocketFD, "w")) == NULL) {
		fprintf(stderr,"%s - %s: %s\n", cpFilename, "fdopen()", /*strerror(errno)*/"open of write stream failed");
		exit(EXIT_FAILURE);
	}
	
	/* write user to stream */
	if (fprintf(fpWriteSocket,"user=%s\n",cpUser) < 0) {
		fprintf(stderr,"%s - %s: %s\n", cpFilename, "fprintf()", /*strerror(errno)*/"print of user failed");
		fclose(fpWriteSocket);
		exit(EXIT_FAILURE);
	}
	
	/* if img exist: write image to sream */
	if (cpImage != NULL) {
		if (fprintf(fpWriteSocket,"img=%s\n", cpImage) < 0) {
			fprintf(stderr,"%s - %s: %s\n", cpFilename, "fprintf()", /*strerror(errno)*/"print of img failed");
			fclose(fpWriteSocket);
			exit(EXIT_FAILURE);
		}
	}
	
	/* write message to stream */
	if (fprintf(fpWriteSocket,"%s\n", cpMessage) < 0) {
		fprintf(stderr,"%s - %s: %s\n", cpFilename, "fprintf()", /*strerror(errno)*/"print of message failed");
		fclose(fpWriteSocket);
		exit(EXIT_FAILURE);
	}
	
	/* flush all unwritten data to the stream */
	if (fflush(fpWriteSocket) != 0) {
		fprintf(stderr,"%s - %s: %s\n", cpFilename, "fflush()", /*strerror(errno)*/"flush of write stream failed");
		fclose(fpWriteSocket);
		exit(EXIT_FAILURE);
	}
	
	/* after writing: disable write operations for socket */
	if (shutdown(*paramISocketFD,SHUT_WR) != 0) {
		fprintf(stderr,"%s - %s: %s\n", cpFilename, "shutdown()", /*strerror(errno)*/"close of write direction failed");
		fclose(fpWriteSocket);
		exit(EXIT_FAILURE);
	}
	
	/* everthing fine - close file pointer */
	
	//fclose(fpWriteSocket);
}

void readResponse(int *paramISocketFD) 
{
	FILE* fpReadSocket = NULL;
	char cBuf[MAX_BUF];
	int iFirstLoop = 1;
	int iRecStatus, iRecLength;
	char *cResponseFilename = NULL;
	
	/* TODO: read response of stream and save in file with specified filename */
	if ((fpReadSocket = fdopen(*paramISocketFD, "r")) == NULL) {
		fprintf(stderr,"%s - %s: %s\n", cpFilename, "fdopen()", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/* loop over response */
	while(fgets(cBuf, MAX_BUF, fpReadSocket)) {
		/* first loop is to get status */
		if (iFirstLoop) {
			iFirstLoop = 0;
			errno = 0;
			
			/* get status of response */
			if (sscanf(cBuf,"status=%d",&iRecStatus) == 0) {
				if (errno != 0) {
					fprintf(stderr,"%s - %s: %s\n", cpFilename, "sscanf()", /*strerror(errno)*/"status could not be scanned");
					fclose(fpReadSocket);
					exit(EXIT_FAILURE);
				}
			}
			
		} else {
			/* get filename of response */
			if (sscanf(cBuf,"file=%s",cResponseFilename) == 0) {
				if (errno != 0) {
					fprintf(stderr,"%s - %s: %s\n", cpFilename, "sscanf()", /*strerror(errno)*/"filename could not be scanned");
					fclose(fpReadSocket);
					exit(EXIT_FAILURE);
				}
			}
			/*
			if (strncmp(cBuf, "file=", 5) != 0) 
			{
				fprintf(stderr,"%s - %s: %s\n", cpFilename, "strncmp()", strerror(errno));
				fclose(fpReadSocket);
				exit(EXIT_FAILURE);
			}
			if ((cResponseFilename = malloc((len(cBuf) - 6) * sizeof(char))) == NULL)
			{
				fprintf(stderr,"%s - %s: %s\n", cpFilename, "malloc()", strerror(errno));
				fclose(fpReadSocket);
				exit(EXIT_FAILURE);
			}
			*/
			
			/* get len of response */
			if (sscanf(cBuf,"len=%d",&iRecLength) == 0) {
				if (errno != 0) {
					fprintf(stderr,"%s - %s: %s\n", cpFilename, "sscanf()", /*strerror(errno)*/"len could not be scanned");
					fclose(fpReadSocket);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	
	printf("status = %d\n", iRecStatus);
	printf("length = %d\n",iRecLength);
	printf("filename = %s\n", cResponseFilename);
	
	/* everthing fine - close file pointer */
	fclose(fpReadSocket);
}
