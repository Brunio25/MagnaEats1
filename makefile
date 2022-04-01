OBJETOS = main.o memory.o
main.o = main.h
memory.o = memory.h
OBJ_dir = obj
CFLAGS = -Wall
CC = gcc
SRC_dir = src
LIBS = -lrt



magnaeats: $(OBJETOS)
	$(CC) $(addprefix $(OBJ_dir)/,$(OBJETOS)) -o bin/magnaeats $(LIBS)

%.o: $(SRC_dir)/%.c $($@)
	$(CC) $(CFLAGS) -o $(OBJ_dir)/$@ -c $<

clean:
	rm obj/*.o
	rm bin/magnaeats


	 