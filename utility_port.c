#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "macro.h"
#include "utility_goods.h"
#include "utility_coordinates.h"
#include "utility_port.h"



#define IN_PORT 2
#define SHIPPED 1
#define ALL 0
#define ONLY_SATISFIED 1

void printDailyReport(port p){
    char *report;
    int tonsShipped, tonsInPort, tonsReceived, freeDocks = 0, num_bytes; /*TODO quante banchine libere?*/
    report = malloc(200);
    tonsShipped= getGeneratedGoods(p, SHIPPED);
    tonsInPort=getGeneratedGoods(p, IN_PORT);
    tonsReceived=getRequest(p, ONLY_SATISFIED);
    num_bytes = sprintf(report, "Porto [%d] in posizione: (%.2f, %.2f)\nBanchine libere %d su %d\nMerci spedite: %d ton\nMerci generate ancora in porto: %d ton\nMerci ricevute: %d ton\n\n", getpid(), p.coord.x, p.coord.y, freeDocks, p.docks, tonsShipped, tonsInPort, tonsReceived);
    fflush(stdout);
    write(1, report, num_bytes);
    free(report);
}

void initializeRequestsAndOffer(port p){
    int i=0;

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
            break;

        case ALL:
            return p.request -> quantity;
            break;
            
        default:
            /*INVALID FLAG*/
            return -1;
            break;
    }

    return total;
}



int getGeneratedGoods(port p, int flag){
    int i=0;
    int total=0;

    while(i<SO_DAYS && p.generatedGoods[i].type!=-1){

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

void generateOffer(port p, int idx){
    int type;
    int plus = 0;
    goods goods;
    srand(getpid());
    type = rand() % SO_MERCI;
    while(plus < SO_MERCI && isRequested(p, (type + plus) % SO_MERCI)){
        plus++;
    }

    if(plus == SO_MERCI) 
        printf("Impossibile generare un'offerta al porto [%d] in posizione: (%2.f, %2.f)\n", getpid(), p.coord.x, p.coord.y);

    goods = generateGoods((type + plus) % SO_MERCI);
    p.generatedGoods[idx] = goods;
}

void generateRequest(port p){
    struct timespec t;

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
    p.request -> quantity = t.tv_nsec % 100;
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