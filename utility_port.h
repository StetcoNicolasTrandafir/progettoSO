#ifndef _UTILITY_PORT_H
#define _UTILITY_PORT_H

/*the request struct is made up by the goods' type and a flag to understand if it has already been satisfied or not*/
struct request{
    int satisfied;
    int goodsType;
    int quantity;
};

/*each port is characterized by its coords (unique), its number of docks (number between [1, SO_BANCHINE]),
an array for the goods request and another one for the offers*/
typedef struct port{
    coordinates coord;
    int docks;
    struct request* requests;
    
    /*REVIEW: potremmo salvare in questo array solo l'identificativo della merce?*/
    goods * generatedGoods;
}port;

/*returns the amount (ton) of the port passed as parameter requests.
if the satisfied flag is set to ONLY_SATISFIED (1) computes only the satisfied requests, 
if is set to ALL (0) returns the total amount of the requests 
*/
int getRequests(port port, int satified);

/*compute the amount (ton) of goods generated, according with the flag value the function returns:
-0 (macro: IN_PORT)->  the genereted goods present in the port
-1 (macro: SHIPPED) -> the goods generated and shipped
-2 (macro: ALL) -> all the generated goods
*/
int getGeneratedGoods(port port, int flag);

/*this function prints all the info about the port passed as parameter needed for the daily report*/
void printPortRepo(port port);


#endif /*_UTILITY_PORT_H*/