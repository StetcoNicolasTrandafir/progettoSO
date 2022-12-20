#ifndef _UTILITY_PORT_H
#define _UTILITY_PORT_H


/*set goodstype of each element of generatedGoods and requests to -1 in order to be checkable in other controls, it also creates the shared memory for the offers*/
void initializeRequestsAndOffer(port p);

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
void generateRequest(port port);

/*generate an offer and update the relative shared memory, returns the type of the good generated, -1 if it isn't possible*/
void generateOffer(port port, int idx);

/*return 1 if the goods type passed as parameter is already offered in the port, 0 otherwhise*/
int isOffered(port port, int goodsType);

/*return 1 if the goods type passed as parameter is already requested in the port, 0 otherwhise*/
int isRequested(port port, int goodsType);

#endif /*_UTILITY_PORT_H*/