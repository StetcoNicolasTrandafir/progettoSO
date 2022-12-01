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

typedef struct port_t {
	coordinates_t coord; /*una coppia di coordinate (x, y)*/
	int docks; /*numero di banchine, estratto casualmente tra 1 e SO_BANCHINE*/
}port_t;

#define SO_BANCHINE 8

void printPort(port_t port) {
	printf("Porto %d: (%.2f, %.2f) - %d banchine\n", getpid(), port.coord.x, port.coord.y, port.docks);
}

int main() {
	port_t port;
	int fifo_fd;
	char name_fifo[100];
	coordinates_t coord;
	sprintf(name_fifo, "%d", getpid());
	fifo_fd = open(name_fifo, O_RDONLY);
	read(fifo_fd, &coord, sizeof(coordinates_t));
	close(fifo_fd);
	unlink(name_fifo);	
	port.coord = coord;
	srand(getpid());
	port.docks = rand() % SO_BANCHINE + 1;
	printPort(port);
}