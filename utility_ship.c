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
#include "utility_meteo.h"
#include "utility_port.h"


int min(int a, int b){ return (a>b) ? b:a; }

void move(ship s,coordinates from, coordinates to){
    double travelTime=getDistance(from, to)/SO_SPEED;
    int intero= (int)travelTime;
    double decimal=travelTime-intero;
    struct timespec sleepTime, rem;

    sleepTime.tv_nsec=decimal;
    sleepTime.tv_sec=intero;

    s.coords.x=-1;
    s.coords.y=-1;

    nanosleep(&sleepTime, &rem);
    if(errno==EINTR){
        errno=0;
        while(nanosleep(&rem, &rem)==-1)
        {
            if(errno!=EINTR)
            {
                TEST_ERROR;
            }
        }
    }else
    {
        TEST_ERROR;
    }

    s.coords=to;
}

void checkExpiredGoods(ship s, int goodsNumber, int *shippedGoods){
    int i;
    
    for(i=0; i<goodsNumber; i++){
        if(isExpired(s.goods[i]))
        {
            s.goods[i].state=expired_ship;
            shippedGoods[i]=-1;
        }
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

void badWeather(struct timespec duration){
    nanosleep(&duration, &duration);
    if(errno==EINTR){
        errno=0;
        while(nanosleep(&duration, &duration)==-1)
        {
            if(errno!=EINTR)
            {
                TEST_ERROR;
            }
        }
    }else
    {
        TEST_ERROR;
    }

    if(errno==4)
        errno=0;
}


void loadUnload(int quantity){
    float neededTime= quantity/SO_LOADSPEED;
    struct timespec sleepTime, rem;

    sleepTime.tv_sec=(int)neededTime;
    sleepTime.tv_nsec=(neededTime-((int)neededTime))*100000000;

    nanosleep(&sleepTime, &rem);
    if(errno==EINTR){
        errno=0;
        while(nanosleep(&rem, &rem)==-1)
        {
            if(errno!=EINTR)
            {
                TEST_ERROR;
            }
        }
    }else
    {
        TEST_ERROR;
    }

    if(errno==4)
        errno=0;
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


int negociate(int portsID, ship s, struct ship_sharedMemory *shared_ship, int shipIndex, int *expiredGood, int sem_expired_goods_id){
    struct port_sharedMemory *ports = shmat(portsID, NULL, 0);
    int indexClosestPort= getNearestPort(ports, s.coords,-1);
    goods *g;
    int i=0, j=0;
    int destinationPortIndex=-1;
    double travelTime;
    struct timespec time, rem; 
    int startingPortSemID, destinationPortSemID;
    int goodIndex=-1;
    struct request *request;
    struct sembuf sops;
    int *shippedGoods;
    int shippedGoodsQuantity=0;
    int shippedGoodsIndex=0;

    bzero(&sops, sizeof(struct sembuf));

    while(j++<SO_NAVI && destinationPortIndex==-1 && indexClosestPort!=-1){
        if(j!=1)
            indexClosestPort = getNearestPort(ports, s.coords, getDistance(s.coords, ports[indexClosestPort].coords));

        if(indexClosestPort!=-1){
            g = shmat(ports[indexClosestPort].offersID, NULL, 0); TEST_ERROR;
        }
        else 
        {
            break;
        }

        while(g[i].type!=0 && destinationPortIndex==-1 && i<SO_FILL ){
            decreaseSem(sops, ports[indexClosestPort].semID, OFFER);TEST_ERROR;
            s.semLastID=ports[indexClosestPort].semID;
            s.semLastNum=OFFER;
            if(g[i].state==in_port){
                increaseSem(sops, ports[indexClosestPort].semID, OFFER);TEST_ERROR;
                s.semLastNum=-1;
                s.semLastID=-1;
                destinationPortIndex=getValidRequestPort(g[i],ports);
                if(destinationPortIndex!=-1){
                    if(willExpire(g[i], s, ports[indexClosestPort].coords, ports[destinationPortIndex].coords))
                        destinationPortIndex=-1;
                    else
                        goodIndex=i;
                }
            }
            else{
                increaseSem(sops, ports[indexClosestPort].semID, OFFER);
                s.semLastNum=-1;
                s.semLastID=-1;
                TEST_ERROR;
            }
            i++;
        }
        if(destinationPortIndex==-1){
            shmdt(g);TEST_ERROR;
        }     
    }

    if(goodIndex!=-1){

        shippedGoods=calloc(SO_CAPACITY, sizeof(int));
        shippedGoods[shippedGoodsIndex++]=goodIndex;
        while(g[i].type!=0){
            if(g[i].type!=g[goodIndex].type && g[i].dimension+shippedGoodsQuantity<= SO_CAPACITY && !willExpire(g[i], s, ports[indexClosestPort].coords,ports[destinationPortIndex].coords)){
                shippedGoodsQuantity+=g[i].dimension;
                shippedGoods=realloc(shippedGoods, sizeof(int)*(shippedGoodsIndex+1));
                shippedGoods[shippedGoodsIndex++]=i;
            }
            i++;
        }

        startingPortSemID = ports[indexClosestPort].semID;
        destinationPortSemID = ports[destinationPortIndex].semID; 


        request = shmat(ports[destinationPortIndex].requestID, NULL, 0); 
        if (request == (void *)-1) {
            if (errno == 22) {
                errno = 0;
                return -1;
            }
            else {TEST_ERROR;}
        }


        /*CAMBIO VALORI RICHIESTA*/      
        decreaseSem(sops, destinationPortSemID, REQUEST);TEST_ERROR;
        s.semLastNum=REQUEST;
        s.semLastID=destinationPortSemID;
        request->booked+=shippedGoodsQuantity;
        increaseSem(sops, destinationPortSemID, REQUEST);TEST_ERROR;
        s.semLastNum=-1;
        s.semLastID=-1;


        /*CAMBIO VALORI OFFERTA*/
        decreaseSem(sops, startingPortSemID, OFFER);TEST_ERROR;
        s.semLastNum=OFFER;
        s.semLastID=startingPortSemID;
        g[goodIndex].booked=1;
        increaseSem(sops, startingPortSemID, OFFER);TEST_ERROR;
        s.semLastNum=-1;
        s.semLastID=-1;


        /*moving towards the port to load goods*/
        decreaseSem(sops,shared_ship[shipIndex].semID, COORDS);
        shared_ship[shipIndex].coords.x=-1;
        shared_ship[shipIndex].coords.y=-1;
        increaseSem(sops,shared_ship[shipIndex].semID, COORDS);
        
        move(s,s.coords,ports[indexClosestPort].coords); TEST_ERROR;
        decreaseSem(sops,shared_ship[shipIndex].semID, COORDS);
        shared_ship[shipIndex].coords=ports[indexClosestPort].coords;
        increaseSem(sops,shared_ship[shipIndex].semID, COORDS);


        /*loading goods*/
        decreaseSem(sops,shared_ship[shipIndex].semID, INDOCK);
        shared_ship[shipIndex].inDock=1;
        increaseSem(sops,shared_ship[shipIndex].semID, INDOCK);

        decreaseSem(sops, startingPortSemID, DOCK);TEST_ERROR;

        for(i=0; i< shippedGoodsIndex; i++){

            loadUnload(g[shippedGoods[i]].dimension);TEST_ERROR;

            decreaseSem(sops, startingPortSemID, OFFER);TEST_ERROR;
            s.semLastNum=OFFER;
            s.semLastID=startingPortSemID;
            g[shippedGoods[i]].state=on_ship;
            increaseSem(sops, startingPortSemID, OFFER);TEST_ERROR;
            s.semLastNum=-1;
            s.semLastID=-1;

            decreaseSem(sops,shared_ship[shipIndex].semID, GOODS);
            s.goods[i]=g[shippedGoods[i]];
            increaseSem(sops,shared_ship[shipIndex].semID, GOODS);

            
        }
        increaseSem(sops, startingPortSemID, DOCK);TEST_ERROR;

        decreaseSem(sops,shared_ship[shipIndex].semID, INDOCK);
        shared_ship[shipIndex].inDock=0;
        increaseSem(sops,shared_ship[shipIndex].semID, INDOCK);


        /*moving towards the port wich made the request*/
        decreaseSem(sops,shared_ship[shipIndex].semID, COORDS);
        shared_ship[shipIndex].coords.x=-1;
        shared_ship[shipIndex].coords.y=-1;
        increaseSem(sops,shared_ship[shipIndex].semID, COORDS);
        move(s,s.coords,ports[destinationPortIndex].coords); TEST_ERROR;
        decreaseSem(sops,shared_ship[shipIndex].semID, COORDS);
        shared_ship[shipIndex].coords=ports[destinationPortIndex].coords;
        increaseSem(sops,shared_ship[shipIndex].semID, COORDS);


        /*scarico merce*/
        decreaseSem(sops,shared_ship[shipIndex].semID, INDOCK);
        shared_ship[shipIndex].inDock=1;
        increaseSem(sops,shared_ship[shipIndex].semID, INDOCK);

        decreaseSem(sops, destinationPortSemID, DOCK);TEST_ERROR;
        for(i=0; i< shippedGoodsIndex; i++){
            if(g[shippedGoods[i]].state==on_ship &&isExpired(g[shippedGoods[i]])){
                
                loadUnload(g[shippedGoods[i]].dimension);TEST_ERROR;
                decreaseSem(sops, shared_ship[shipIndex].semID, GOODS); TEST_ERROR;
                s.goods[i].state = delivered;
                increaseSem(sops, shared_ship[shipIndex].semID, GOODS); TEST_ERROR;

            }
            else{

                decreaseSem(sops,destinationPortSemID, OFFER);
                s.semLastNum=OFFER;
                s.semLastID=destinationPortSemID;
                g[shippedGoods[i]].state=expired_ship;
                increaseSem(sops,destinationPortSemID, OFFER);
                s.semLastNum=-1;
                s.semLastID=-1;
                decreaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;
                expiredGood[(g[shippedGoods[i]].type)-1]+=g[shippedGoods[i]].dimension;
                increaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;
            }
        }
        increaseSem(sops, destinationPortSemID, DOCK);TEST_ERROR;

        decreaseSem(sops,shared_ship[shipIndex].semID, INDOCK);
        shared_ship[shipIndex].inDock=0;
        increaseSem(sops,shared_ship[shipIndex].semID, INDOCK);


        /*CAMBIO VALORI RICHIESTA*/
        decreaseSem(sops, destinationPortSemID, REQUEST);TEST_ERROR;
        s.semLastID=destinationPortSemID;
        s.semLastNum=REQUEST;
        request->satisfied+=shippedGoodsQuantity;
        increaseSem(sops, destinationPortSemID, REQUEST); TEST_ERROR;
        s.semLastNum=-1;
        s.semLastID=-1;
        
        shmdt(request); TEST_ERROR;
        shmdt(g); TEST_ERROR;

        decreaseSem(sops, shared_ship[shipIndex].semID, GOODS);
        bzero(s.goods, sizeof(goods)*SO_CAPACITY);
        increaseSem(sops, shared_ship[shipIndex].semID, GOODS);

        free(shippedGoods);
        
        shmdt(ports); TEST_ERROR;

        return destinationPortIndex;
    }else{
        return -1;
    }
}



int getValidRequestPort(goods good, struct port_sharedMemory * sh_port) {
    struct msg_request msg;
    int ret = 0, first_idx = -1, sem_id, i;
    struct request *request;
    struct sembuf sops;
    int msg_id;

    bzero(&sops, sizeof(sops));

    msg_id=msgget(getppid(), 0600);
    if (errno == 2) {
        errno = 0;
        return -1;
    }
    else
        {TEST_ERROR;}

    for (i = 0; i < SO_PORTI; i++) {
        ret = msgrcv(msg_id, &msg, sizeof(struct msg_request), good.type, IPC_NOWAIT); 

        if(errno!=42){
            TEST_ERROR;
        }else 
            errno=0;

        if (ret == -1)
            return -1;

        sem_id = sh_port[msg.idx].semID;
        request = shmat(sh_port[msg.idx].requestID, NULL, 0); TEST_ERROR;

        decreaseSem(sops, sem_id, 1);TEST_ERROR;

        if (request -> satisfied < request -> quantity) {
            increaseSem(sops, sem_id, 1);
            msgsnd(msg_id, &msg, sizeof(struct msg_request), 0);
            shmdt(request); TEST_ERROR;
            return msg.idx;
        }else{
            increaseSem(sops, sem_id, 1);TEST_ERROR;
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

