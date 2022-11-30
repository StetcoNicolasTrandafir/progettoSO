#ifndef _COORDINATES_H
#define _COORDINATES_H


struct coordinates{
    double x;
    double y;
};

/*returns a couple of random coords, both x and y are beteen the range [0, SO_LATO]*/
struct coordinates randomCoords();

/*print the coords passed as paramter in the format "X: coords.x - Y: coords.y"*/
void printCoords(struct coordinates coord);


#endif /*_COORDINATES_H*/