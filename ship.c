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
#include "types_module.h"
#include "utility_port.h"
#include "utility_coordinates.h"
#include "utility_ship.h"

void printShip(ship s) {
	char *string;
	int numBytes;
	string=malloc(30);
	numBytes=sprintf(string,"Ship[%d]: (%.2f, %.2f)\n", getpid(), s.coords.x, s.coords.y);

	fflush(stdout);
	write(1, string, numBytes);
	free(string);

}


struct port_sharedMemory *shared_portCoords;
ship s;

void handleSignal(int signal) {
	int index, portsSharedMemoryID;
	int i;
	char *string;
	int numBytes;
	switch(signal) {
		case SIGUSR1:
			/*
			printf("\nNAVE IN POSIZIONE (%f,%f):\n", s.coords.x, s.coords.y);
			index= getNearestPort(shared_portCoords, s.coords, 0);
			printf("\n\nINDICE === %d", index);
			printf("Il porto più vicino a questo porto è in posizione (%f,%f) (PID: %d)) \n", shared_portCoords[index].coords.x, shared_portCoords[index].coords.y,shared_portCoords[index].pid);
			printf("\nSegnale personalizzato della nave [%d] intercettato\n", getpid());
			*/
			break;

        case SIGALRM:
            break;


		/*SIGSTOP:
			string=malloc(76);
			numBytes=sprintf(string,"\n[%d]IMPREVISTO METEO! Le operazioni della nave saranno compromesse...", getpid());
			fflush(stdout);
			write(1, string, numBytes);
			break;*/

		case SIGCONT:
			string=malloc(74);
			numBytes=sprintf(string,"\n[%d]MALTEMPO FINITO! Le operazioni della nave possono riprendere...",getpid());
			fflush(stdout);
			write(1, string, numBytes);

			break;

		case SIGINT:
			string=malloc(27);
			numBytes=sprintf(string,"\n[%d]NAVE AFFONDATA!\n", getpid());
			fflush(stdout);
			write(1, string, numBytes);
			//TODO pulire IPCS
			break;
	}
}

int main(int argc, char *argv[]) {
	sigset_t set;
	int sem_sync_id, portsSharedMemoryID;
	int i, msg_id, *ptr_set;
	struct sembuf sops;
	struct msg_request msg_request;
	struct sigaction sa;

	shared_portCoords = shmat(atoi(argv[2]), NULL, 0); TEST_ERROR;

	/*
	printf("\n");
	for(i=0; i<SO_PORTI; i++){
		printf("\n[%d] (%f, %f)", shared_portCoords[i].pid, shared_portCoords[i].coords.x,shared_portCoords[i].coords.y);
	}
	printf("\n");
	*/
	TEST_ERROR;
	bzero(&sa, sizeof(sa));TEST_ERROR;
	TEST_ERROR;
	sa.sa_handler = handleSignal;TEST_ERROR;
	sigaction(SIGUSR1, &sa, NULL);TEST_ERROR;
	sigaction(SIGALRM, &sa, NULL);TEST_ERROR;
	sigaction(SIGCONT, &sa, NULL);TEST_ERROR;
	sigaction(SIGINT, &sa, NULL);TEST_ERROR;
	

	bzero(&sops, sizeof(sops));

	s.coords = getRandomCoords();
	printShip(s);TEST_ERROR;
	sem_sync_id = atoi(argv[1]);
	portsSharedMemoryID = atoi(argv[2]);TEST_ERROR;
	sops.sem_num = 0;
	sops.sem_op = -1;
	semop(sem_sync_id, &sops, 1);TEST_ERROR;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1);TEST_ERROR;


	msg_id = msgget(getppid(), IPC_CREAT | 0600); TEST_ERROR;
	shared_portCoords = shmat(portsSharedMemoryID, NULL, 0); TEST_ERROR;
	
	negociate(shared_portCoords, s); TEST_ERROR;

	/*getNearestPort(shared_portCoords, s.coords,-1); TEST_ERROR;*/

	sigemptyset(&set);
	sigaddset(&set, SIGALRM);

	for(i=0; i<SO_DAYS; i++) {
		sigwait(&set, ptr_set); TEST_ERROR;
	}


	sops.sem_num = 1;
	sops.sem_op = -1;
	semop(sem_sync_id, &sops, 1); TEST_ERROR;

	exit(0);
}