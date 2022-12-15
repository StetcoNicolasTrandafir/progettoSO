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

    clock_gettime(CLOCK_REALTIME, &g.generationTime);
    clock_gettime(CLOCK_REALTIME, &t);
    g.type=type;
    g.satisfied=0;
    g.dimension= (t.tv_nsec%SO_SIZE)+1;
    g.state=in_port;
    clock_gettime(CLOCK_REALTIME, &t);
    g.lifeTime=(t.tv_nsec%(SO_MAX_VITA-SO_MIN_VITA))+SO_MIN_VITA+1;

    return g;
}


int isExpired(goods g){
    
    struct timespec now;
    
    clock_gettime(CLOCK_REALTIME, &now);
    
    if(now.tv_sec-g.generationTime.tv_sec > lifeTime ) return 0;
    else if (now.tv_sec-g.generationTime.tv_sec == lifeTime){
        if(now.tv_nsec >= g.generationTime.tv_nsec) return 1;
        else return 0;
    }else return 1;

    return ((currentTime-g.generationTime)>=g.lifeTime) ? 1 : 0;
}