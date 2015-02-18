C = gcc
CFLAGS = -g -Wall
OBJECTS = *.c
NAME = bbpeer
TODELETE = $(NAME) *.o
LIBS = -lpthread

$(NAME) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME) $(LIBS)

.PHONY: clean
clean:
	rm -f $(TODELETE)
