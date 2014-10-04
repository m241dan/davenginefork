CC      = gcc -std=c99
C_FLAGS = -g3 -ggdb3 -Wall -pedantic `mysql_config --cflags`
L_FLAGS = -lcrypt `mysql_config --libs`

O_FILES = socket.o io.o strings.o utils.o interpret.o help.o  \
	  llist.o stack.o mccp.o event.o event-handler.o \
          account.o strings_table.o nanny.o id-handler.o \
          communication.o olc.o frameworks.o editor.o \
          specifications.o instances.o projects.o

all: $(O_FILES)
	rm -f SocketMud
	$(CC) -rdynamic -export-dynamic -o SocketMud $(O_FILES) $(L_FLAGS)

.c.o: all
	@echo [`date +%T`] Compiling $< ...
	@$(CC) -c $(C_FLAGS) $<

clean:
	@echo Cleaning code $< ...
	@rm -f *.o
	@rm -f SocketMud
	@rm -f *.*~
