OBJETOS = main.o memory.o client.o driver.o restaurante.o process.o synchronization.o configuration.o metime.o log.o mesignal.o stats.o
main.o = main.h process.h main-private.h synchronization.h configuration.h metime.h log.h mesignal.h stats.h
memory.o = memory.h main.h
client.o = memory.h main.h client.h 
driver.o = driver.h main.h memory.h
restaurante.o = main.h memory.h restaurante.h
process.o = client.h driver.h main.h memory.h restaurante.h process.h mesignal.h
synchronization.o = synchronization.h
metime.o = client.h driver.h restaurante.h main.h memory.h
configuration.o = configuration.h main-private.h
log.o = log.h metime.h
mesignal.o = mesignal.h main.h main-private.h
stats.o = stats.h main.h



OBJ_dir = obj
CFLAGS = -Wall -g
CC = gcc
SRC_dir = src
LIBS = -lrt -lpthread


magnaeats: $(OBJETOS)
	$(CC) $(addprefix $(OBJ_dir)/,$(OBJETOS)) -o magnaeats $(LIBS)

%.o: $(SRC_dir)/%.c $($@)
	$(CC) $(CFLAGS) -o $(OBJ_dir)/$@ -c $<

clean:
	rm obj/*.o
	rm bin/magnaeats