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
#include "coordinates.h"

#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

struct ship {
	struct coordinates coord; /*una coppia di coordinate (x, y)*/
	int capacity; /*in tonnellate, uguale a SO_CAPACITY*/
};

void printShip(struct ship s) {
	printf("Ship %d: (%.2f, %.2f) - S = %.2f - C = %d\n", getpid(), s.coord.x, s.coord.y, SO_SPEED, s.capacity);
}

int main(int argc, char *argv[]) {
	int sem_id;
	struct ship s;
	struct sembuf sops;
	s.coord = getRandomCoords();
	s.capacity = SO_CAPACITY;
	printShip(s);
	sem_id = atoi(argv[1]);
	bzero(&sops, sizeof(sops));
	sops.sem_op = -1;
	semop(sem_id, &sops, 1);
	TEST_ERROR;
	sops.sem_op = 0;
	semop(sem_id, &sops, 1);
	TEST_ERROR;
	printf("Sono la nave %d\n", getpid());	
	exit(0);
}