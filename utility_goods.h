#ifndef _UTILITY_GOODS_H
#define _UTILITY_GOODS_H

/*each goods is classified with these states:*/
enum states {in_port, on_ship, delivered, expired_port, expired_ship};

/*each unit of goods has a type (encoded by an int), a state, its dimension expressed ton (bewteen [0, SO_SIZE]),
a lifetime that expresses the number of days until its expiration (between [SO_MIN_VITA,SO_MAX_VITA]) and the time when it was generated*/
struct goods{
    int type;
    enum states state;
    int dimension;
    int lifeTime;
    double generationTime;
};

/*this function returns a unity*/
struct goods generateGoods();

/*returns 1 if the good passed as parameter is expired, otherwise 0*/
int isExpired(struct goods good);

#endif /*_UTILITY_GOODS_H*/