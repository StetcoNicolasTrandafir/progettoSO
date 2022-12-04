#ifndef _UTILITY_PORT_H
#define _UTILITY_PORT_H

/*the request struct is made up by the goods' type and a flag to understand if it has already been satisfied or not*/
struct request{
    int satified;
    int goodsType;
};

/*each port is characterized by its coords (unique), its number of docks (number between [1, SO_BANCHINE]),
an array for the goods request and another one for the offers*/
struct port{
    struct coordinates coord;
    int docks;
    struct request* requests;

    //REVIEW: potremmo salvare in questo array solo l'identificativo della merce?
    struct goods* generatedGoods;
};

/*this function prints all the info about the port parameter needed for the daily report*/
void printPortRepo(struct port port);

#endif /*_UTILITY_PORT_H*/