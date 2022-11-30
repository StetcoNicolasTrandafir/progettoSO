#ifndef _PORT_H
#define _PORT_H

/*the request struct is made up by the goods' type and a flag to understand if it has already been satisfied or not*/
struct request{
    int satified;
    int goodsType;
}

/*each port is characterized by its coords (unique), its number of docks (number between [1, SO_BANCHINE]),
an array for the goods request and another one for the offers*/
struct port{
    struct coordinates coords;
    int docks;
    struct request requests[SO_DAYS];

    //REVIEW: potremmo salvare in questo array solo l'identificativo della merce?
    struct goods generatedGoods[SO_DAYS];
}



#endif /*_PORT_H*/