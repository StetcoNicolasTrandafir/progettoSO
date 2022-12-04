#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "macro.h"
#include "utility_coordinates.h"
#include "utility_port.h"

void printPortRepo(struct port port){
    printf("Port in position(%2f,%2f):\n", port.coord.x, port.coord.y);
}