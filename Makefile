CC = gcc
CFLAGS = -g -Wall
SERVEROBJECTS = ./bbserver/bbserver.c common.c
CLIENTOBJECTS = ./bbpeer/bbwriter.c ./bbpeer/bbpeer.c common.c
SERVERNAME = ./bbserver/bbserver
CLIENTNAME = ./bbpeer/bbpeer
TODELETE = $(SERVERNAME) $(CLIENTNAME) *.o
LIBS = -pthread

all: $(SERVERNAME) $(CLIENTNAME)

$(SERVERNAME): $(SERVEROBJECTS)
	$(CC) $(CFLAGS) $(SERVEROBJECTS) $(LIBS) -o $(SERVERNAME) 

$(CLIENTNAME): $(CLIENTOBJECTS)
	$(CC) $(CFLAGS) $(CLIENTOBJECTS) $(LIBS) -o $(CLIENTNAME)

.PHONY: clean
clean:
	rm -f $(TODELETE)