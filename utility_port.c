#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>
#include <math.h>

#include "macro.h"
#include "semaphore_library.h"
#include "types_module.h"
#include "utility_goods.h"
#include "utility_port.h"

#define IN_PORT 2
#define SHIPPED 1
#define ALL 0
#define ONLY_SATISFIED 1

int generateOffer(port p, int idx, int numPortShmID, int sem_offer_id){
    struct timespec t;
    struct sembuf sops;
    int type, *numPorts;
    int plus = 0;
    int numBytes;
    char *string; 
    int quantityToGenerate;
    int generatedQuantity=0;
    goods goods;
    int i;
    
    bzero(&sops, sizeof(struct sembuf));

    numPorts = shmat(numPortShmID, NULL, 0); TEST_ERROR;

    quantityToGenerate=SO_FILL/SO_DAYS;
    quantityToGenerate=quantityToGenerate/(*numPorts);

    while(generatedQuantity<quantityToGenerate) {
        clock_gettime(CLOCK_REALTIME, &t);
        type=(t.tv_nsec%SO_MERCI)+1;

        if(p.request->goodsType==type){
            if(SO_MERCI==1){
                string=malloc(90);
                numBytes=sprintf(string,"Impossibile generare un'offerte al porto [%d] in posizione: (%2.f, %2.f)\n", getpid(), p.coords.x, p.coords.y);
                fflush(stdout);
                write(1, string, numBytes);
                free(string);
                break;
            }else{
                type=(type%SO_MERCI)+1;
            }
        }

        goods = generateGoods(type);
        generatedQuantity+=goods.dimension;

        if(generatedQuantity>quantityToGenerate){
            goods.dimension-=(generatedQuantity-quantityToGenerate);
        }

        decreaseSem(sops, sem_offer_id, 1);
        p.generatedGoods[idx++] = goods;
        increaseSem(sops, sem_offer_id, 1);

    }

    shmdt(numPorts); TEST_ERROR;
    return idx;
}


void generateRequest(port p, int sum_requestID, int sem_sum_id){
    struct timespec t;
    int *sum_request;
    struct sembuf sops;

    bzero(&sops, sizeof(struct sembuf));

    srand(getpid());
    p.request -> goodsType = (rand() % SO_MERCI) + 1;

    /*req.goodsType=(type+plus)%SO_MERCI;*/
    p.request -> satisfied = 0;
    p.request -> booked = 0;
    /*REVIEW QUESTO È SBAGLIATISSIMO MA NON SO COSA METTERE ORA
    q = SO_FILL / SO_PORTI;
    x = q * 1 / 10;
    req.quantity = (rand() % ((q + x) - (q - x))) + (q - x);*/
    clock_gettime(CLOCK_REALTIME, &t);
    p.request -> quantity = t.tv_nsec % 1000;

    sum_request = shmat(sum_requestID, NULL, 0); TEST_ERROR;


    decreaseSem(sops, sem_sum_id, 0);

    *sum_request += p.request -> quantity;

    increaseSem(sops, sem_sum_id, 0);

    decreaseSem(sops, sem_sum_id, 1);

    waitForZero(sops, sem_sum_id, 1);

    if((p.request -> quantity = round((p.request -> quantity * SO_FILL) / *sum_request)) == 0)
        p.request -> quantity++;

    shmdt(sum_request); TEST_ERROR;
}


int isOffered(port port, int goodsType){
    int i=0;
    while(port.generatedGoods[i].type!=-1 && i<SO_DAYS) {
        if (port.generatedGoods[i].type == goodsType && port.generatedGoods[i].state==in_port) return 1;
        i++;
    }
    return 0;
}

int isRequested(port port, int goodsType){ return (goodsType==port.request -> goodsType && port.request -> quantity > port.request -> satisfied) ? 1:0; }

void updateGoods(port port, int semID){
    int i=0;
    struct sembuf sops;
    
    while(port.generatedGoods[i].type!=-1 && i<SO_DAYS){

        /*printf("\n\nPORTO IN POSIZIONE (%.2f,%.2f) CONTROLLO %dton DI MERCE TI TIPO: %d...", port.coords.x,port.coords.y, port.generatedGoods[i].dimension, port.generatedGoods[i].type);*/

        if(port.generatedGoods[i].state==in_port && isExpired(port.generatedGoods[i])){
            decreaseSem(sops, semID, OFFER);
            /*printf("\n\nSTATO MERCE PRIMA: %d", port.generatedGoods[i].state);*/
            port.generatedGoods[i].state=expired_port;
            /*printf("\n\nSTATO MERCE DOPO: %d", port.generatedGoods[i].state);*/

            increaseSem(sops, semID, OFFER);
        }
        i++;
    }

}