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
	int sem_sync_id, sem_report_id, shm_id, idx, i;
	coordinates coord;
	struct sembuf sops;
	struct shared_port *port_coordinates;
	struct sigaction sa;
	coordinates coordv[SO_PORTI];

	bzero(&sa, sizeof(sa));
	sa.sa_handler = handleSignal;
	sigaction(SIGUSR1, &sa, NULL);
	bzero(&p, sizeof(p));
	sem_sync_id = atoi(argv[1]);
	shm_id = atoi(argv[2]);
	idx = atoi(argv[3]);
	sem_report_id = atoi(argv[4]);
	port_coordinates = shmat(shm_id, NULL, 0);
	TEST_ERROR;
	if (idx > 3) {
		LOCK;
		idx = port_coordinates -> cur_idx;
		for (i = 0; i < idx; i++) {
			printf("%.2f %.2f\n", port_coordinates -> coords[i].x, port_coordinates -> coords[i].y);
		}
		do {
			coord = getRandomCoords();
		}
		while (existCoords(port_coordinates -> coords, idx, coord));
		port_coordinates -> cur_idx++;
		port_coordinates -> coords[idx] = coord;
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
	shmdt(port_coordinates);
	TEST_ERROR;
	srand(getpid());
	p.docks = rand() % SO_BANCHINE + 1;
	printPort(p);
	sops.sem_num = 0;
	sops.sem_op = -1;
	semop(sem_sync_id, &sops, 1);
	TEST_ERROR;
	sops.sem_num = 0;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1);
	TEST_ERROR;
	p = initializePort(p);
	generateOffer(p, 0);
	generateRequest(p, 0);
	sops.sem_num = 0;
	sops.sem_op = -1;
	semop(sem_report_id, &sops, 1);
	printDailyReport(p);
	sops.sem_op = 1;
	semop(sem_report_id, &sops, 1);
	sleep(1);
	exit(0);
}
