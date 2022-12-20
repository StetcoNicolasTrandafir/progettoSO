#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "macro.h"
#include "types_module.h"
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
    
    if(now.tv_sec-g.generationTime.tv_sec > g.lifeTime ) return 0;
    else if (now.tv_sec-g.generationTime.tv_sec == g.lifeTime){
        if(now.tv_nsec >= g.generationTime.tv_nsec) return 1;
        else return 0;
    }else return 1;
}


int willExpire(goods g, int quantity, ship s, port startingPort, port destinationPort){
    double totalTime=0;
    struct timespec expireTime;
    totalTime+=getTravelTime(getDistance(startingPort.coord, s.coords)+getDistance(startingPort.coord, destinationPort.coord));
    totalTime+=2*(quantity*SO_LOADSPEED);

    clock_gettime(CLOCK_REALTIME, &expireTime);

    expireTime.tv_sec += (int)totalTime;
    if(expireTime.tv_nsec+(totalTime-(int)totalTime)>=1000000000){
        expireTime.tv_sec++;
        expireTime.tv_nsec=expireTime.tv_nsec-1000000000+(totalTime-(int)totalTime);
    }

    if(expireTime.tv_sec > g.generationTime.tv_sec+g.lifeTime) return 0;
    else if(expireTime.tv_nsec== g.generationTime.tv_sec+g.lifeTime){
        if(expireTime.tv_nsec>=g.generationTime.tv_nsec) return 0;
    }else return 1;

}