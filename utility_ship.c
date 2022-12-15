#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include "macro.h"
#include "utility_coordinates.h"
#include "utility_goods.h"
#include "utility_port.h"
#include "utility_ship.h"

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


int getNearestPort(struct port_sharedMemory * ports, coordinates coords, double min){
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

void loadUnload(goods goods, struct timespec rem){
    double neededTime= goods.dimension/SO_LOADSPEED;
    int intero= (int)neededTime;
    double decimal=neededTime-intero;
    struct timespec sleepTime;

    sleepTime.tv_nsec=decimal;
    sleepTime.tv_sec=intero;

    nanosleep(&sleepTime, &rem);
}

int getValidRequestPort(goods good, int msg_id, int shm_id) {
    struct msg_request msg;
    int ret = 0, first_idx = -1, q;
    struct request *requests;
    requests = shmat(shm_id, NULL, 0);
    while (1) {
        ret = msgrcv(msg_id, &msg, sizeof(struct msg_request), good.type, IPC_NOWAIT);
        if (ret == -1)
            return -1;
        else {
            /*LOCK*/
            q = min(min(SO_CAPACITY, requests[msg.idx].quantity - requests[msg.idx].booked), good.dimension);
            if (requests[msg.idx].booked < requests[msg.idx].quantity) {
                requests[msg.idx].booked += q;
                /*UNLOCK*/
                good.state = delivered;
                msgsnd(msg_id, &msg, sizeof(struct msg_request), 0);
                shmdt(requests);
                return msg.idx;
            }
            if (first_idx == -1) {
                first_idx == msg.idx;
                msgsnd(msg_id, &msg, sizeof(struct msg_request), 0);
            }
            else if (msg.idx == first_idx) {
                msgsnd(msg_id, &msg, sizeof(struct msg_request), 0);
                shmdt(requests);
                return -1;
            }
        }
    }
}