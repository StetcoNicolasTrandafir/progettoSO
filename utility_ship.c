#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>

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
    goods *g= shmat(ports[indexClosestPort], NULL, 0);
    int i=0, j=0;
    int destinationPortIndex=-1;
    double travelTime;
    struct timespec time, rem; 
    int goodsQuantity;
    struct sembuf semaphores;
    int startingPortSemID, destinationPortSemID;

    while(j++<SO_NAVI && destinationPortIndex==-1){

        indexClosestPort= getNearestPort(ports, s.coords, getDistance(s.coords, ports[indexClosestPort].coords));
        g= shmat(ports[indexClosestPort], NULL, 0);

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