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
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "macro.h"
#include "utility_coordinates.h"
#include "utility_goods.h"
#include "utility_port.h"
#include "utility_sem.h"
#include "utility_shm.h"

#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

void printPort(struct port p) {
	printf("Porto %d: (%.2f, %.2f) - %d banchine\n", getpid(), p.coord.x, p.coord.y, p.docks);
}

int main(int argc, char *argv[]) {
	struct port p;
	int fifo_fd, sem_id, shm_id, idx;
	char name_fifo[100];
	struct coordinates coord;
	struct sembuf sops;
	struct shared_port *port_coords;

	/*sprintf(name_fifo, "%d", getpid());
	fifo_fd = open(name_fifo, O_RDONLY);
	TEST_ERROR;
	read(fifo_fd, &coord, sizeof(struct coordinates));
	close(fifo_fd);
	TEST_ERROR;
	unlink(name_fifo);	
	TEST_ERROR;*/

	sem_id = atoi(argv[1]);
	idx = atoi(argv[3]);
	shm_id = atoi(argv[2]);
	port_coords = shmat(shm_id, NULL, 0);
	TEST_ERROR;
	if (idx > 3) {
		LOCK;
		idx = port_coords -> cur_idx;
		do {
			coord = getRandomCoords();
		}
		while (existCoords(port_coords -> coords, idx, coord));
		port_coords -> cur_idx++;
		port_coords -> coords[idx] = coord;
		UNLOCK;
	}
	else {
		switch(idx) {
			case 0:
				coord.x = 0.0;
				coord.y = 0.0;
				break;

			case 1:
				coord.x = SO_LATO;
				coord.y = 0.0;
				break;

			case 2:
				coord.x = 0.0;
				coord.y = SO_LATO;
				break;

			case 3:
				coord.x = SO_LATO;
				coord.y = SO_LATO;
				break;
		}
	}
	p.coord = coord;
	shmdt(port_coords);
	srand(getpid());
	p.docks = rand() % SO_BANCHINE + 1;
	printPort(p);
	sops.sem_num = 0;
	sops.sem_op = -1;
	semop(sem_id, &sops, 1);
	TEST_ERROR;
	sops.sem_num = 0;
	sops.sem_op = 0;
	semop(sem_id, &sops, 1);
	TEST_ERROR;
	exit(0);
}
