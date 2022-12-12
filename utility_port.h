#ifndef _UTILITY_PORT_H
#define _UTILITY_PORT_H

/*the request struct is made up by the goods' type and a flag to understand if it has already been satisfied or not*/
struct request{
    int satisfied;
    int goodsType;
    int quantity;
};

/*this struct contains the coords of the port and the pid, needed to access the port's generatedGoods IPC*/
struct port_sharedMemory{
    coordinates coords;
    int pid;
    /*TODO aggiungere puntatore a struct che mi contiene tutte le info del singolo porto di cui gli altri processi hanno bisogno*/
};

struct msg_request {
    int mtype;
    int quantity;
    int idx;
};

/*
NEGOZIAZIONE NAVI-PORTI:
1) verifico i porti in ordine di vicinanza rispetto alla nave
2) controllo le richieste del porto
3) per ogni offerta, controllo se c'è un porto (sempre in ordine di prossimità) che fa richiesta di quel bene e se ci arrivo (per la scadenza)
4) se c'è una richiesta, segno come impegnate tot merci sia nella richiesta che nell'offerta
5) se non c'è nessuna richiesta, considero il prossimo porto più vicino
6) se non c'è nessun porto che richiede una determinata merce di un offerta, considero la prossima offerta del porto
7) se non ci sono richieste per nessuna offerta del porto preso in considerazione, prendo in considerazione il seguente porto
8) se non ci sono offerte e richieste che la nave può soddisfare in nessun porto, mi muovo verso il porto più vicino aspettando la generazione del giorno dopo
*/


/*each port is characterized by its coords (unique), its number of docks (number between [1, SO_BANCHINE]),
an array for the goods request and another one for the offers*/


typedef struct port{
    coordinates coord;
    int docks;
    goods *generatedGoods;
    struct request request;
    
}port;


/*set goodstype of each element of generatedGoods and requests to -1 in order to be checkable in other controls */
port initializeRequestsAndOffer(port p);

/*returns the amount (ton) of the port passed as parameter requests.
if the satisfied flag is set to ONLY_SATISFIED (1) computes only the satisfied requests, 
if is set to ALL (0) returns the total amount of the requests 
*/
int getRequest(port port, int satified);

/*compute the amount (ton) of goods generated, according with the flag value the function returns:
-0 (macro: IN_PORT)->  the genereted goods present in the port
-1 (macro: SHIPPED) -> the goods generated and shipped
-2 (macro: ALL) -> all the generated goods
*/
int getGeneratedGoods(port port, int flag);

/*prints all the info needed for the daily report*/
void printDailyReport(port port);

/*generate a request and update the relative shared memory, returns the type of the good requested, -1 if it isn't possible*/
struct request generateRequest(port port);

/*generate an offer and update the relative shared memory, returns the type of the good generated, -1 if it isn't possible*/
int generateOffer(port port, int day);

/*return 1 if the goods type passed as parameter is already offered in the port, 0 otherwhise*/
int isOffered(port port, int goodsType);

/*return 1 if the goods type passed as parameter is already requested in the port, 0 otherwhise*/
int isRequested(port port, int goodsType);

#endif /*_UTILITY_PORT_H*/