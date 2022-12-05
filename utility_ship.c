#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.jh>

#include "macro.h"
#include "utility_coordinates.h"
#include "utility_port.h"

void move(coordinates from, coordinates to, struct timespec rem){
    double travelTime=getDistance(from, to)/SO_SPEED;
    int intero= (int)travelTime;
    double decimal=travelTime-intero;
    struct timespec sleepTime;

    sleepTime.tv_nsec=decimal;
    sleepTime.tc_sec=intero;

    nanosleep(sleepTime, rem);
}

coordinates getPosition(ship ship, struct timespec rem){
    if(rem){
        //TODO calcolare la posizione
        nanosleep(rem, rem);
    }else{
        return ship.coords;
    }
}

void loadUnload(goods goods, struct timespec rem){
    double neededTime= goods.dimension/SO_LOADSPEED;
    int intero= (int)neededTime;
    double decimal=neededTime-intero;
    struct timespec sleepTime;

    sleepTime.tv_nsec=decimal;
    sleepTime.tc_sec=intero;

    nanosleep(sleepTime, rem);
}
