CC=gcc
CFLAGS=-std=c89 -Wpedantic -O0 -g
LDFLAGS=-lm

simulation:  master.o utility_coordinates.o utility_goods.o utility_port.o utility_ship.o utility_meteo.o semaphore_library.o ship port meteo
	gcc -o simulation *.o $(LDFLAGS)


objects:
	gcc -c utility_coordinates.c
	gcc -c utility_goods.c
	gcc -c utility_port.c
	gcc -c semaphore_library.c

port:
	gcc port.c utility_coordinates.o utility_goods.o utility_port.o semaphore_library.o utility_meteo.o -o port $(LDFLAGS)

ship:
	gcc ship.c utility_coordinates.o utility_port.o utility_ship.o utility_goods.o semaphore_library.o utility_meteo.o -o ship $(LDFLAGS)

meteo: 
	gcc meteo.c utility_meteo.o utility_coordinates.o utility_port.o utility_ship.o utility_goods.o semaphore_library.o -o meteo $(LDFLAGS)

setEnv:
	source ./setEnv.sh

clean: 
	rm -f *.o
	rm port
	rm ship
	rm meteo
	rm simulation
	

run: simulation
	./simulation


log: simulation
	./simulation > log.txt