#ifndef _COORDINATES_H
#define _COORDINATES_H


struct coordinates{
    double x;
    double y;
};

/*returns a couple of random coords, both x and y are beteen the range [0, SO_LATO]*/
struct coordinates getRandomCoords();

/*returns 1 if there is an entity in the space with range SO_DISTANZA centered in coord, 0 otherwise*/
int existCoords(struct coordinates coordv[], int idx, struct coordinates coord);

/*print the coords passed as paramter in the format "X: coords.x - Y: coords.y"*/
void printCoords(struct coordinates coord);

/*returns the distance between the point A and B, computed with pythagoras theorem*/
double getDistance(struct coordinates A, struct coordinates B);

/*returns the time (expressed in days) needed to travel the distance passed as paramter*/
double getTravelTime(double space);


#endif /*_COORDINATES_H*/