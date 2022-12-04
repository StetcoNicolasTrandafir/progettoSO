#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "macro.h"
#include "utility_goods.h"


struct goods generateGoods(){
    struct goods g;
    srand(getpid());
    //TODO: impostare il generationTime "all'orario" corrente
    g.generationTime=0;
    g.type=(rand()%SO_MERCI);
    g.dimension= (rand()%SO_SIZE)+1;
    g.state=in_port;

    return g;
}


int isExpired(struct goods g){
    //TODO: valorizzare correttamente la variabile currentTime 
    double currentTime=(double)SO_DAYS; 
    //NOTE: per adesso impostato al massimo dei giorni, in questo modo isExpired() sarà sempre falso

    return ((currentTime-g.generationTime)>=g.lifeTime) ? 1 : 0;
}