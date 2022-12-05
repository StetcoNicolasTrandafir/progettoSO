CC=gcc
CFLAGS=-std=c89 -Wpedantic
LDFLAGS=-lm

simulation: master.o utility_coordinates.o utility_goods.o utility_port.o port ship port 
	gcc -o simulation *.o $(LDFLAGS)

objects:
	gcc -c utility_coordinates.c
	gcc -c utility_goods.c
	gcc -c utility_port.c

port:
	gcc port.c utility_coordinates.o utility_goods.o utility_port.o -o port $(LDFLAGS)

ship:
	gcc ship.c utility_coordinates.o -o ship $(LDFLAGS)


setEnv:
	source ./setEnv.sh

clean: 
	rm -f *.o
	rm port
	rm ship
	rm simulation

run:
	./simulation