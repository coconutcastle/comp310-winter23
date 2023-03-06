CC=gcc
CFLAGS=

mysh: shell.c interpreter.c shellmemory.c processControlBlock.c ready_queue.c
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c processControlBlock.c ready_queue.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o processControlBlock.o ready_queue.o

clean: 
	rm mysh; rm *.o
