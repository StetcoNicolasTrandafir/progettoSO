#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "macro.h"

#include "coordinates.h"

int main(){
    printf("NAVI: %s\n",SO_NAVI);
    printf("BANCHINE: %s\n\n",SO_BANCHINE);
    printf("PORTI: %s\n\n",SO_PORTI);

    struct coordinates coords= randomCoords();
    printCoords(coords);
}
