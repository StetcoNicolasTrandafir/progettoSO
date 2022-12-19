
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "macro.h"
#include "utility_coordinates.h"
#include "utility_goods.h"
#include "utility_ship.h"
#include "utility_port.h"
#include "utility_meteo.h"


void mealstrom(struct ship_sharedMemory * ships){
    int shipIndex;
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    shipIndex=now.tv_nsec%SO_NAVI;

    kill(SIGINT, ships[shipIndex].pid);
}


void storm(struct ship_sharedMemory * ships){
    int shipIndex;
    struct timespec now;
    pid_t *shipPids;

    shipPids=getShipsInMovement(ships);

    clock_gettime(CLOCK_REALTIME, &now);
    shipIndex=now.tv_nsec%(sizeof(shipPids)/sizeof(pid_t));


    /*REVIEW : lo gestiamo con SIGSTOP e SIGCONT? 
    effetti dei segnali sulle sleep/nanosleep?*/
    kill(SIGSTOP, shipPids[shipIndex]);


    /*TODO qua? facciamo una nanosleep?*/

    kill(SIGCONT, shipPids[shipIndex]);
}


void swell(struct port_sharedMemory *ports, struct ship_sharedMemory *ships){
    int portIndex;
    int i;
    struct timespec now;
    pid_t *shipPids;

    clock_gettime(CLOCK_REALTIME, &now);
    portIndex=now.tv_nsec%SO_PORTI;

    shipPids=getShipsInPort(ships,ports[portIndex].coords);

    /*REVIEW : lo gestiamo con SIGSTOP e SIGCONT? 
    effetti dei segnali sulle sleep/nanosleep?*/

    for(i=0; i< sizeof(shipPids)/sizeof(pid_t); i++){
        kill(SIGSTOP, shipPids[i]);
    }

    /*TODO qua? facciamo una nanosleep?*/

    for(i=0; i< sizeof(shipPids)/sizeof(pid_t); i++){
        kill(SIGCONT, shipPids[i]);
    }
}



struct timespec getMealstromQuantum(){
    struct timespec t;
    t.tv_sec= (int)(1/24)*SO_MEALSTROM;
    t.tv_nsec=((int)((1/24)*SO_MEALSTROM)*100000000)%100000000;

    return t;
}

