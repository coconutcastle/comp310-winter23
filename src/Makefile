CC=gcc
CFLAGS=

mysh: shell.c interpreter.c shellmemory.c kernel.c pcb.c ready_queue.c
	$(CC) $(CFLAGS) -D X=$(fsize) -D Y=$(vsize) -c shell.c interpreter.c shellmemory.c kernel.c pcb.c ready_queue.c -lpthread
	# $(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c kernel.c pcb.c ready_queue.c -lpthread
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o kernel.o pcb.o ready_queue.o

clean: 
	rm mysh; rm *.o
