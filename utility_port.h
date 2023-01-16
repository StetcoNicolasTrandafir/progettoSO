#ifndef _UTILITY_PORT_H
#define _UTILITY_PORT_H



struct msg_request {
    long mtype;
    int idx;
};

/*generate a request and update the relative shared memory, returns the type of the good requested, -1 if it isn't possible*/
void generateRequest(port port, int sum_requestID, int sem_sum_id);

/*generate an offer and update the relative shared memory, returns the type of the good generated, -1 if it isn't possible*/
int generateOffer(port port, int idx, int sum_offerID, int sem_offer_id);

/*return 1 if the goods type passed as parameter is already offered in the port, 0 otherwhise*/
int isOffered(port port, int goodsType);

/*return 1 if the goods type passed as parameter is already requested in the port, 0 otherwhise*/
int isRequested(port port, int goodsType);

void updateGoods(port port, int semID);

#endif /*_UTILITY_PORT_H*/