#ifndef _UTILITY_COORDINATES_H
#define _UTILITY_COORDINATES_H

void printTest(int riga);
/*returns a couple of random coords, both x and y are beteen the range [0, SO_LATO]*/
coordinates getRandomCoords();

/*returns 1 if there is an entity in the space with range SO_DISTANZA centered in coord, 0 otherwise*/
int existCoords(coordinates coordv[], int idx, coordinates coord);

/*returns the distance between the point A and B, computed with pythagoras theorem*/
double getDistance(coordinates A, coordinates B);

/*returns the time (expressed in days) needed to travel the distance passed as paramter*/
double getTravelTime(double space);


#endif /*_UTILITY_COORDINATES_H*/