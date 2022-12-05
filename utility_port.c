#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "macro.h"
#include "utility_goods.h"
#include "utility_coordinates.h"
#include "utility_port.h"




#define IN_PORT 2
#define SHIPPED 1
#define ALL 0
#define ONLY_SATISFIED 1

void printDailyPortRepo(port port){
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