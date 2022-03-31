exec: main.o memory.o
	gcc main.o memory.o -o exec -lrt




main.o: src/main.c include/main.h
	gcc -c src/main.c -g

memory.o: src/memory.c include/memory.h
	gcc -c src/memory.c -g -lrt

clean:
	rm *.o
	rm exec


	 