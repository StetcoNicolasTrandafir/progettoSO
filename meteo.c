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

#include "macro.h"
#include "utility_coordinates.h"
#include "utility_goods.h"
#include "utility_port.h"
#include "utility_meteo.h"


void handleSignal(int signal) {
	struct timespec now;
	pid_t randomPort, randomShip;
    
	switch(signal) {
        
		case SIGUSR1:
			clock_gettime(CLOCK_REALTIME, &now);
    		randomPort = now.tv_nsec % SO_PORTI;

			printf("\n\n[%d]METEO: sto per colpire il porto con indice %d con una maregiata e la nave %d con una tempesta", getpid(),randomPort,-1);
			break;

        case SIGALRM:

			printf("\n\n[%d]METEO: vortice! la nave %d verrà affondata", getpid(),-1);
			break;
	}
}


int main(int argc, char *argv[]){
    int i;
	struct sigaction sa;
	struct timespec mealstromQuantum;
	


	mealstromQuantum=getMealstromQuantum();

    bzero(&sa, sizeof(sa));
	
	sa.sa_handler = handleSignal;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);

   
    sleep(20);
}