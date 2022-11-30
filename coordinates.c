#include <stdlib.h>
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