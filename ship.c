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
#include <time.h>

#include "macro.h"
#include "semaphore_library.h"
#include "types_module.h"
#include "utility_port.h"
#include "utility_coordinates.h"
#include "utility_ship.h"
#include "utility_meteo.h"

struct port_sharedMemory *shared_portCoords;
struct ship_sharedMemory *shared_shipCoords;
ship s;
int pastDays = 0, sem_sync_id, shipIndex;

void printShip(ship s) {
	char *string;
	int numBytes;
	string=malloc(30);
	numBytes=sprintf(string,"Ship[%d]: (%.2f, %.2f)\n", getpid(), s.coords.x, s.coords.y);

	fflush(stdout);
	write(1, string, numBytes);
	free(string);
}

void cleanUp() {
	struct sembuf sops;
	bzero(&sops, sizeof(struct sembuf));
	if (shared_shipCoords[shipIndex].pid != -1){
		waitForZero(sops, sem_sync_id, 3); TEST_ERROR;
	}
	semctl(shared_shipCoords[shipIndex].semID, 0, IPC_RMID); TEST_ERROR;
	shmdt(s.goods); TEST_ERROR;
	shmdt(shared_shipCoords); TEST_ERROR;
	
	decreaseSem(sops, sem_sync_id, 2); TEST_ERROR;
}
void printTime(){
    int numBytes;
    char *string;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    
    string=malloc(70);
    numBytes=sprintf(string,"\n[%d]GIORNO: %ld MILLISECONDI: %ld\n", getpid(), now.tv_sec, now.tv_nsec);

    fflush(stdout);
    write(1, string, numBytes);
    free(string);
}


void handleSignal(int signal) {
	int index, portsSharedMemoryID;
	int i;
	char *string;
	int numBytes;
	switch(signal) {
		case SIGUSR1:
			badWeather(getSwellDuration());
			break;

		case SIGUSR2:
			printTime();
			badWeather(getStormDuration());
			printTime();
        	break;

        case SIGALRM:
			pastDays++;
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
			cleanUp();
			exit(EXIT_SUCCESS);
			break;
	}
}

int main(int argc, char *argv[]) {
	sigset_t set;
	int portsSharedMemoryID;
	int i, msg_id, *ptr_set;
	struct sembuf sops;
	struct msg_request msg_request;
	struct sigaction sa;

	/*shared_portCoords = shmat(atoi(argv[2]), NULL, 0); TEST_ERROR;*/
	shared_shipCoords= shmat(atoi(argv[3]), NULL, 0); TEST_ERROR;
	shipIndex= atoi(argv[4]);

	/*
	printf("\n");
	for(i=0; i<SO_PORTI; i++){
		printf("\n[%d] (%f, %f)", shared_portCoords[i].pid, shared_portCoords[i].coords.x,shared_portCoords[i].coords.y);
	}
	printf("\n");
	*/
	bzero(&sa, sizeof(sa));TEST_ERROR;

	sa.sa_handler = handleSignal;TEST_ERROR;
	sigaction(SIGUSR1, &sa, NULL);TEST_ERROR;
	sigaction(SIGUSR2, &sa, NULL);TEST_ERROR;
	sigaction(SIGALRM, &sa, NULL);TEST_ERROR;
	sigaction(SIGCONT, &sa, NULL);TEST_ERROR;
	sigaction(SIGINT, &sa, NULL);TEST_ERROR;
	
	bzero(&sops, sizeof(sops));
	
	s.coords = getRandomCoords();
	shared_shipCoords[shipIndex].coords = s.coords;
	shared_shipCoords[shipIndex].inDock= 0;
	shared_shipCoords[shipIndex].goodsQuantity= 0;

	shared_shipCoords[shipIndex].goodsID = shmget(IPC_PRIVATE, SO_CAPACITY * sizeof(goods), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	s.goods = shmat(shared_shipCoords[shipIndex].goodsID, NULL, 0); TEST_ERROR;
	shmctl(shared_shipCoords[shipIndex].goodsID, IPC_RMID, NULL); TEST_ERROR;
	bzero(s.goods, SO_CAPACITY * sizeof(goods));

	shared_shipCoords[shipIndex].semID = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600); TEST_ERROR;
	semctl(shared_shipCoords[shipIndex].semID, 0, SETVAL, 1); TEST_ERROR;

	printShip(s); TEST_ERROR;
	sem_sync_id = atoi(argv[1]);

	decreaseSem(sops, sem_sync_id, 0); TEST_ERROR;

	waitForZero(sops, sem_sync_id, 0); TEST_ERROR;

	msg_id = msgget(getppid(), IPC_CREAT | 0600); TEST_ERROR;
	
	while (pastDays < SO_DAYS) {
		if(negociate(atoi(argv[2]), s, shared_shipCoords,shipIndex)== -1) {
			pause();
			if (errno == 4) errno = 0;
			else TEST_ERROR;
		}
	}	
	pause();
	
	cleanUp();

	exit(0);
}