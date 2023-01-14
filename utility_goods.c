#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "macro.h"
#include "types_module.h"
#include "utility_goods.h"


void printGood(goods g){
    char *string;
	int numBytes;
	string=malloc(85);
	numBytes=sprintf(string,"\n\n======>MERCE:\nTipo: %d\nQuantità: %d\nState: %d\nBooked:%d scadenza  il giorno %ld", g.type, g.dimension, g.state, g.booked, g.expiringDate.tv_sec);

	fflush(stdout);
	write(1, string, numBytes);
	free(string); 
}

goods generateGoods(int type){
    goods g;
    struct timespec t;

    clock_gettime(CLOCK_REALTIME, &t);
    g.type=type;
    g.booked=0;
    g.dimension = (t.tv_nsec%SO_SIZE) + 1;
    g.state=in_port;
    clock_gettime(CLOCK_REALTIME, &t);

    if (SO_MIN_VITA == SO_MAX_VITA) t.tv_sec+= SO_MIN_VITA;
    else t.tv_sec=(t.tv_nsec%(SO_MAX_VITA-SO_MIN_VITA))+SO_MIN_VITA + 1;

    g.expiringDate=t;

    return g;
}


int isExpired(goods g){
    
    struct timespec now;
    
    clock_gettime(CLOCK_REALTIME, &now);

    if(now.tv_sec>g.expiringDate.tv_sec) return 1;
    else if (now.tv_sec<g.expiringDate.tv_sec) return 0;
    else if (now.tv_nsec> g.expiringDate.tv_nsec) return 1;
    else return 0;
}


int willExpire(goods g, ship s, coordinates from ,coordinates to){
    double totalTime=0;
    struct timespec endTravelDate;
    totalTime+=getTravelTime(getDistance(from, s.coords)+getDistance(from, to));
    totalTime+=2*(g.dimension*SO_LOADSPEED);

    clock_gettime(CLOCK_REALTIME, &endTravelDate);

    endTravelDate.tv_sec += (int)totalTime;
    if(endTravelDate.tv_nsec+(totalTime-(int)totalTime)>=1000000000){
        endTravelDate.tv_sec++;
        endTravelDate.tv_nsec=endTravelDate.tv_nsec-1000000000+(totalTime-(int)totalTime);
    }

    if(endTravelDate.tv_sec>g.expiringDate.tv_sec)  return 1;
    else if (endTravelDate.tv_sec<g.expiringDate.tv_sec) return 0;
    else if (endTravelDate.tv_nsec> g.expiringDate.tv_nsec) return 1;
    else return 0;

}