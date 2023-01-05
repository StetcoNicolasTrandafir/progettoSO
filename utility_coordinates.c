#define _GNU_SOURCE
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "macro.h"
#include "types_module.h"
#include "utility_coordinates.h"


coordinates getRandomCoords(){
    double x, y;
    struct timespec now;
    coordinates coords;
    clock_gettime(CLOCK_REALTIME, &now);
    x = (double)(now.tv_nsec % (SO_LATO * 100)) / 100.0;
    clock_gettime(CLOCK_REALTIME, &now);
    y = (double)(now.tv_nsec % (SO_LATO * 100)) / 100.0;
    coords.x=x;
    coords.y=y;
    return coords;
}

int existCoords(coordinates coordv[], int idx, coordinates coord) {
    int j;
    for (j = 0; j < idx; j++) {
        if ((coord.x < coordv[j].x + SO_DISTANZA_PORTI && coord.x > coordv[j].x - SO_DISTANZA_PORTI) &&
            (coord.y < coordv[j].y + SO_DISTANZA_PORTI && coord.y > coordv[j].y - SO_DISTANZA_PORTI))
            return 1;
    }
    return 0;
}

double getDistance( coordinates A, coordinates B){
    double deltaX= A.x-B.x;
    double deltaY= A.y-B.y;
    double ris=sqrt(pow(deltaY,2)+pow(deltaX,2));

    return ris;
}

double getTravelTime(double space){
    return space/SO_SPEED;
}