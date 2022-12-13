
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "macro.h"
#include "utility_meteo.h"

void swell(int portPid){
    kill(SIGINT, portPid);
}

struct timespec getMealstromQuantum(){
    struct timespec t;
    t.tv_sec= (int)(1/24)*SO_MEALSTROM;
    t.tv_nsec=((int)((1/24)*SO_MEALSTROM)*100000000)%100000000;

    return t;
}


pid_t [] getShipsInMovement(struct shared_ship ships){
    int i, j;
    int count=0;
    pid_t *pids;
    for(i=0; i<SO_NAVI; i++)
        if(ships[i].coords.x==-1&&ships[i].coords.y==-1)
            count++;
    pids= calloc(count, sizeof(pid_t));
    count=0;
    for(i=0; i<SO_NAVI; i++)
        if(ships[i].coords.x==-1&&ships[i].coords.y==-1)
            pids[j]=ships[i].pid;

    return pids;
}