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

struct ship_sharedMemory *shared_shipCoords;
ship s;
int pastDays = 0, sem_sync_id, shipIndex;
struct timespec stormDuration, swellDuration;
int *expiredGoods;

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
	if (shared_shipCoords[shipIndex].sinked != 1){
		waitForZero(sops, sem_sync_id, 3); TEST_ERROR;
	}
	else {
		if(s.semLastID != -1 && s.semLastNum != -1) {
			printTest(53);
			increaseSem(sops, s.semLastID, s.semLastNum); TEST_ERROR;
		}
	}

	shmdt(expiredGoods); TEST_ERROR;
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
	int prevErrno=errno;

	errno=0;
	
	switch(signal) {
		case SIGUSR1:
			badWeather(getSwellDuration());
			break;

		case SIGUSR2:
			badWeather(stormDuration);
        	break;

        case SIGALRM:
			pastDays++;
            break;

		case SIGINT:
			cleanUp();
			exit(EXIT_SUCCESS);
			break;
	}
	errno=prevErrno;
}

int main(int argc, char *argv[]) {
	sigset_t set;
	int portsSharedMemoryID, sem_expired_goods_id;
	int i;
	struct sembuf sops;
	struct sigaction sa;

	shared_shipCoords= shmat(atoi(argv[3]), NULL, 0); TEST_ERROR;
	shipIndex= atoi(argv[4]);
	expiredGoods=shmat(atoi(argv[5]), NULL, 0); TEST_ERROR;
	sem_expired_goods_id = atoi(argv[6]); 

	bzero(&sa, sizeof(sa));TEST_ERROR;

	swellDuration=getSwellDuration();
	stormDuration=getStormDuration();

	sa.sa_handler = handleSignal;TEST_ERROR;
	sigaction(SIGUSR1, &sa, NULL);TEST_ERROR;
	sigaction(SIGUSR2, &sa, NULL);TEST_ERROR;
	sigaction(SIGALRM, &sa, NULL);TEST_ERROR;
	sigaction(SIGINT, &sa, NULL);TEST_ERROR;
	
	bzero(&sops, sizeof(sops));
	
	s.coords = getRandomCoords();
	shared_shipCoords[shipIndex].coords = s.coords;
	shared_shipCoords[shipIndex].inDock = 0;
	shared_shipCoords[shipIndex].storm = 0;
	shared_shipCoords[shipIndex].sinked = 0;
	s.semLastID = -1;
	s.semLastNum = -1;

	shared_shipCoords[shipIndex].goodsID = shmget(IPC_PRIVATE, SO_CAPACITY * sizeof(goods), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	s.goods = shmat(shared_shipCoords[shipIndex].goodsID, NULL, 0); TEST_ERROR;
	bzero(s.goods, SO_CAPACITY * sizeof(goods));


	shared_shipCoords[shipIndex].semID = semget(IPC_PRIVATE, 4, IPC_CREAT | 0600); TEST_ERROR;
	semctl(shared_shipCoords[shipIndex].semID, 0, SETVAL, 1); TEST_ERROR; /*goods*/
	semctl(shared_shipCoords[shipIndex].semID, 1, SETVAL, 1); TEST_ERROR; /*coords*/
	semctl(shared_shipCoords[shipIndex].semID, 2, SETVAL, 1); TEST_ERROR; /*inDock*/
	semctl(shared_shipCoords[shipIndex].semID, 3, SETVAL, 1); TEST_ERROR; /*storm*/

	printShip(s); TEST_ERROR;
	sem_sync_id = atoi(argv[1]);

	decreaseSem(sops, sem_sync_id, 0); TEST_ERROR;

	waitForZero(sops, sem_sync_id, 0); TEST_ERROR;
	
	while (pastDays < SO_DAYS) {
		if(negociate(atoi(argv[2]), s, shared_shipCoords,shipIndex,expiredGoods, sem_expired_goods_id)== -1) {
			pause();
		}
	}	

	while(1) {
		pause();
	}

	exit(0);
}