CC = gcc
CFLAGS = -g -Wall
SERVEROBJECTS = ./bbserver/bbserver.c
CLIENTOBJECTS = ./bbpeer/bbwriter.c ./bbpeer/bbpeer.c
SERVERNAME = ./bbserver/bbserver
CLIENTNAME = ./bbpeer/bbpeer
TODELETE = $(SERVERNAME) $(CLIENTNAME) *.o
LIBS = -lpthread

all: $(SERVERNAME) $(CLIENTNAME)

$(SERVERNAME): $(SERVEROBJECTS)
	$(CC) $(CFLAGS) $(SERVEROBJECTS) -o $(SERVERNAME) $(LIBS)

$(CLIENTNAME): $(CLIENTOBJECTS)
	$(CC) $(CFLAGS) $(CLIENTOBJECTS) -o $(CLIENTNAME)

.PHONY: clean
clean:
	rm -f $(TODELETE)