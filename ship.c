#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct coordinates_t {
	double x;
	double y;
}coordinates_t;

typedef struct ship_t {
	int speed; /*in km, uguale a SO_SPEED (uguale per tutte le navi)*/
	coordinates_t coord; /*una coppia di coordinate (x, y)*/
	int capacity; /*in tonnellate, uguale a SO_CAPACITY*/
}ship_t;

#define SO_SPEED 40
#define SO_CAPACITY 60
#define SO_LATO 100

void printShip(ship_t ship) {
	printf("Ship %d: (%.2f, %.2f) - S = %d - C = %d\n", getpid(), ship.coord.x, ship.coord.y, ship.speed, ship.capacity);
}

int main() {
	ship_t ship;
	ship.coord.x = SO_LATO / 2;
	ship.coord.y = SO_LATO / 2;
	ship.speed = SO_SPEED;
	ship.capacity = SO_CAPACITY;
	printShip(ship);	
}