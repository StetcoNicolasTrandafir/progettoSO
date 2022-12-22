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



pid_t * getShipsInMovement(struct ship_sharedMemory * ships){
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

pid_t * getShipsInPort(struct ship_sharedMemory *ships, coordinates portCoords){
    int i, j;
    int count=0;
    pid_t *pids;

    for(i=0; i<SO_NAVI; i++)
        if(ships[i].coords.x==portCoords.x&&ships[i].coords.y==portCoords.x)
            count++;

    pids= calloc(count, sizeof(pid_t));
    i=0;

    while(i<SO_NAVI && j< count){
        if(ships[i].coords.x==portCoords.x&&ships[i].coords.y==portCoords.x)
            pids[j++]=ships[i].pid;
        i++;
    }
        

    return pids;
}




/*
NEGOZIAZIONE NAVI-PORTI:
1) verifico i porti in ordine di vicinanza rispetto alla nave
2) controllo le richieste del porto
3) per ogni offerta, controllo se c'è un porto (sempre in ordine di prossimità) che fa richiesta di quel bene e se ci arrivo (per la scadenza)
4) se c'è una richiesta, segno come impegnate tot merci sia nella richiesta che nell'offerta
5) se non c'è nessuna richiesta, considero il prossimo porto più vicino
6) se non c'è nessun porto che richiede una determinata merce di un offerta, considero la prossima offerta del porto
7) se non ci sono richieste per nessuna offerta del porto preso in considerazione, prendo in considerazione il seguente porto
8) se non ci sono offerte e richieste che la nave può soddisfare in nessun porto, mi muovo verso il porto più vicino aspettando la generazione del giorno dopo
*/

int negociate(struct port_sharedMemory *ports, ship s){

    int indexClosestPort= getNearestPort(ports, s.coords, -1);
    goods *g= shmat(ports[indexClosestPort].offersID, NULL, 0);
    int i=0, j=0;
    int destinationPortIndex=-1;
    double travelTime;
    struct timespec time, rem; 
    int goodsQuantity;
    struct sembuf semaphores;
    int startingPortSemID, destinationPortSemID;

    while(j++<SO_NAVI && destinationPortIndex==-1){

        indexClosestPort= getNearestPort(ports, s.coords, getDistance(s.coords, ports[indexClosestPort].coords));
        g= shmat(ports[indexClosestPort].offersID, NULL, 0);

        while(g[i].type!=-1 && destinationPortIndex==-1 && i<SO_DAYS ){
            destinationPortIndex=5; /*funzioneDiSte(g[i++]);*/
        }
    }

    startingPortSemID=semget(ports[indexClosestPort].pid, 3, 0600);
    destinationPortSemID=semget(ports[destinationPortIndex].pid, 3, 0600);

    /*moving towards the port to load goods*/
    travelTime= getTravelTime(getDistance(s.coords,ports[indexClosestPort].coords));
    s.coords.x=-1;
    s.coords.y=-1;
    time.tv_sec=(int)travelTime;
    time.tv_nsec=travelTime-time.tv_sec;
    nanosleep(&time, &rem);

    /*arrived at the port, loading the goods*/
    s.coords=ports[indexClosestPort].coords;
    

    semaphores.sem_num=0;
    semaphores.sem_op=-1;
    semaphores.sem_flg=0;
    semop(startingPortSemID, &semaphores, 1);

    loadUnload(goodsQuantity, rem);

    semaphores.sem_num=0;
    semaphores.sem_op=1;
    semaphores.sem_flg=0;
    semop(startingPortSemID, &semaphores, 1);

    /*moving towards the port wich made the request*/
    travelTime= getTravelTime(getDistance(s.coords,ports[destinationPortIndex].coords));
    s.coords.x=-1;
    s.coords.y=-1;
    time.tv_sec=(int)travelTime;
    time.tv_nsec=travelTime-time.tv_sec;
    nanosleep(&time, &rem);


    /*arrived at the port, loading the goods*/
    s.coords=ports[destinationPortIndex].coords;

    semaphores.sem_num=0;
    semaphores.sem_op=-1;
    semaphores.sem_flg=0;
    semop(destinationPortSemID, &semaphores, 1);

    loadUnload(goodsQuantity, rem);

    semaphores.sem_num=0;
    semaphores.sem_op=1;
    semaphores.sem_flg=0;
    semop(destinationPortSemID, &semaphores, 1);

    return destinationPortIndex;
}

int getValidRequestPort(goods good, int msg_id, int shm_id) {
    struct msg_request msg;
    int ret = 0, first_idx = -1, q, request_id, sem_id;
    struct request *request;
    struct port_sharedMemory *sh_port;
    struct sembuf sops;

    bzero(&sops, sizeof(sops));
    
    sh_port = shmat(shm_id, NULL, 0);
    while (1) {
        ret = msgrcv(msg_id, &msg, sizeof(struct msg_request), good.type, IPC_NOWAIT);
        request_id = shmget(sh_port[msg.idx].pid, sizeof(struct request), 0600); TEST_ERROR;
        sem_id = semget(sh_port[msg.idx].pid, 3, 0600);
        shmdt(sh_port);
        request = shmat(request_id, NULL, 0); TEST_ERROR;
        if (ret == -1)
            return -1;
        else {
            sops.sem_num = 1;
            sops.sem_op = -1;
            semop(sem_id, &sops, 1);
            q = min(min(SO_CAPACITY, request -> quantity - request -> booked), good.dimension);
            if (request -> booked < request -> quantity) {
                request -> booked += q;
                sops.sem_num = 1;
                sops.sem_op = 1;
                semop(sem_id, &sops, 1);
                good.state = delivered;
                msgsnd(msg_id, &msg, sizeof(struct msg_request), 0);
                shmdt(request);
                return msg.idx;
            }
            if (first_idx == -1) {
                first_idx == msg.idx;
                msgsnd(msg_id, &msg, sizeof(struct msg_request), 0);
            }
            else if (msg.idx == first_idx) {
                msgsnd(msg_id, &msg, sizeof(struct msg_request), 0);
                shmdt(request);
                return -1;
            }
        }
    }
}