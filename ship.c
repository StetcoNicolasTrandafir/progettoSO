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

#include "macro.h"
#include "coordinates.h"

struct ship {
	struct coordinates coord; /*una coppia di coordinate (x, y)*/
	int capacity; /*in tonnellate, uguale a SO_CAPACITY*/
};

void printShip(struct ship s) {
	printf("Ship %d: (%.2f, %.2f) - S = %.2f - C = %d\n", getpid(), s.coord.x, s.coord.y, SO_SPEED, s.capacity);
}

int main() {
	struct ship s;
	s.coord.x = SO_LATO / 2;
	s.coord.y = SO_LATO / 2;
	s.capacity = SO_CAPACITY;
	printShip(s);	
}