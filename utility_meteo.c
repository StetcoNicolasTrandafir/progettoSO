#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>       

#include "macro.h"
#include "types_module.h"
#include "utility_ship.h"
#include "utility_coordinates.h"
#include "utility_meteo.h"

struct timespec getStormDuration(){
    struct timespec t;
    int h;

    t.tv_sec= (int)(SO_STORM_DURATION/24);
    h=SO_STORM_DURATION - (t.tv_sec*24);
    t.tv_nsec=(h*100000000)/24;

    return t;
}

struct timespec getSwellDuration(){
    struct timespec t;
    int h;

    t.tv_sec= (int)(SO_SWELL_DURATION/24);
    h=SO_SWELL_DURATION - (t.tv_sec*24);
    t.tv_nsec=(h*100000000)/24;

    return t;
}