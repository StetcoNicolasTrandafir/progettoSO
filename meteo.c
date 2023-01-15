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
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/time.h>

#include "macro.h"
#include "types_module.h"
#include "utility_coordinates.h"
#include "semaphore_library.h"
#include "utility_meteo.h"


struct ship_sharedMemory *ships;
struct port_sharedMemory *ports;
int sem_sync_id, pastDays = 0;
struct itimerval mealstromQuantum;

void cleanUp() {
	struct sembuf sops;
	bzero(&sops, sizeof(struct sembuf));
	shmdt(ports); TEST_ERROR;
	shmdt(ships); TEST_ERROR;
	decreaseSem(sops, sem_sync_id, 3); TEST_ERROR;
}

void handleSignal(int signal) {
	struct timespec now;
	pid_t randomPort, randomShip;
	int plus=0;
	int i=0;
	int endWhile=0, flag = 0;
	struct sembuf sops;
	int prevErrno=errno;
	errno=0;

    
	switch(signal) {

		case SIGINT:
			printTest(53);
			cleanUp();
			exit(EXIT_SUCCESS);
			break;

		case SIGUSR2:
			bzero(&sops, sizeof(struct sembuf));

			pastDays++;
			if (pastDays < SO_DAYS) {
				clock_gettime(CLOCK_REALTIME, &now);
	    		randomShip = now.tv_nsec % SO_NAVI;
				

				while(!endWhile && plus < SO_NAVI) {
					TEST_ERROR;
					if(ships[(randomShip+plus)%SO_NAVI].pid!=-1){

						decreaseSem(sops,ships[i].semID, INDOCK); TEST_ERROR;
						if( ships[(randomShip+plus)%SO_NAVI].inDock==0)
							endWhile++;
						else
							plus++;
						increaseSem(sops,ships[i].semID, INDOCK); TEST_ERROR;

					}else{
						plus++;
					}
				}

				if(plus!=SO_NAVI){
					/*printf("\n\nPID: %d (%.2lf,%.2lf)",ships[(randomShip + plus)%SO_NAVI].pid,ships[(randomShip + plus)%SO_NAVI].coords.x,ships[(randomShip + plus)%SO_NAVI].coords.y);*/
					kill(ships[(randomShip+plus)%SO_NAVI].pid, SIGUSR2); TEST_ERROR;
					decreaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, STORM); TEST_ERROR;
					ships[(randomShip+plus)%SO_NAVI].storm++;
					increaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, STORM); TEST_ERROR;
				}

				randomPort=now.tv_nsec%SO_PORTI;
				/*TODO - SEMAFORO BANCHINE PORTO*/

				kill(ports[randomPort].pid, SIGUSR2); TEST_ERROR;
				decreaseSem(sops, ports[randomPort].semID, SWELL); TEST_ERROR;
				ports[randomPort].swell++;
				increaseSem(sops, ports[randomPort].semID, SWELL); TEST_ERROR;

				for(i=0; i< SO_NAVI;i++){
					decreaseSem(sops,ships[i].semID, COORDS); TEST_ERROR;
					if(ships[i].coords.x==ports[randomPort].coords.x && ships[i].coords.y==ports[randomPort].coords.y){
						
						decreaseSem(sops,ships[i].semID, PID); TEST_ERROR;

						if(ships[i].pid!=-1)
						{
							kill(ships[i].pid, SIGUSR1); TEST_ERROR;
						}
						increaseSem(sops,ships[i].semID, PID); TEST_ERROR;
					}
					increaseSem(sops,ships[i].semID, COORDS); TEST_ERROR;
				}
			}
			break;

        case SIGALRM:
			if (pastDays < SO_DAYS) {
        		clock_gettime(CLOCK_REALTIME, &now);
		    	randomShip = now.tv_nsec % SO_NAVI;
		    	while(!endWhile && plus < SO_NAVI)  {
		    		decreaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, PID); TEST_ERROR;
		    		if (ships[(randomShip+plus)%SO_NAVI].pid!=-1) {
		    			endWhile++;
		    		}
		    		else {
		    			plus++;
		    		}
		    		increaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, PID); TEST_ERROR;
		    	}
				if(plus < SO_NAVI){
		    		decreaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, INDOCK); TEST_ERROR;
					if (!ships[(randomShip+plus)%SO_NAVI].inDock) {
		    			increaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, INDOCK); TEST_ERROR;
						for (i = 0; i < SO_PORTI && !flag; i++) {
			    			decreaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, COORDS); TEST_ERROR;
							if (ports[i].coords.x == ships[(randomShip+plus)%SO_NAVI].coords.x && ports[i].coords.y == ships[(randomShip+plus)%SO_NAVI].coords.y) {
			    				increaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, COORDS); TEST_ERROR;
								increaseSem(sops, ports[i].semID, DOCK); TEST_ERROR;
								flag++;
							}
							else {
			    				increaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, COORDS); TEST_ERROR;
							}
						}
					}
					else {
		    			increaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, INDOCK); TEST_ERROR;
					}
					printTest(ships[(randomShip+plus)%SO_NAVI].pid);
		    		decreaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, PID); TEST_ERROR;
		    		printTest(153);
		    		ships[(randomShip+plus)%SO_NAVI].sinked = 1;
					kill(ships[(randomShip+plus)%SO_NAVI].pid, SIGINT); TEST_ERROR;
					/*ships[(randomShip+plus)%SO_NAVI].pid = -1;*/
					printTest(155);
		    		increaseSem(sops, ships[(randomShip+plus)%SO_NAVI].semID, PID); TEST_ERROR;
		    		printTest(157);

					setitimer(ITIMER_REAL, &mealstromQuantum, NULL); TEST_ERROR;
				}
			}
			break;
	}
	errno = prevErrno;
}


int main(int argc, char *argv[]){
    int i;
    int h;	
	struct sigaction sa;
	int numBytes;
	char *string;
	struct sembuf sops;

	sem_sync_id = atoi(argv[1]);
	ports = shmat(atoi(argv[3]), NULL, 0);
	ships = shmat(atoi(argv[2]), NULL, 0);

	mealstromQuantum.it_value.tv_sec = (SO_MEALSTROM/24);
    h = SO_MEALSTROM - (mealstromQuantum.it_value.tv_sec*24);
    mealstromQuantum.it_value.tv_usec=(h*100000)/24;
    mealstromQuantum.it_interval = mealstromQuantum.it_value;

	/*string=realloc(string,120);
	numBytes=sprintf(string,"\n\nUNA NAVE VERRÀ AFFONDATA OGNI %ld,%ld giorni", mealstromQuantum.tv_sec,mealstromQuantum.tv_nsec);
	fflush(stdout);

	write(1, string, numBytes);*/

	bzero(&sops, sizeof(struct sembuf));
    bzero(&sa, sizeof(sa));
	
	sa.sa_handler = handleSignal;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	decreaseSem(sops, sem_sync_id, 0); TEST_ERROR;
	waitForZero(sops, sem_sync_id, 0); TEST_ERROR;

	setitimer(ITIMER_REAL, &mealstromQuantum, NULL); TEST_ERROR;

	while (1) {
		pause();
	}
    exit(EXIT_SUCCESS);

}