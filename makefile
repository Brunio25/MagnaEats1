exec: main.o memory.o
	gcc main.o memory.o -o exec




main.o: src/main.c include/main.h
	gcc -c src/main.c

memory.o: src/memory.c src/memory.h
	gcc -c src/memory.c -lrt



	 