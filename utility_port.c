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

void printDailyReport(port p){
    char *report;
    int tonsShipped, tonsInPort, tonsReceived, freeDocks = 0, num_bytes; /*TODO quante banchine libere?*/
    report = malloc(200);
    tonsShipped = getGeneratedGoods(p, SHIPPED);
    tonsInPort = getGeneratedGoods(p, IN_PORT);
    tonsReceived = getRequest(p, ONLY_SATISFIED);
    num_bytes = sprintf(report, "Porto [%d] in posizione: (%.2f, %.2f)\nBanchine libere %d su %d\nMerci spedite: %d ton\nMerci generate ancora in porto: %d ton\nMerci ricevute: %d ton\n\n", getpid(), p.coords.x, p.coords.y, freeDocks, p.docks, tonsShipped, tonsInPort, tonsReceived);
    fflush(stdout);
    write(1, report, num_bytes);
    free(report);
}

void initializeRequestsAndOffer(port p){
    int i=0;

    p.request -> booked = 0;
    p.request -> satisfied = 0;
    p.request -> goodsType=-1;
    p.generatedGoods=calloc(SO_DAYS, sizeof(goods));
    for(i=0; i< SO_DAYS; i++){
        p.generatedGoods[i].type=-1;
    }
    /*return p;*/
}

/*NOTE assumo che gli array p.requests e p.generatedGoods siano 
    array NULL terminated e di lunghezza SO_DAYS*/

int getRequest(port p, int satisfied){
    int i=0;
    int total=0;

    switch(satisfied){

        case ONLY_SATISFIED:  
            return p.request -> satisfied;

        case ALL:
            return p.request -> quantity;
            
        default:
            /*INVALID FLAG*/
            return -1;
    }
    /*return total;*/
}



int getGeneratedGoods(port p, int flag){
    int i=0;
    int total=0;
    printf("Entrato in generateGoods\n");
    while(i<SO_DAYS && p.generatedGoods[i].type!=-1){
        printf("Ciclo %d\n", i);

        switch(flag){
            case SHIPPED:
                if(p.generatedGoods[i].state!=in_port && p.generatedGoods[i].state!=expired_port)
                    total+=p.generatedGoods[i].dimension;
                break;

            case IN_PORT:
                if(p.generatedGoods[i].state==in_port /*|| p.generatedGoods[i].state!=expired_port*/)
                    total+=p.generatedGoods[i].dimension;
                break;
             
            case ALL:
                total+=p.generatedGoods[i].dimension;
                break;
            
            default:
                /*INVALID FLAG*/
                return -1;
                break;
        }
        i++;
    }

    return total;
}

void generateOffer(port p, int idx, int sum_offerID, int sem_sum_id){
    struct timespec t;
    struct sembuf sops;
    int type, *sum_offer;
    int plus = 0;
    int numBytes;
    char *string; 
    goods goods;

    bzero(&sops, sizeof(struct sembuf));

    srand(getpid());
    type = rand() % SO_MERCI + 1;
    while(plus < SO_MERCI && isRequested(p, (type + plus) % SO_MERCI)){
        plus++;
    }

    if(plus == SO_MERCI){
        string=malloc(90);
        numBytes=sprintf(string,"Impossibile generare un'offerta al porto [%d] in posizione: (%2.f, %2.f)\n", getpid(), p.coords.x, p.coords.y);
        fflush(stdout);
        write(1, string, numBytes);
        free(string);
    }else{
        sum_offer = shmat(sum_offerID, NULL, 0); TEST_ERROR;

        goods = generateGoods((type + plus) % SO_MERCI);
        goods.type++;
        clock_gettime(CLOCK_REALTIME, &t);
        goods.dimension = t.tv_nsec % 1000;

        decreaseSem(sops, sem_sum_id, 2);

        *sum_offer += goods.dimension;

        increaseSem(sops, sem_sum_id, 2);

        decreaseSem(sops, sem_sum_id, 3);

        waitForZero(sops, sem_sum_id, 3);


        if((goods.dimension = round((goods.dimension * (SO_FILL / SO_DAYS)) / *sum_offer)) == 0)
            goods.dimension++;


        shmdt(sum_offer); TEST_ERROR;

        p.generatedGoods[idx] = goods;
    }
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
    decreaseSem(sops, semID, OFFER);
    while(port.generatedGoods[i].type!=-1 && i<SO_DAYS){


        if(port.generatedGoods[i].state==in_port &&isExpired(port.generatedGoods[i])){
            port.generatedGoods[i].state=expired_port;
        }
            
        i++;
    }

    increaseSem(sops, semID, OFFER);
}