#ifndef _TYPES_MODULE_H
#define _TYPES_MODULE_H


typedef struct coordinates{
    double x;
    double y;
}coordinates;



/*each goods is classified with these states:*/
enum states {in_port, on_ship, delivered, expired_port, expired_ship};


/*each unit of goods has a type (encoded by an int), a state, its dimension expressed ton (bewteen [0, SO_SIZE]),
a lifetime that expresses the number of days until its expiration (between [SO_MIN_VITA,SO_MAX_VITA]) and the time when it was generated*/
typedef struct goods{
    int type;
    enum states state;
    int dimension;
    int satisfied;
    int lifeTime;
    struct timespec generationTime;
}goods;

/*each port is characterized by its coords (unique), its number of docks (number between [1, SO_BANCHINE]),
an array for the goods request and another one for the offers*/
typedef struct port {
    coordinates coord;
    int docks;
    goods *generatedGoods;
    struct request *request; 
}port;


/*the request struct is made up by the goods' type and a flag to understand if it has already been satisfied or not*/
struct request {
    int satisfied;
    int booked;
    int goodsType;  
    int quantity;
};


/*each ship needs to keep track of its coordinates and the goods that is carrying on, furthermore a flag determinats if the ship using a dock*/
typedef struct ship{
    coordinates coords; 
    goods *goods; 
    int inDock; 
}ship;


/*this struct contains the coords of the port and the pid, needed to access the port's generatedGoods IPC*/
struct port_sharedMemory {
    coordinates coords;
    int pid;
    int offersID;
};

struct msg_request {
    long mtype;
    int idx;
};



#endif
