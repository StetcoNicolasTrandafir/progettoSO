#define _GNU_SOURCE

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
#include "utility_ship.h"


#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

void printShip(ship s) {
	printf("Ship %d: (%.2f, %.2f) - S = %.2f - C = %d\n", getpid(), s.coords.x, s.coords.y, SO_SPEED, SO_CAPACITY);
}


struct port_sharedMemory *shared_portCoords;

ship s;


void handleSignal(int signal) {
	int index;
	int i;
	switch(signal) {
		case SIGUSR1:
			printf("\n\n\nPORTI\n\n:");
			for(i=0; i<SO_PORTI; i++){	
				printCoords(shared_portCoords[i].coords);
				printf(" (%d)\n\n ",shared_portCoords[i].pid);
			}
			printf("\nNAVE IN POSIZIONE (%f,%f):\n", s.coords.x, s.coords.y);
			index= getNearestPort(shared_portCoords, s.coords, 2*SO_LATO);
			printf("Il porto più vicino a questo porto è in posizione (%f,%f) (PID: %d)) \n", shared_portCoords[i].coords.x, shared_portCoords[i].coords.y,shared_portCoords[i].pid);
			printf("\nSegnale personalizzato della nave [%d] intercettato\n", getpid());
			break;
	}
}


int main(int argc, char *argv[]) {
	int sem_id;
	int i;
	struct sembuf sops;

	bzero(&sops, sizeof(sops));

	s.coords = getRandomCoords();
	printShip(s);
	sem_sync_id = atoi(argv[1]);
	sops.sem_num = 0;
	sops.sem_op = -1;
	semop(sem_sync_id, &sops, 1);
	TEST_ERROR;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1);
	TEST_ERROR;
	sleep(20);
	exit(0);
}