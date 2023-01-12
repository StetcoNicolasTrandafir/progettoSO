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
}


/*NON È PIÙ NECESSARIO E IO GODO COME UN RICCIO*/
/*coordinates getPosition(ship ship, struct timespec rem){
    if(rem){
        nanosleep(rem, rem);
    }else{
        return ship.coords;
    }
}*/


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

void loadUnload(int quantity){
    float neededTime= quantity/SO_LOADSPEED;
    struct timespec sleepTime;

    sleepTime.tv_sec=(int)neededTime;
    sleepTime.tv_nsec=(neededTime-((int)neededTime))*100000000;

    nanosleep(&sleepTime, &sleepTime);
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
7) se non ci sono richieste per nessuna offerta del porto preso in considerazione, prendo in considerazione ilc
*/



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
    int *shippedGoods;
    int shippedGoodsQuantity=0;
    int shippedGoodsIndex=0;

    bzero(&sops, sizeof(struct sembuf));
    bzero(s.goods, sizeof(goods)*SO_CAPACITY);

    while(j++<SO_NAVI && destinationPortIndex==-1 && indexClosestPort!=-1){
        indexClosestPort= getNearestPort(ports, s.coords, getDistance(s.coords, ports[indexClosestPort].coords));
        g= shmat(ports[indexClosestPort].offersID, NULL, 0);TEST_ERROR;

        while(g[i].type!=0 && destinationPortIndex==-1 && i<SO_FILL ){
            decreaseSem(sops, ports[indexClosestPort].semID, OFFER);TEST_ERROR;
            if(g[i].state==in_port){
                increaseSem(sops, ports[indexClosestPort].semID, OFFER);TEST_ERROR;
                destinationPortIndex=getValidRequestPort(g[i],ports);
                if(destinationPortIndex!=-1)
                    goodIndex=i;
            }
            else{
                increaseSem(sops, ports[indexClosestPort].semID, OFFER);
                TEST_ERROR;
            }
            i++;
        }
        if(destinationPortIndex==-1){
            shmdt(g);TEST_ERROR;
        }     
    }


    
    /*NOTE: 
invalid argumento, forse perchè ===>
    set di semafori non esistente
    
    */


    if(goodIndex!=-1){

        /*!SECTION
        conosco il tipo di bene per cui c'è richiesta
        0)load/unload e move
        1)mi muovo verso il porto
        2)carico tutte le offerte che non scadono di quel bene
            -aggiorno tutti gli state dei lotti di offerta
            -carico tutti i lotti sulla nave
        3)mi muovo verso la destinazione
        4)scarico merci
            -per ogni lotto, prima di scaricare ==> check scadenza
            -unload lotto
            -scarico nave ==> aggiorno stati
        5) negociate
        */


        shippedGoods=calloc(SO_CAPACITY, sizeof(int));
        shippedGoods[shippedGoodsIndex++]=goodIndex;
        while(g[i].type!=0){
            if(g[i].type!=g[goodIndex].type && g[i].dimension+shippedGoodsQuantity<= SO_CAPACITY){
                shippedGoodsQuantity+=g[i].dimension;
                shippedGoods=realloc(shippedGoods, sizeof(int)*(shippedGoodsIndex+1));
                shippedGoods[shippedGoodsIndex++]=i;
            }
            i++;
        }
        startingPortSemID = ports[indexClosestPort].semID;
        destinationPortSemID = ports[destinationPortIndex].semID;


        for(i=0; i<shippedGoodsIndex; i++){
            g[shippedGoods[i]].state=on_ship;
            s.goods[i]=g[shippedGoods[i]];
        }

        /*startingPortSemID=semget(ports[indexClosestPort].semID, 3, 0600);TEST_ERROR;
        destinationPortSemID=semget(ports[destinationPortIndex].semID, 3, 0600); TEST_ERROR; /*[0]=banchine [1]=offerta [2]=richiesta*/

        /*TODO AGGIORNO I BOOKED DELLA RICHIESTA E DELL'OFFERTA*/

        /*CAMBIO VALORI RICHIESTA*/
        
        request = shmat(ports[destinationPortIndex].requestID, NULL, 0); TEST_ERROR;
        
        decreaseSem(sops, destinationPortSemID, REQUEST);TEST_ERROR;

        shippedQuantity=min((request->quantity - request->booked), (g[goodIndex].dimension - g[goodIndex].booked));
        request->booked+=shippedQuantity;

        increaseSem(sops, destinationPortSemID, REQUEST);TEST_ERROR;
        /*UNLOCK(destinationPortIndex, 2)*/

        /*CAMBIO VALORI OFFERTA*/
        /*LOCK(destinationPortIndex, 2)*/
        decreaseSem(sops, startingPortSemID, OFFER);TEST_ERROR;

        g[goodIndex].booked+=shippedQuantity;

        increaseSem(sops, startingPortSemID, OFFER);TEST_ERROR;


        /*moving towards the port to load goods*/

/*TODO controllo semafori navi*/

        shared_ship.coords.x=-1;
        shared_ship.coords.x=-1;

        move(s.coords,ports[indexClosestPort].coords); TEST_ERROR;

        s.coords= ports[indexClosestPort].coords;
        shared_ship.coords=s.coords;

        /*loading goods*/
        /*LOCK(destinationPortIndex, 2)*/
        decreaseSem(sops, startingPortSemID, DOCK);TEST_ERROR;

        shared_ship.inDock=1;
        for(i=0; i< shippedGoodsIndex; i++){
            loadUnload(g[shippedGoods[i]].dimension);
            TEST_ERROR;
        }
        
        shared_ship.inDock=0;

        increaseSem(sops, startingPortSemID, DOCK);TEST_ERROR;

        /*CAMBIO VALORI OFFERTA*/
        /*LOCK(destinationPortIndex, 2)*/

        shmdt(g); TEST_ERROR;
        
        /*moving towards the port wich made the request*/

        shared_ship.coords.x=-1;
        shared_ship.coords.x=-1;

        move(s.coords,ports[destinationPortIndex].coords); TEST_ERROR;


        /*arrived at the port, downloading the goods*/
        s.coords=ports[destinationPortIndex].coords;
        shared_ship.coords=s.coords;

        /*LOCK(destinationPortIndex, 2)*/
        decreaseSem(sops, destinationPortSemID, DOCK);TEST_ERROR;

        shared_ship.inDock=1;

        checkExpiredGoods(s,shippedGoodsIndex,shippedGoods);
        for(i=0; i< shippedGoodsIndex; i++){
            if(shippedGoods[i]!=-1){
                loadUnload(g[shippedGoods[i]].dimension);
                TEST_ERROR;
            }
                
        }
        
        shared_ship.inDock=0;

        increaseSem(sops, destinationPortSemID, DOCK);TEST_ERROR;


        /*CAMBIO VALORI RICHIESTA*/
        /*LOCK(destinationPortIndex, 2)*/
        decreaseSem(sops, destinationPortSemID, REQUEST);TEST_ERROR;

        request->satisfied+=shippedQuantity;

        increaseSem(sops, destinationPortSemID, REQUEST); TEST_ERROR;
        
        shmdt(request); TEST_ERROR;

        string=malloc(70);
        numBytes=sprintf(string,"\n[%d]FINITO DI SCARICARE! Merce portata dal punto A al punto B!\n\n", getpid());

        fflush(stdout);
        write(1, string, numBytes);
        
        free(shippedGoods);
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

        if (ret == -1)
            return -1;

        sem_id = sh_port[msg.idx].semID;
        request = shmat(sh_port[msg.idx].requestID, NULL, 0); TEST_ERROR;

        decreaseSem(sops, sem_id, 1);

        q = min(min(SO_CAPACITY, request -> quantity - request -> booked), good.dimension);
        if (request -> booked < request -> quantity) {
            request -> booked += q;

            increaseSem(sops, sem_id, 1);

            msgsnd(msg_id, &msg, sizeof(struct msg_request), 0);
            shmdt(request);
            return msg.idx;
        }else{
            increaseSem(sops, sem_id, 1);
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

