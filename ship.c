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
#include <sys/msg.h>

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
	int index, portsSharedMemoryID;
	int i;
	switch(signal) {
		case SIGUSR1:
			
			printf("\nNAVE IN POSIZIONE (%f,%f):\n", s.coords.x, s.coords.y);
			index= getNearestPort(shared_portCoords, s.coords, 0);
			printf("\n\nINDICE === %d", index);
			printf("Il porto più vicino a questo porto è in posizione (%f,%f) (PID: %d)) \n", shared_portCoords[index].coords.x, shared_portCoords[index].coords.y,shared_portCoords[index].pid);
			printf("\nSegnale personalizzato della nave [%d] intercettato\n", getpid());
			
			break;


		case SIGSTOP:
			printf("\n[%d]IMPREVISTO METEO! Le operazioni della nave saranno compromesse...", getpid());
			break;

		case SIGCONT:
			printf("\n[%d]MALTEMPO FINITO! Le operazioni della nave possono riprendere...",getpid());
			break;

		case SIGINT:
			printf("\n[%d]NAVE AFFONDATA!", getpid());
			//TODO pulire IPCS
			break;
	}
}

int main(int argc, char *argv[]) {
	int sem_sync_id, portsSharedMemoryID;
	int i, msg_id;
	struct sembuf sops;
	struct msg_request msg_request;
	struct sigaction sa;


	shared_portCoords = shmat(atoi(argv[2]), NULL, 0);

	/*
	printf("\n");
	for(i=0; i<SO_PORTI; i++){
		printf("\n[%d] (%f, %f)", shared_portCoords[i].pid, shared_portCoords[i].coords.x,shared_portCoords[i].coords.y);
	}
	printf("\n");

	*/

	bzero(&sa, sizeof(sa));
	
	sa.sa_handler = handleSignal;
	sigaction(SIGUSR1, &sa, NULL);

	bzero(&sops, sizeof(sops));

	s.coords = getRandomCoords();
	printShip(s);
	sem_sync_id = atoi(argv[1]);
	portsSharedMemoryID = atoi(argv[2]);
	sops.sem_num = 0;
	sops.sem_op = -1;
	semop(sem_sync_id, &sops, 1);
	TEST_ERROR;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1);
	TEST_ERROR;
	msg_id = msgget(getppid(), IPC_CREAT | 0600); TEST_ERROR;
	shared_portCoords = shmat(portsSharedMemoryID, NULL, 0);
	TEST_ERROR;
	sleep(2);
	exit(0);
}