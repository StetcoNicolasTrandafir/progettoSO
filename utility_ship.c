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
#include "semaphore_library.h"
#include "types_module.h"
#include "utility_coordinates.h"
#include "utility_port.h"


int min(int a, int b){ return (a>b) ? b:a; }

void move(coordinates from, coordinates to){
    double travelTime=getDistance(from, to)/SO_SPEED;
    int intero= (int)travelTime;
    double decimal=travelTime-intero;
    struct timespec sleepTime, rem;

    sleepTime.tv_nsec=decimal;
    sleepTime.tv_sec=intero;

    nanosleep(&sleepTime, &rem);
    if(errno==EINTR){

        while(nanosleep(&rem, &rem)==-1)
        {
            if(errno!=EINTR)
            {
                TEST_ERROR;
            }else{
                errno=0;
            }
        }
    }else
    {
        TEST_ERROR;
    }
}


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

void loadUnload(int quantity){
    double neededTime= quantity/SO_LOADSPEED;
    struct timespec sleepTime, rem;

    sleepTime.tv_sec=(int)neededTime;
    sleepTime.tv_nsec=neededTime-((int)neededTime);

    printf("\nQUANTITY: %d NEEDED TIME: %f SECONDI: %ld NANOSECONDI: %ld",quantity, neededTime,sleepTime.tv_sec,sleepTime.tv_nsec);

    nanosleep(&sleepTime, &rem);
    if(errno==EINTR)
    {
        errno=0;
        while(nanosleep(&rem, &rem)==-1){
            if(errno!=EINTR)
            {
                TEST_ERROR;
            }else{
                errno=0;
            }
        }
    }else{
        TEST_ERROR;
    }
    
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



int negociate(struct port_sharedMemory *ports, ship s, struct ship_sharedMemory shared_ship){

    int indexClosestPort= getNearestPort(ports, s.coords, -1);
    goods *g= shmat(ports[indexClosestPort].offersID, NULL, 0);
    int i=0, j=0;
    int destinationPortIndex=-1;
    double travelTime;
    struct timespec time, rem; 
    int goodsQuantity;
    int startingPortSemID, destinationPortSemID;
    int shippedQuantity=0;
    int goodIndex=-1;
    struct request *request;
    struct sembuf sops;
    char *string;
	int numBytes;

    bzero(&sops, sizeof(struct sembuf));

    while(j++<SO_NAVI && destinationPortIndex==-1 && indexClosestPort!=-1){
        indexClosestPort= getNearestPort(ports, s.coords, getDistance(s.coords, ports[indexClosestPort].coords));
        g= shmat(ports[indexClosestPort].offersID, NULL, 0);TEST_ERROR;

        while(g[i].type!=-1 && destinationPortIndex==-1 && i<SO_DAYS ){
            if(g[i].state==in_port){
                destinationPortIndex=getValidRequestPort(g[i],ports);
                if(destinationPortIndex!=-1)
                    goodIndex=i;
            }
            
            i++;
        }
        if(destinationPortIndex==-1){
            shmdt(g);TEST_ERROR;
        }
            
    }

    if(goodIndex!=-1){
        printf("\n\n %d || %d \n\n", indexClosestPort, destinationPortIndex);
        startingPortSemID = ports[indexClosestPort].semID;
        destinationPortSemID = ports[destinationPortIndex].semID;
        /*startingPortSemID=semget(ports[indexClosestPort].semID, 3, 0600);TEST_ERROR;
        destinationPortSemID=semget(ports[destinationPortIndex].semID, 3, 0600); TEST_ERROR; /*[0]=banchine [1]=offerta [2]=richiesta*/


        printf("\n\nSTAMPIAMO STO BENE\n\nTIPO %d QUANTITÀ %d", g[goodIndex].type,g[goodIndex].dimension);
        /*TODO AGGIORNO I BOOKED DELLA RICHIESTA E DELL'OFFERTA*/

        /*CAMBIO VALORI RICHIESTA*/
        TEST_ERROR;
        decreaseSem(sops, destinationPortSemID, REQUEST);
        TEST_ERROR;
        



        request = shmat(ports[destinationPortIndex].requestID, NULL, 0); TEST_ERROR;
        shippedQuantity=min((request->quantity - request->booked), (g[goodIndex].dimension - g[goodIndex].booked));
        request->booked+=shippedQuantity;

        TEST_ERROR;
        increaseSem(sops, destinationPortSemID, REQUEST);
        TEST_ERROR;
        /*UNLOCK(destinationPortIndex, 2)*/

        /*CAMBIO VALORI OFFERTA*/
        /*LOCK(destinationPortIndex, 2)*/
        TEST_ERROR;
        decreaseSem(sops, startingPortSemID, OFFER);
        TEST_ERROR;

        g[goodIndex].booked+=shippedQuantity;


        TEST_ERROR;
        increaseSem(sops, startingPortSemID, OFFER);
        TEST_ERROR;


        /*moving towards the port to load goods*/
        s.coords.x=-1;
        s.coords.y=-1;
        shared_ship.coords=s.coords;

        move(s.coords,ports[indexClosestPort].coords);
        
        /*arrived at the port*/
        s.coords=ports[indexClosestPort].coords;
        shared_ship.coords=s.coords;

        /*loading goods*/
        /*LOCK(destinationPortIndex, 2)*/
        TEST_ERROR;
        decreaseSem(sops, startingPortSemID, DOCK);
        TEST_ERROR;

        shared_ship.inDock=1;
        printf("\n\nCARICANDO");
        loadUnload(g[goodIndex].dimension);

        TEST_ERROR;
        increaseSem(sops, startingPortSemID, DOCK);
        TEST_ERROR;

        shared_ship.inDock=0;
        shared_ship.goodsQuantity=shippedQuantity;

        /*CAMBIO VALORI OFFERTA*/
        /*LOCK(destinationPortIndex, 2)*/
        TEST_ERROR;
        decreaseSem(sops, startingPortSemID, OFFER);
        TEST_ERROR;

        g[goodIndex].shipped+=shippedQuantity;
        shmdt(g); TEST_ERROR;
        
        TEST_ERROR;
        increaseSem(sops, startingPortSemID, OFFER);
        TEST_ERROR;
        
        /*moving towards the port wich made the request*/

        s.coords.x=-1;
        s.coords.y=-1;
        shared_ship.coords=s.coords;

        move(s.coords,ports[destinationPortIndex].coords);

        /*arrived at the port, downloading the goods*/
        s.coords=ports[destinationPortIndex].coords;
        shared_ship.coords=s.coords;

        /*LOCK(destinationPortIndex, 2)*/
        TEST_ERROR;
        decreaseSem(sops, destinationPortSemID, DOCK);
        TEST_ERROR;
        shared_ship.inDock=1;

        printf("\n\nSCARICANDO");
        loadUnload(g[goodIndex].dimension);

        shared_ship.goodsQuantity=0;
        shared_ship.inDock=0;

        TEST_ERROR;
        increaseSem(sops, destinationPortSemID, DOCK);
        TEST_ERROR;

        /*CAMBIO VALORI RICHIESTA*/
        /*LOCK(destinationPortIndex, 2)*/
        TEST_ERROR;
        decreaseSem(sops, destinationPortSemID, REQUEST);
        TEST_ERROR;
        request->satisfied+=shippedQuantity;
        shmdt(request);


        TEST_ERROR;
        increaseSem(sops, destinationPortSemID, REQUEST); 
        TEST_ERROR;

    
        string=malloc(70);
        numBytes=sprintf(string,"\n[%d]FINITO DI SCARICARE! Merce portata dal punto A al punto B!\n\n", getpid());

        fflush(stdout);
        write(1, string, numBytes);
        
        
        free(string);

        return destinationPortIndex;
    }else{
        printf("\n\nNessun posto dove andare feeeeega");

        return -1;
    }
}


int getValidRequestPort(goods good, struct port_sharedMemory * sh_port) {
    struct msg_request msg;
    int ret = 0, first_idx = -1, q, request_id, sem_id, i;
    struct request *request;
    struct sembuf sops;
    int msg_id;

    bzero(&sops, sizeof(sops));

    msg_id=msgget(getppid(), 0600); TEST_ERROR;

    for (i = 0; i < SO_DAYS; i++) {
        ret = msgrcv(msg_id, &msg, sizeof(struct msg_request), good.type, IPC_NOWAIT); 

        if(errno!=42){
            TEST_ERROR;
        }else 
            errno=0;

        if (ret == -1){
            return -1;
        }   
        sem_id = sh_port[msg.idx].semID;
        request = shmat(sh_port[msg.idx].requestID, NULL, 0); TEST_ERROR;

        decreaseSem(sops, sem_id,1);

        q = min(min(SO_CAPACITY, request -> quantity - request -> booked), good.dimension);
        if (request -> booked < request -> quantity) {
            request -> booked += q;
            TEST_ERROR;
            increaseSem(sops,sem_id,1 );

            msgsnd(msg_id, &msg, sizeof(struct msg_request), 0);
            shmdt(request);
            return msg.idx;
        }else{
            TEST_ERROR;
            increaseSem(sops,sem_id,1 );
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