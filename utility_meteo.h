#ifndef _UTILITY_METEO_H
#define _UTILITY_METEO_H


/*it sinks the ship with the pid passed as parameter, terminating its the process*/
void mealstrom(int shipPid);

/*stops the movement of the ship with the pid passed as parameter*/
void storm(int shipPid);

/*stops the loading/downloading activity of the port with the pid passed as parameter*/
void swell(int portPid);

/*returns a struct timespec that rapresents the cadence of the swells (based on SO_MEALSTORM and the actual duration of an hour)*/
struct timespec getMealstromQuantum();

/*returns an array that contains the pid of all the ships in movement*/
pid_t [] getShipsInMovement(struct shared_ship ships);

#endif /*_UTILITY_METEO_H*/