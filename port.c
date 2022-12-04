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
	int fifo_fd, sem_id;
	char name_fifo[100];
	struct coordinates coord;
	struct sembuf sops;
	sprintf(name_fifo, "%d", getpid());
	fifo_fd = open(name_fifo, O_RDONLY);
	TEST_ERROR;
	read(fifo_fd, &coord, sizeof(struct coordinates));
	close(fifo_fd);
	TEST_ERROR;
	unlink(name_fifo);	
	TEST_ERROR;
	printf("unlink\n");
	srand(getpid());
	p.docks = rand() % SO_BANCHINE + 1;
	printPort(p);
	sem_id = atoi(argv[1]);
	bzero(&sops, sizeof(sops));
	sops.sem_op = -1;
	semop(sem_id, &sops, 1);
	TEST_ERROR;
	sops.sem_op = 0;
	semop(sem_id, &sops, 1);
	TEST_ERROR;
	printf("Sono il porto %d\n", getpid());
	exit(0);
}