#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>


#include "macro.h"
#include "types_module.h"
#include "utility_port.h"
#include "utility_ship.h"

#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s: a riga %d PID=%d: Error %d: %s\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

int min(int a, int b){ return (a>b) ? b:a; }

void move(coordinates from, coordinates to, struct timespec rem){
    double travelTime=getDistance(from, to)/SO_SPEED;
    int intero= (int)travelTime;
    double decimal=travelTime-intero;
    struct timespec sleepTime;

    sleepTime.tv_nsec=decimal;
    sleepTime.tv_sec=intero;

    nanosleep(&sleepTime, &rem);
}


/*NON È PIÙ NECESSARIO E IO GODO COME UN RICCIO*/
/*coordinates getPosition(ship ship, struct timespec rem){
    if(rem){
        
        nanosleep(rem, rem);
    }else{
        return ship.coords;
    }
}*/


int getNearestPort(struct port_sharedMemory *ports, coordinates coords, double min){
    int i;
    int minIndex=-1;
    double minDist=2*SO_LATO;
    double tempDistance;

    for(i=0; i<SO_PORTI; i++){
        if((tempDistance=getDistance(coords, ports[i].coords))< minDist && tempDistance>min){
            minIndex=i;
            minDist= tempDistance;
        }
    }
    return minIndex;
}

void loadUnload(int quantity, struct timespec rem){
    double neededTime= quantity/SO_LOADSPEED;
    struct timespec sleepTime;

    sleepTime.tv_sec=(int)neededTime;
    sleepTime.tv_nsec=neededTime-((int)neededTime);

    nanosleep(&sleepTime, &rem);
}
