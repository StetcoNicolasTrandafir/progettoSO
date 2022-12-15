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
#include <math.h>

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
int idxOfferts;

void printPort(port p) {
	printf("Porto %d: (%.2f, %.2f) - %d banchine\n", getpid(), p.coord.x, p.coord.y, p.docks);
}

void handleSignal(int signal) {
	switch(signal) {
		case SIGUSR1:
			printDailyReport(p);
			generateOffer(p, ++idxOfferts);
			break;
		case SIGUSR2:
			printDailyReport(p);
			break;
	}
}

int main(int argc, char *argv[]) {
	
	int i;
	int sem_sync_id, sem_request_id, portsSharedMemoryID, idx, sum_requestID, *sum_request, msg_id;
	coordinates coord;
	struct sembuf sops;
	struct port_sharedMemory *shared_portCoords;
	struct sigaction sa;
	struct msg_request msg_request;
	int portSemId;
	goods *g;

	bzero(&p, sizeof(p));
	bzero(&sa, sizeof(sa));
	bzero(&sops, sizeof(sops));
	
	sa.sa_handler = handleSignal;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);

	sem_sync_id = atoi(argv[1]);
	portsSharedMemoryID=atoi(argv[2]);
	sum_requestID = atoi(argv[3]);
	sem_request_id = atoi(argv[4]);
	msg_id = atoi(argv[5]);
	idx = atoi(argv[6]);

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
	shared_portCoords[idx].offersID=shmget(IPC_PRIVATE, SO_DAYS*sizeof(goods),S_IRUSR | S_IWUSR | IPC_CREAT);
	p.generatedGoods=shmat(shared_portCoords[idx].offersID, NULL, 0);
	shmdt(shared_portCoords);

	srand(getpid());
	p.docks = rand() % SO_BANCHINE + 1;
	portSemId=semget(getpid(),3,0600); /*3 semaphores: sem[0]=docks, sem[1]= offers handling, sem[2]=???*/
	semctl(portSemId, 0, SETVAL, p.docks); 
	semctl(portSemId, 1, SETVAL, 1); 
	p.coord = coord;

	printPort(p);
	/*p = */initializeRequestsAndOffer(p);
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

	if((p.request.quantity = round((p.request.quantity * SO_FILL) / *sum_request)) == 0)
		p.request.quantity++;
	/*if (idx == 0) {
		p.request.quantity += SO_FILL - round(*sum_request / SO_FILL);
	}*/

	msg_request.mtype = p.request.goodsType;
	msg_request.idx = idx;
	msgsnd(msg_id, &msg_request, sizeof(struct msg_request), 0); TEST_ERROR;

	shmdt(sum_request); TEST_ERROR;

	sops.sem_num = 0; /*semaforo di sincronizzazione*/
	sops.sem_op = -1;
	semop(sem_sync_id, &sops, 1); TEST_ERROR;
	sops.sem_num = 0;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1); TEST_ERROR;

	for(i=0; i<SO_DAYS; i++)
		sleep(2);

	shmdt(p.generatedGoods);
	free(p.generatedGoods);

	exit(0);
}