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
#include "types_module.h"
#include "utility_coordinates.h"
#include "utility_port.h"



port p;
int idxOfferts, sum_offerID, sem_sum_id;

void printPort(port p, int i) {
	char *string;
	int numBytes;
	string=malloc(70);
	numBytes=sprintf(string,"Porto[%d] numero %d: (%.2f, %.2f) - %d banchine\n", getpid(),i, p.coords.x, p.coords.y, p.docks);

	fflush(stdout);
	write(1, string, numBytes);
	free(string);
}

void handleSignal(int signal) {
	switch(signal) {
		case SIGUSR1:
			printf("SEGNALE SIGUSR1 RICEVUTO\n");
			generateOffer(p, ++idxOfferts, sum_offerID, sem_sum_id);
			printf("Cazzo\n");
			break;

		case SIGUSR2:
			printf("SEGNALE SIGUSR2 RICEVUTO\n");
			printDailyReport(p);
			/*printf("STAMPATO REPORT GIORNALIERO\n");*/
			break;
	}
}

int main(int argc, char *argv[]) {
	sigset_t set;
	int i, *ptr_set;
	int sem_sync_id, portsSharedMemoryID, idx, sum_requestID, *sum_request, msg_id, sh_request_id;
	coordinates coords;
	struct sembuf sops;
	struct port_sharedMemory *shared_portCoords;
	struct request *shared_request;
	struct sigaction sa;
	struct msg_request msg_request;
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
	sem_sum_id = atoi(argv[4]);
	msg_id = atoi(argv[5]);
	idx = atoi(argv[6]);
	sum_offerID = atoi(argv[7]);

	shared_portCoords = shmat(portsSharedMemoryID, NULL, 0);
	TEST_ERROR;

	sum_request = shmat(sum_requestID, NULL, 0);
	TEST_ERROR;

	if (idx > 3) {
		coords = getRandomCoords();
	}
	else {
		switch(idx) {
			case 0:
				coords.x = 0.0;
				coords.y = 0.0;
				break;

			case 1:
				coords.x = SO_LATO;
				coords.y = 0.0;
				break;

			case 2:
				coords.x = 0.0;
				coords.y = SO_LATO;
				break;

			case 3:
				coords.x = SO_LATO;
				coords.y = SO_LATO;
				break;
		}
	}

	/*dopo che il porto inserisci i suoi dati, non ha più bisogno di accedere alla memoria*/
	shared_portCoords[idx].coords=coords;
	shared_portCoords[idx].pid=getpid();
	shared_portCoords[idx].offersID=shmget(IPC_PRIVATE, SO_DAYS*sizeof(goods), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	p.generatedGoods=shmat(shared_portCoords[idx].offersID, NULL, 0);
	shmctl(shared_portCoords[idx].offersID, IPC_RMID, NULL); TEST_ERROR;
	shared_portCoords[idx].requestID = shmget(IPC_PRIVATE, sizeof(struct request), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	p.request = shmat(shared_portCoords[idx].requestID, NULL, 0); TEST_ERROR;

	srand(getpid());
	p.docks = rand() % SO_BANCHINE + 1;
	shared_portCoords[idx].semID = semget(IPC_PRIVATE, 3, IPC_CREAT | 0600); /*3 semaphores: sem[0]=docks, sem[1]= offers handling, sem[2]=request handling*/ TEST_ERROR;
	semctl(shared_portCoords[idx].semID, 0, SETVAL, p.docks); TEST_ERROR;
	semctl(shared_portCoords[idx].semID, 1, SETVAL, 1); TEST_ERROR;
	semctl(shared_portCoords[idx].semID, 2, SETVAL, 1); TEST_ERROR;
	p.coords = coords;
    printf("semID in port: %d\n", shared_portCoords[idx].semID);

	printPort(p, idx);
	/*p = */initializeRequestsAndOffer(p);
	generateRequest(p, sum_requestID, sem_sum_id);
	generateOffer(p, 0, sum_offerID, sem_sum_id);

	msg_request.mtype = p.request -> goodsType;
	msg_request.idx = idx;

	msgsnd(msg_id, &msg_request, sizeof(struct msg_request), 0); TEST_ERROR;

	sops.sem_num = 0; /*semaforo di sincronizzazione*/
	sops.sem_op = -1;
	semop(sem_sync_id, &sops, 1); TEST_ERROR;
	sops.sem_num = 0;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1); TEST_ERROR;

	printf("Ciao dal porto\n");

	sigemptyset(&set);
	sigaddset(&set, SIGUSR2);

	for(i=0; i<SO_DAYS - 1; i++) {
		printf("%d", i);
		sigwait(&set, ptr_set);
	}

	shmdt(p.request); TEST_ERROR;
	semctl(shared_portCoords[idx].semID, 0, IPC_RMID); TEST_ERROR;
	shmctl(shared_portCoords[idx].requestID, IPC_RMID, NULL); TEST_ERROR;
	shmdt(shared_portCoords);
	/*shmdt(p.generatedGoods);*/
	/*free(p.generatedGoods);*/

	sops.sem_num = 1;
	sops.sem_op = -1;
	semop(sem_sync_id, &sops, 1); TEST_ERROR;

	exit(0);
}