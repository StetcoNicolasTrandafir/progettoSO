#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "macro.h"
#include "utility_coordinates.h"
#include "utility_goods.h"
#include "utility_port.h"
#include "utility_ship.h"

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
int getNearestPort(struct port_sharedMemory * ports, coordinates coords, double min){
    int i;
    int minIndex = -1;
    double minDist = min;
    double tempDistance;

    for(i=0; i<SO_PORTI; i++){
        if((tempDistance = getDistance(coords, ports[i].coords)) < minDist) {
            minIndex = i;
            minDist= tempDistance;
        }
    }
    return minIndex;
}

void loadUnload(goods goods, struct timespec rem){
    double neededTime= goods.dimension/SO_LOADSPEED;
    int intero= (int)neededTime;
    double decimal=neededTime-intero;
    struct timespec sleepTime;

    sleepTime.tv_nsec=decimal;
    sleepTime.tv_sec=intero;

    nanosleep(&sleepTime, &rem);
}
