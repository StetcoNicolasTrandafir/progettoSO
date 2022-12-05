#ifndef _UTILITY_SHIP_H
#define _UTILITY_SHIP_H

typedef struct ship{
    coordinates coords;
    goods *goods;
};


/*this function compute the time to cover the distance between the coords passed as parameter,
and make the nanosleep that simulate the travel*/
void move(coordinates from, coordinates to, struct timespec rem);

/*this function prints all the info about the ship passed as parameter needed for the daily report*/
/*void printShipRepo(ship ship);*/


/*computes and returns the actual position of the ship.
it takes a timespec parameter in order to stop/restart the nanosleep, if interrupted*/
coordinates getPosition(ship ship, struct timespec rem);

/*make the nanosleep for the time needed to load/unload goods from/to a port*/
void loadUnload(goods goods, struct timespec rem);



#endif /*_UTILITY_SHIP_H*/
