#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "macro.h"
#include "utility_goods.h"


goods generateGoods(int type){
    goods g;
    struct timespec t;
    /*TODO: impostare il generationTime "all'orario" corrente*/
    clock_gettime(CLOCK_REALTIME, &g.generationTime);
    clock_gettime(CLOCK_REALTIME, &t);
    g.type=type;
    g.dimension= (t.tv_nsec%SO_SIZE)+1;
    g.state=in_port;
    clock_gettime(CLOCK_REALTIME, &t);
    g.lifeTime=(t.tv_nsec%(SO_MAX_VITA-SO_MIN_VITA))+SO_MIN_VITA+1;

    return g;
}


/*int isExpired(goods g){
    /*TODO: valorizzare correttamente la variabile currentTime 
    double currentTime=(double)SO_DAYS; 
    /*NOTE: per adesso impostato al massimo dei giorni, in questo modo isExpired() sarà sempre falso

    return ((currentTime-g.generationTime)>=g.lifeTime) ? 1 : 0;
}*/