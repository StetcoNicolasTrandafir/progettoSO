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
#include <sys/msg.h>

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
			generateRequest(p); */
			printDailyReport(p);
			printf("\nSegnale personalizzato del porto [%d] intercettato\n", getpid());
			break;
	}
}

int main(int argc, char *argv[]) {
	
	int sem_sync_id, sem_request_id, portsSharedMemoryID, idx, sum_requestID, *sum_request, msg_id;
	coordinates coord;
	struct sembuf sops;
	struct port_sharedMemory *shared_portCoords;
	struct sigaction sa;
	struct msg_request msg_request;

	bzero(&p, sizeof(p));
	bzero(&sa, sizeof(sa));
	bzero(&sops, sizeof(sops));
	
	sa.sa_handler = handleSignal;
	sigaction(SIGUSR1, &sa, NULL);

	sem_sync_id = atoi(argv[1]);
	portsSharedMemoryID=atoi(argv[2]);
	sum_requestID = atoi(argv[3]);
	sem_request_id = atoi(argv[4]);
	idx = atoi(argv[5]);

	shared_portCoords = shmat(portsSharedMemoryID, NULL, 0);
	TEST_ERROR;

	sum_request = shmat(sum_requestID, NULL, 0);
	TEST_ERROR;

	if (idx > 3) {
		coord = getRandomCoords();
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
	

	/*dopo che il porto inserisci i suoi dati, non ha più bisogno di accedere alla memoria*/
	shared_portCoords[idx].coords=coord;
	shared_portCoords[idx].pid=getpid();
	shmdt(shared_portCoords);

	srand(getpid());
	p.docks = rand() % SO_BANCHINE + 1;
	p.coord = coord;
	
	printPort(p);

	sops.sem_num = 0;
	sops.sem_op = -1;
	semop(sem_sync_id, &sops, 1); TEST_ERROR;
	sops.sem_num = 0;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1); TEST_ERROR;

	p = initializePort(p);
	generateOffer(p, 0);
	p.request = generateRequest(p);

	sops.sem_num = 0;
	sops.sem_op = -1;
	semop(sem_request_id, &sops, 1); TEST_ERROR;

	*sum_request += p.request.quantity;

	sops.sem_num = 0;
	sops.sem_op = 1;
	semop(sem_request_id, &sops, 1); TEST_ERROR;

	sops.sem_num = 1;
	sops.sem_op = -1;
	semop(sem_request_id, &sops, 1); TEST_ERROR;

	sops.sem_num = 1;
	sops.sem_op = 0;
	semop(sem_request_id, &sops, 1); TEST_ERROR;

	if (idx == 0) {
		p.request.quantity += SO_FILL - *sum_request;
	}

	msg_id = msgget(getppid(), IPC_CREAT | 0600);
	msg_request.mtype = p.request.goodsType;
	msg_request.idx = idx;
	msg_request.quantity = p.request.quantity;
	msgsnd(msg_id, &msg_request, sizeof(msg_request), 0);
	shmdt(sum_request);

	printDailyReport(p);

	sleep(1);

	exit(0);
}
