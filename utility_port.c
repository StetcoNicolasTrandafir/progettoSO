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
    int tonsShipped, tonsInPort, tonsReceived, freeDocks = 0; /*TODO quante banchine libere?*/
    tonsShipped= getGeneratedGoods(p, SHIPPED);
    tonsInPort=getGeneratedGoods(p, IN_PORT);
    tonsReceived=getRequest(p, ONLY_SATISFIED);
    printf("Porto in posizione: (%.2f, %.2f)\nBanchine libere %d su %d\nMerci spedite: %d ton\nMerci generate ancora in porto: %d ton\nMerci ricevute: %d ton\n\n", p.coord.x, p.coord.y, freeDocks, p.docks, tonsShipped, tonsInPort, tonsReceived);
    /*TODO Gestire l'ordine di stampa del report*/
}

port initializePort(port p){
    int i=0;

    p.request.goodsType=-1;
    p.generatedGoods=calloc(SO_DAYS, sizeof(goods));
    for(i=0; i< SO_DAYS; i++){
        p.generatedGoods[i].type=-1;
    }
    return p;
}

/*NOTE assumo che gli array p.requests e p.generatedGoods siano 
    array NULL terminated e di lunghezza SO_DAYS*/

int getRequest(port p, int satisfied){
    int i=0;
    int total=0;

    switch(satisfied){

        case ONLY_SATISFIED:    
            return p.request.quantity;
            break;

        case ALL:
            return p.request.quantity;
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

int generateOffer(port p, int day){
    int type;
    int plus=0;
    goods goods;
    srand(getpid());
    type= rand()%SO_MERCI;
    while(plus<SO_MERCI && isRequested(p, (type+plus)%SO_MERCI)){
        plus++;
    }

    if(plus==SO_MERCI) return -1;

    goods=generateGoods((type+plus)%SO_MERCI);
    p.generatedGoods[day]=goods;

    return goods.type;
}

int generateRequest(port p){
    int type;
    int plus=0;
    struct request req;

    srand(getpid());
    type = rand()%SO_MERCI;
    while(plus<SO_MERCI && isOffered(p, (type+plus)%SO_MERCI)){
        plus++;
    }
    if(plus==SO_MERCI) return -1;

    req.goodsType=(type+plus)%SO_MERCI;
    req.satisfied=0;
    /*REVIEW QUESTO È SBAGLIATISSIMO MA NON SO COSA METTERE ORA*/
    req.quantity=(rand()%SO_SIZE)+1;
    
    p.request=req;

    return req.goodsType;
}


int isOffered(port port, int goodsType){
    int i=0;
    while(port.generatedGoods[i].type!=-1 && i<SO_DAYS) {
        if (port.generatedGoods[i].type == goodsType && port.generatedGoods[i].state==in_port) return 1;
        i++;
    }
    return 0;
}

int isRequested(port port, int goodsType){ return (goodsType==port.request.goodsType && port.request.quantity > port.request.satisfied) ? 1:0; }