#ifndef _UTILITY_GOODS_H
#define _UTILITY_GOODS_H


/*this function returns a unity*/
goods generateGoods();

/*returns 1 if the good passed as parameter is expired, otherwise 0*/
int isExpired(goods good);

/*returns 1 if the good passed as parameter wuold expire: thif function compute the loading/unloading time and the travel time, it doesn't consider unexpected weather/*/
int willExpire(goods g, ship s, coordinates from ,coordinates to);

#endif /*_UTILITY_GOODS_H*/