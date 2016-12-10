##
## @file Makefile
## SIMPLE_MESSAGE_CLIENT and SIMPLE_MESSAGE_Server
## TCP
##
## @author Karin Kalman <karin.kalman@technikum-wien.at>
## @author Michael Mueller <michael.mueller@technikum-wien.at>
## @author Gerhard Sabeditsch <gerhard.sabeditsch@technikum-wien.at>
## @date 2016/12/10
##
## @version $Revision: 1.1 $
##
##
## Last Modified: $Author: Gerhard $
##

##
## ------------------------------------------------------------- variables --
##

CC=gcc52
CFLAGS=-DDEBUG -Wall -Werror -Wextra -Wstrict-prototypes -pedantic -fno-common -g -O3
CD=cd
CP=cp
MV=mv
GREP=grep
DOXYGEN=doxygen

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o: %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##


all: simple_message_client simple_message_server

simple_message_client: simple_message_client_commandline_handling.o simple_message_client.o
	$(CC) $(OPTFLAGS) simple_message_client_commandline_handling.o simple_message_client.o -o simple_message_client
	$(RM) *.o
	
simple_message_server: simple_message_server_commandline_handling.o simple_message_server.o
	$(CC) $(OPTFLAGS) simple_message_server_commandline_handling.o simple_message_server.o -o simple_message_server
	$(RM) *.o
	
clean:
	$(RM) *.o *~  simple_message_client simple_message_server
	
distclean: clean
	$(RM) -r doc

doc: html pdf

html:
	$(DOXYGEN) doxygen.dcf

pdf: html
	$(CD) doc/pdf && \
	$(MV) refman.tex refman_save.tex && \
	$(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
	$(RM) refman_save.tex && \
	make && \
	$(MV) refman.pdf refman.save && \
	$(RM) *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
	      *.ilg *.toc *.tps Makefile && \
	$(MV) refman.save refman.pdf
