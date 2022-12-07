#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
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
#include "utility_shm.h"
#include "utility_sem.h"

#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}


port p;

void printPort(port p) {
	printf("Porto %d: (%.2f, %.2f) - %d banchine\n", getpid(), p.coord.x, p.coord.y, p.docks);
}

void handleSignal(int signal) {
	switch(signal) {
		case SIGUSR1:
			/*generateOffer(p, 0);
			generateRequest(p,0);
			printDailyReport(p); */
			printf("\nSegnale personalizzato del porto [%d] intercettato\n", getpid());
			break;
	}
}


int main(int argc, char *argv[]) {
	
	int sem_id, portsSharedMemoryID, idx;
	coordinates coord;
	struct sembuf sops;
	struct port_sharedMemory *shared_portCoords;
	struct sigaction sa;


	bzero(&sa, sizeof(sa));
	sa.sa_handler = handleSignal;
	sigaction(SIGUSR1, &sa, NULL);

	bzero(&p, sizeof(p));
	sem_id = atoi(argv[1]);
	portsSharedMemoryID=atoi(argv[2]);
	
	/*shm_id = atoi(argv[2]);*/
	idx = atoi(argv[3]);


	shared_portCoords = shmat(portsSharedMemoryID, NULL, 0);

	TEST_ERROR;
	if (idx > 3) {
		coord = getRandomCoords();
/*
	LOCK;
		idx = port_coords -> cur_idx;
		do {
			coord = getRandomCoords();
		}
		while (existCoords(port_coords -> coords, idx, coord));
		port_coords -> cur_idx++;
		port_coords -> coords[idx] = coord;
		UNLOCK;
*/
		
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
		printf("\n\nPROCESSO PORTO, [%d] la mia posizione è X: %.2f Y: %.2f\n", getpid(), shared_portCoords[idx].coords.x, shared_portCoords[idx].coords.y);
	}
	

	shared_portCoords[idx].coords=coord;
	shared_portCoords[idx].pid=getpid();
	shmdt(shared_portCoords);

	p.coord = coord;
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
	p = initializePort(p);
	generateOffer(p, 0);
	generateRequest(p, 0);

	printDailyReport(p);
	printf("ciao");

	printf("\n\nFINE PROCESSO PORTO, la mia posizione è X: %.2f Y: %.2f\n", shared_portCoords[idx].coords.x, shared_portCoords[idx].coords.y);

	sleep(5);


	exit(0);
}
