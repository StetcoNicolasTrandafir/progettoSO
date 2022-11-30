#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


#include "coordinates.h"
#include "macro.h"


struct coordinates randomCoords(){
    srand(getpid());
    struct coordinates coords= {rand()%SO_LATO, rand()%SO_LATO};
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