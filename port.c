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
#include "goods.h"
#include "port.h"


void printPort(struct port p) {
	printf("Porto %d: (%.2f, %.2f) - %d banchine\n", getpid(), p.coord.x, p.coord.y, p.docks);
}

int main() {
	struct port p;
	int fifo_fd;
	char name_fifo[100];
	struct coordinates coord;
	sprintf(name_fifo, "%d", getpid());
	fifo_fd = open(name_fifo, O_RDONLY);
	read(fifo_fd, &coord, sizeof(struct coordinates));
	close(fifo_fd);
	unlink(name_fifo);	
	p.coord = coord;
	srand(getpid());
	p.docks = rand() % SO_BANCHINE + 1;
	printPort(p);
}