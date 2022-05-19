OBJETOS = main.o memory.o client.o driver.o restaurante.o process.o synchronization.o metime.o
main.o = main.h process.h main-private.h synchronization.h
memory.o = memory.h main.h
client.o = memory.h main.h client.h
driver.o = driver.h main.h memory.h
restaurante.o = main.h memory.h restaurante.h
process.o = client.h driver.h main.h memory.h restaurante.h process.h
synchronization.o = synchronization.h synchronization.c
metime.o = client.h driver.h restaurante.h main.h memory.h



OBJ_dir = obj
CFLAGS = -Wall -g
CC = gcc
SRC_dir = src
LIBS = -lrt -lpthread -lm


magnaeats: $(OBJETOS)
	$(CC) $(addprefix $(OBJ_dir)/,$(OBJETOS)) -o magnaeats $(LIBS)

%.o: $(SRC_dir)/%.c $($@)
	$(CC) $(CFLAGS) -o $(OBJ_dir)/$@ -c $<

clean:
	rm obj/*.o
	rm bin/magnaeats