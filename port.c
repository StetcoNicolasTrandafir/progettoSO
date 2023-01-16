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
#include <time.h>

#include "macro.h"
#include "semaphore_library.h"
#include "types_module.h"
#include "utility_meteo.h"
#include "utility_coordinates.h"
#include "utility_port.h"


port p;
int idxOffer = 0, sum_offerID, sem_sum_id, idx, pastDays = 0, sem_sync_id;
struct port_sharedMemory *shared_portCoords;

void cleanUp() {
	struct sembuf sops;
	bzero(&sops, sizeof(struct sembuf));
	waitForZero(sops, sem_sync_id, 2); TEST_ERROR;
	/*free(p.generatedGoods); TEST_ERROR;*/
	shmdt(p.request); TEST_ERROR;
	semctl(shared_portCoords[idx].semID, 0, IPC_RMID); TEST_ERROR;
	shmdt(shared_portCoords); TEST_ERROR;
	shmdt(p.generatedGoods); TEST_ERROR;
	
	decreaseSem(sops, sem_sync_id, 1); TEST_ERROR;
}

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
	int prevErrno=errno;
	int i;
	int freeDocks;
	struct sembuf sops;
	struct timespec sleepTime, rem;

	bzero(&sops, sizeof(struct sembuf));

	errno=0;
	
	switch(signal) {
		case SIGUSR1:
			idxOffer=generateOffer(p, idxOffer, sum_offerID, shared_portCoords[idx].semID);
			break;

		case SIGUSR2:
			sleepTime = getSwellDuration();
			freeDocks = semctl(shared_portCoords[idx].semID, DOCK, GETVAL); TEST_ERROR;

			for(i=0; i<freeDocks; i++)
			{
				decreaseSem(sops,shared_portCoords[idx].semID,DOCK); TEST_ERROR;
			}

			nanosleep(&sleepTime, &rem);
		    if(errno==EINTR){
		        errno=0;
		        while(nanosleep(&rem, &rem)==-1)
		        {
		            if(errno!=EINTR)
		            {
		                TEST_ERROR;
		            }
		        }
		    }else
		    {
		        TEST_ERROR;
		    }

			for(i=0; i<freeDocks; i++)
			{
				increaseSem(sops,shared_portCoords[idx].semID,DOCK); TEST_ERROR;
			}

			break;
			

		case SIGALRM:
			pastDays++;
			updateGoods(p,shared_portCoords[idx].semID);
			break;

		case SIGINT:
			cleanUp();
			exit(EXIT_SUCCESS);
			break;
	}
	errno=prevErrno;
}

int main(int argc, char *argv[]) {
	char *string;
	sigset_t set;
	int i, numBytes;
	int portsSharedMemoryID, sum_requestID, *sum_request, msg_id;
	coordinates coords;
	struct sembuf sops;
	struct sigaction sa;
	struct msg_request msg_request;
	goods *g;

	bzero(&p, sizeof(p));
	bzero(&sa, sizeof(sa));
	bzero(&sops, sizeof(sops));

	sa.sa_handler = handleSignal;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	sem_sync_id = atoi(argv[1]);
	portsSharedMemoryID=atoi(argv[2]);
	sum_requestID = atoi(argv[3]);
	sem_sum_id = atoi(argv[4]);
	msg_id = atoi(argv[5]);
	idx = atoi(argv[6]);
	sum_offerID = atoi(argv[7]);

	shared_portCoords = shmat(portsSharedMemoryID, NULL, 0);
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

	shared_portCoords[idx].coords=coords;
	shared_portCoords[idx].pid=getpid();
	shared_portCoords[idx].offersID=shmget(IPC_PRIVATE, SO_FILL*sizeof(goods), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	p.generatedGoods=shmat(shared_portCoords[idx].offersID, NULL, 0);
	shmctl(shared_portCoords[idx].offersID, IPC_RMID, NULL); TEST_ERROR;
	shared_portCoords[idx].requestID = shmget(IPC_PRIVATE, sizeof(struct request), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	p.request = shmat(shared_portCoords[idx].requestID, NULL, 0); TEST_ERROR;
	shmctl(shared_portCoords[idx].requestID, IPC_RMID, NULL); TEST_ERROR;

	srand(getpid());
	p.docks = rand() % SO_BANCHINE + 1;
	shared_portCoords[idx].docks=p.docks;
	shared_portCoords[idx].swell=0;
	shared_portCoords[idx].semID = semget(IPC_PRIVATE, 4, IPC_CREAT | 0600); /*3 semaphores: sem[0]=docks, sem[1]= offers handling, sem[2]=request handling, sem[3]=swell*/ TEST_ERROR;
	semctl(shared_portCoords[idx].semID, 0, SETVAL, p.docks); TEST_ERROR;
	semctl(shared_portCoords[idx].semID, 1, SETVAL, 1); TEST_ERROR;
	semctl(shared_portCoords[idx].semID, 2, SETVAL, 1); TEST_ERROR;
	semctl(shared_portCoords[idx].semID, 3, SETVAL, 1); TEST_ERROR;
	p.coords = coords;

	printPort(p, idx);
	bzero(p.generatedGoods,SO_FILL*sizeof(goods));

	generateRequest(p, sum_requestID, sem_sum_id);

	idxOffer=generateOffer(p, 0, sum_offerID,  shared_portCoords[idx].semID);

	msg_request.mtype = p.request -> goodsType;
	msg_request.idx = idx;

	msgsnd(msg_id, &msg_request, sizeof(struct msg_request), 0); TEST_ERROR;


	decreaseSem(sops, sem_sync_id, 0);


	waitForZero(sops, sem_sync_id, 0);

	while (1) {
		pause();
		if (errno == 4) errno = 0;
		else TEST_ERROR;
	}

	exit(0);
}