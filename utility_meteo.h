#ifndef _UTILITY_METEO_H
#define _UTILITY_METEO_H


/*it sinks the ship with the pid passed as parameter, terminating its the process*/
void mealstrom(struct ship_sharedMemory * ships);

/*stops the movement of a random ship*/
void storm(struct ship_sharedMemory * ships);

/*stops the loading/downloading activity of a random port */
void swell(struct port_sharedMemory * ports, struct ship_sharedMemory * ships);

/*returns a struct timespec that rapresents the cadence of the swells (based on SO_MEALSTORM and the actual duration of an hour)*/
struct itimerval getMealstromQuantum();

struct timespec getSwellDuration();

struct timespec getStormDuration();

#endif /*_UTILITY_METEO_H*/