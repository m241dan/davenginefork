CC      = gcc
C_FLAGS = -Wall -g -pedantic
L_FLAGS = -lz -lpthread -lcrypt -lm -ldl

O_FILES = socket.o io.o strings.o utils.o interpret.o help.o  \
	  list.o stack.o mccp.o event.o event-handler.o \
          account.o strings_table.o nanny.o

all: $(O_FILES)
	rm -f SocketMud
	$(CC) -o SocketMud $(O_FILES) $(L_FLAGS)

.c.o: all
	@echo [`date +%T`] Compiling $< ...
	@$(CC) -c $(C_FLAGS) $<

clean:
	@echo Cleaning code $< ...
	@rm -f *.o
	@rm -f SocketMud
	@rm -f *.*~
