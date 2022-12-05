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

void printDailyReport(port port){

    int tonsShipped= getGeneratedGoods(port, SHIPPED);
    int tonsInPort=getGeneratedGoods(port, IN_PORT);
    int tonsReceived=getRequests(port, ONLY_SATISFIED);
    
    /*TODO quante banchine libere?*/
    int freeDocks=0;

    printf("\n\n\nPorto in posizione:\n");
    printCoords(port.coord);
    printf("Banchine libere %d su %d", freeDocks, port.docks);

    printf("Merci spedite: %d ton",tonsShipped);
    printf("\nMerci in generate ancora in porto: %d ton",tonsInPort);
    printf("\nMerci in ricevute: %d ton",tonsReceived);

}

void initializePort(port port){
    int i=0;
    
    
    port.requests=calloc(SO_DAYS, sizeof(struct request));
    printf("\r req\n");
	port.generatedGoods=calloc(SO_DAYS, sizeof(goods));
    printf("\n gen\n");
    for(i=0; i< SO_DAYS; i++ ){
        printf("\nCleaning the element number %d\n", i);
        port.requests[i].goodsType=-1;
        port.generatedGoods[i].type=-1;
        printf("\nCleaned the element number %d\n", i);
    }
    printf("\nHo finito");
}


/*NOTE assumo che gli array port.requests e port.generatedGoods siano 
    array NULL terminated e di lunghezza SO_DAYS*/

int getRequests(port port, int satisfied){
    int i=0;
    int total=0;
    
    while(i<SO_DAYS && port.requests[i].goodsType!=-1){
        switch(satisfied){

            case ONLY_SATISFIED:
                if(port.requests[i].satisfied)
                    total+= port.requests[i].quantity;
                break;

            case ALL:
                total+= port.requests[i].quantity;
                break;
            
            default:
                /*INVALID FLAG*/
                return -1;
                break;
        }
    }

    return total;
}



int getGeneratedGoods(port port, int flag){
    int i=0;
    int total=0;

    while(i<SO_DAYS && port.generatedGoods[i].type!=-1){

        switch(flag){
            case SHIPPED:
                if(port.generatedGoods[i].state!=in_port && port.generatedGoods[i].state!=expired_port)
                    total+=port.generatedGoods[i].dimension;
                break;

            case IN_PORT:
                if(port.generatedGoods[i].state==in_port /*|| port.generatedGoods[i].state!=expired_port*/)
                    total+=port.generatedGoods[i].dimension;
                break;
             
            case ALL:
                total+=port.generatedGoods[i].dimension;
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

int generateOffer(port port, int day){
    int type;
    int plus=0;
    goods goods;
    srand(getpid());
    type= rand()%SO_MERCI;
    while(plus<SO_MERCI && isRequested(port,(type+plus)%SO_MERCI)){
        printf("\nPLUS= %d", plus);
        plus++;
    }

    if(plus==SO_MERCI) return -1;

    goods=generateGoods((type+plus)%SO_MERCI);
    port.generatedGoods[day]=goods;

    return goods.type;
}

int generateRequest(port port, int day){
    int type;
    int plus=0;
    struct request req;

    srand(getpid());
    type= rand()%SO_MERCI;

    while(plus<SO_MERCI && isOffered(port,(type+plus)%SO_MERCI)){
        plus++;
    }

    if(plus==SO_MERCI) return -1;

    req.goodsType=(type+plus)%SO_MERCI;
    req.satisfied=0;
    /*REVIEW QUESTO È SBAGLIATISSIMO MA NON SO COSA METTERE ORA*/
    req.quantity=(rand()%SO_SIZE)+1;
    
    port.requests[day]=req;

    return req.goodsType;
}


int isOffered(port port, int goodsType){
    int i=0;
    while(port.generatedGoods[i].type!=-1 && i<SO_DAYS) 
        if (port.generatedGoods[i].type== goodsType && port.generatedGoods[i++].state==in_port) return 1;
    return 0;
}

int isRequested(port port, int goodsType){
    int i=0;
    while(port.requests[i].goodsType!=-1 && i<SO_DAYS) 
        if(port.requests[i].goodsType==goodsType && port.requests[i].quantity > port.requests[i++].satisfied)  return 1;
    return 0;
}
