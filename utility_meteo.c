
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
       
       

#include "macro.h"
#include "utility_coordinates.h"
#include "utility_goods.h"
#include "utility_port.h"
#include "utility_ship.h"
#include "utility_meteo.h"



#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s: a riga %d PID=%5d: Error %d: %s\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}


void mealstrom(struct ship_sharedMemory * ships){
    int shipIndex;
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    shipIndex=now.tv_nsec%SO_NAVI;

    kill(SIGINT, ships[shipIndex].pid);
}


void storm(struct ship_sharedMemory * ships){
    int shipIndex;
    struct timespec t;
    pid_t *shipPids;

    shipPids=getShipsInMovement(ships);

    clock_gettime(CLOCK_REALTIME, &t);
    shipIndex=t.tv_nsec%(sizeof(shipPids)/sizeof(pid_t));


    /*REVIEW : lo gestiamo con SIGSTOP e SIGCONT? 
    effetti dei segnali sulle sleep/nanosleep?*/
    kill(SIGSTOP, shipPids[shipIndex]);

    if(SO_STORM_DURATION%24==0){
        t.tv_nsec=0;
        t.tv_sec=SO_STORM_DURATION/24;
    }else if(SO_STORM_DURATION<24){
        t.tv_sec=0;
        t.tv_nsec=(1000000000/24)*SO_STORM_DURATION;
    }else{
        t.tv_sec=SO_STORM_DURATION/24;
        t.tv_nsec=(1000000000/24)*(SO_STORM_DURATION-24*t.tv_sec);
    }

    nanosleep(&t, NULL);

    kill(SIGCONT, shipPids[shipIndex]);
}


void swell(struct port_sharedMemory *ports, struct ship_sharedMemory *ships){
    int portIndex;
    int i;
    struct timespec t;
    pid_t *shipPids;
    int portSemaphoreID;
    int freeDocks;
    struct sembuf sops;
    

    bzero(&sops, sizeof(sops));

    clock_gettime(CLOCK_REALTIME, &t);
    portIndex=t.tv_nsec%SO_PORTI;

    shipPids=getShipsInPort(ships,ports[portIndex].coords);

    portSemaphoreID= semget(ports[portIndex].pid,3,0600);
    freeDocks= semctl(portSemaphoreID, 0, GETVAL);
    if(freeDocks)
    {
        sops.sem_num = 0;
	    sops.sem_op = -freeDocks;
	    semop(portSemaphoreID, &sops, 1); TEST_ERROR;
    }


    /*REVIEW : lo gestiamo con SIGSTOP e SIGCONT? 
    effetti dei segnali sulle sleep/nanosleep?*/
    for(i=0; i< sizeof(shipPids)/sizeof(pid_t); i++){
        kill(SIGSTOP, shipPids[i]);
    }

    if(SO_SWELL_DURATION%24==0){
        t.tv_nsec=0;
        t.tv_sec=SO_SWELL_DURATION/24;
    }else if(SO_SWELL_DURATION<24){
        t.tv_sec=0;
        t.tv_nsec=(1000000000/24)*SO_SWELL_DURATION;
    }else{
        t.tv_sec=SO_SWELL_DURATION/24;
        t.tv_nsec=(1000000000/24)*(SO_SWELL_DURATION-24*t.tv_sec);
    }

    nanosleep(&t, NULL);

    for(i=0; i< sizeof(shipPids)/sizeof(pid_t); i++){
        kill(SIGCONT, shipPids[i]);
    }

    freeDocks= semctl(portSemaphoreID, 0, GETVAL);
    if(freeDocks)
    {
        sops.sem_num = 0;
	    sops.sem_op = freeDocks;
	    semop(portSemaphoreID, &sops, 1); TEST_ERROR;
    }
}



struct timespec getMealstromQuantum(){
    struct timespec t;
    t.tv_sec= (int)(1/24)*SO_MEALSTROM;
    t.tv_nsec=((int)((1/24)*SO_MEALSTROM)*100000000)%100000000;

    return t;
}

