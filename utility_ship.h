#ifndef _UTILITY_SHIP_H
#define _UTILITY_SHIP_H


/*this function compute the time to cover the distance between the coords passed as parameter,
and make the nanosleep that simulate the travel*/
void move(ship s,coordinates from, coordinates to);

/*this function prints all the info about the ship passed as parameter needed for the daily report*/
/*void printShipRepo(ship ship);*/

/*computes and returns the actual position of the ship.
it takes a timespec parameter in order to stop/restart the nanosleep, if interrupted*/
coordinates getPosition(ship ship, struct timespec rem);

/*make the nanosleep for the time needed to load/unload goods from/to a port*/
void loadUnload(int quantity, struct timespec rem);


/*return the shared memory index of the port that is closest to the given coords.
the third parameter rapresent the starting distance (e.g. of min=3, the function returns the index of the nearest port with a minimum distance of 3)*/
int getNearestPort(struct port_sharedMemory * ports, coordinates coords, double min);


/*return an array containg all the pids of the ships that are using a dock in some port*/
pid_t * getShipsInPort(struct ship_sharedMemory *ships, coordinates portCoords);


/*returns an array containing the pids of the ships in movement*/
pid_t * getShipsInMovement(struct ship_sharedMemory * ships);

/*make all the process of negotiation of the goods: here the ship understand where move towards to load and transport goods*/
int negociate(int portsID, ship s, struct ship_sharedMemory *shared_ship, int shipIndex, int *expiredGood, int sem_expired_goods_id);


/*returns the index of port in the shared memory wich has a request that can be satisfied, -1 if there's no request satisfiable */
int getValidRequestPort(goods good, int msg_id, int shm_id);

void checkExpiredGoods(ship s, int goodsNumber, int *shippedGoods);


void badWeather(struct timespec duration);

#endif /*_UTILITY_SHIP_H*/
