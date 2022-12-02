#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "macro.h"
#include "coordinates.h"


struct coordinates getRandomCoords(){
    double x, y;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    x = (double)(now.tv_nsec % (SO_LATO * 1000000)) / 1000000.0;
    clock_gettime(CLOCK_REALTIME, &now);
    y = (double)(now.tv_nsec % (SO_LATO * 1000000)) / 1000000.0;
    struct coordinates coords= {x, y};
    return coords;
}

void printCoords(struct coordinates coords){
    printf("X: %lf - Y: %lf", coords.x, coords.y);
}

double getDistance(struct coordinates A,struct coordinates B){
    double deltaX= A.x-B.x;
    double deltaY= A.y-B.y;
    return sqrt(pow(deltaY,2)+pow(deltaX,2));
}

double getTravelTime(double space){
    return space/SO_SPEED;
}