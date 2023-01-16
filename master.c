#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>	
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "macro.h"
#include "semaphore_library.h"
#include "types_module.h"
#include "utility_coordinates.h"
#include "utility_port.h"

int pastDays = 0, *sum_request, *sum_offer;
int sem_sync_id, sem_sum_id;
int port_sharedMemoryID, sum_requestID, sum_offerID;
int msg_id;
struct port_sharedMemory *sharedPortPositions;
pid_t meteoPid;
struct ship_sharedMemory *shared_ship;
int shipSharedMemoryID;
int expiredGoodsSharedMemoryID;
int *expiredGoods;
int sem_expired_goods_id;

void killChildren(){
	int i;
	kill(meteoPid, SIGINT); TEST_ERROR;
	for(i=0; i< SO_NAVI; i++){
		if (shared_ship[i].sinked != 1) {
			kill(shared_ship[i].pid, SIGINT); TEST_ERROR;
		}
	}
	for(i=0; i< SO_PORTI; i++) {
		kill(sharedPortPositions[i].pid, SIGINT); TEST_ERROR;
	}
}

void cleanUp(){
	struct sembuf sops;
	int i;
	waitForZero(sops, sem_sync_id,1); TEST_ERROR;
	for(i = 0; i < SO_NAVI; i++) {
		semctl(shared_ship[i].semID, 0, IPC_RMID); TEST_ERROR;
		shmctl(shared_ship[i].goodsID, IPC_RMID, NULL); TEST_ERROR;
	}

	shmdt(sharedPortPositions); TEST_ERROR;
	shmdt(sum_request); TEST_ERROR;
	shmdt(shared_ship);TEST_ERROR;
	shmdt(expiredGoods);TEST_ERROR;
	shmdt(sum_offer); TEST_ERROR; 	
	semctl(sem_sync_id, 0, IPC_RMID); TEST_ERROR;
	semctl(sem_sum_id, 0, IPC_RMID); TEST_ERROR;
	semctl(sem_expired_goods_id, 0, IPC_RMID); TEST_ERROR;
	msgctl(msg_id, IPC_RMID, NULL); TEST_ERROR;
}

void finalReport(){
	int i, j;
	int maxRequestPortIndex=-1, maxRequest=0;
	int maxOfferPortIndex=-1;
	int *goodsStateSum;
	int totalGoodsSum=0;
	int inPortGoods=0;
	int shippedGoods=0;
	int *offerSum;
	struct goodsTypeReport *goodsReport;
	goods *g;
	enum states state;
	struct request *r;
	char *string;
	int numBytes;
	struct sembuf sops;
	int sinked=0;
	int busyDocks=0;
	int dischargedShips=0;
	int chargedShips=0;
	int swellPort=0;
	int stormed=0;
	bzero(&sops, sizeof(struct sembuf));

	goodsReport=calloc(SO_MERCI, sizeof(struct goodsTypeReport));
	offerSum=calloc(SO_MERCI, sizeof(int));
	goodsStateSum=calloc(5, sizeof(int));

	bzero(goodsReport, SO_MERCI*sizeof(struct goodsTypeReport));
	bzero(goodsStateSum, 5*sizeof(int));


	string=malloc(200);
	numBytes=sprintf(string,"\n\n========================================================================\n\t\tREPORT FINALE:\n========================================================================\n\n\n==================>\t\tPORTI\n");

	fflush(stdout);
	write(1, string, numBytes);
	string=realloc(string,150);
	for(i=0; i<SO_PORTI; i++){
		g=shmat(sharedPortPositions[i].offersID, NULL, 0);
		r=shmat(sharedPortPositions[i].requestID, NULL, 0);

		bzero(offerSum, SO_MERCI * sizeof(int));

		inPortGoods=0;
		shippedGoods=0;
		
		
		for(j=0; j<SO_FILL && g[j].type!=0; j++){

			totalGoodsSum+=g[j].dimension;
			offerSum[(g[j].type-1)]+=g[j].dimension;
			goodsReport[(g[j].type-1)].totalSum+=g[j].dimension;

			if(g[j].state==in_port){
				goodsStateSum[in_port]+=g[j].dimension;
				goodsReport[(g[j].type-1)].inPort+=g[j].dimension;
				inPortGoods+=g[j].dimension;
			} else if(g[j].state==expired_port){
				goodsReport[(g[j].type-1)].expiredInPort+=g[j].dimension;
				goodsStateSum[expired_port]+=g[j].dimension;
			}else if(g[j].state==on_ship){
				shippedGoods+=g[j].dimension;
			}
		}

        decreaseSem(sops, sharedPortPositions[i].semID, REQUEST);
		goodsReport[(r->goodsType-1)].delivered+=r->satisfied;
		goodsStateSum[delivered]+=r->satisfied;
        increaseSem(sops, sharedPortPositions[i].semID, REQUEST);

		for(j=0; j<SO_MERCI; j++){

			if(goodsReport[j].maxOffer<offerSum[j]){
				goodsReport[j].maxOffer=offerSum[j];
				goodsReport[j].maxOfferPortIndex=i;
			}
		}
		

		decreaseSem(sops, sharedPortPositions[i].semID, REQUEST);
		if(goodsReport[(r->goodsType-1)].maxRequest < r->quantity){
			goodsReport[(r->goodsType-1)].maxRequest = r->quantity;
			goodsReport[(r->goodsType-1)].maxRequestPortIndex = i;
		}
		increaseSem(sops, sharedPortPositions[i].semID, REQUEST);


		
		decreaseSem(sops, sharedPortPositions[i].semID, REQUEST);
		decreaseSem(sops, sharedPortPositions[i].semID, SWELL);
		if(sharedPortPositions[i].swell) swellPort++;

		numBytes=sprintf(string,"\n\nPORTO[%d] NUMERO %d (%.2f,%.2f):\nMerce in porto:%dton\nMerce spedita:%dton\nMerce ricevuta:%dton\nPorto colpito da mareggiata %d volte\n",  sharedPortPositions[i].pid,(i+1),sharedPortPositions[i].coords.x,sharedPortPositions[i].coords.y, inPortGoods,shippedGoods,r-> satisfied, sharedPortPositions[i].swell);
		increaseSem(sops, sharedPortPositions[i].semID, SWELL);
		increaseSem(sops, sharedPortPositions[i].semID, REQUEST);

		fflush(stdout);
		write(1, string, numBytes);

		shmdt(g);
		shmdt(r);
		
	}
	free(offerSum);

	for(i=0; i< SO_NAVI; i++){

		
		if(shared_ship[i].sinked!=1){

			decreaseSem(sops,shared_ship[i].semID, STORM);
			if(shared_ship[i].storm) stormed++;
			increaseSem(sops,shared_ship[i].semID, STORM);
			
			decreaseSem(sops,shared_ship[i].semID, INDOCK);
			if(shared_ship[i].inDock) {
				busyDocks++;
				increaseSem(sops,shared_ship[i].semID, INDOCK);
			}

			else {
				increaseSem(sops,shared_ship[i].semID, INDOCK);
				j=0;
				g=shmat(shared_ship[i].goodsID, NULL, 0); TEST_ERROR;
				if(g[0].type!=0)
					chargedShips++;
				else 
					dischargedShips++;
				}

			while(g[j].type!=0 && j< SO_CAPACITY){
				decreaseSem(sops,shared_ship[i].semID, GOODS);
				if(g[j].state==on_ship && isExpired(g[j])){
					g[j].state=expired_ship;
					decreaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;
					expiredGoods[g[j].type-1]=g[j].dimension;
					increaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;
				}else{
					goodsStateSum[on_ship]+=g[j].dimension;
				}
				increaseSem(sops,shared_ship[i].semID, GOODS);
				j++;
			}
		}else 
			sinked++;
	}


	decreaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;
	for(i=0; i<SO_MERCI; i++){
		goodsStateSum[expired_ship]+=expiredGoods[i];
	}
	increaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;


	string=realloc(string,187);TEST_ERROR;
	numBytes=sprintf(string,"\n\n ==================>\t\tNAVI\n\nNavi affondate: %d\nNumero di navi facendo operazioni di carico/scarico: %d\nIn mare con un carico a bordo: %d\nIn mare senza carico a bordo: %d",sinked, busyDocks,chargedShips, dischargedShips);
	fflush(stdout);
	write(1, string, numBytes);

	string=realloc(string,65);
	numBytes=sprintf(string,"\n\n ==================>\t\tREPORT MERCI\n\n---------->\tPER STATI:");
	fflush(stdout);
	write(1, string, numBytes);

	string=realloc(string,260);
	numBytes=sprintf(string,"\n\nMerce in porto (disponibile): %dton\nMerce scaduta in porto: %dton\nMerce consegnata: %dton\nMerce in nave: %dton\nMerce scaduta in nave: %dton\n\n---------->\tPER TIPOLOGIA:\n\n\nTOTALE MERCE GENERATA: %dton",goodsStateSum[in_port],goodsStateSum[expired_port],goodsStateSum[delivered],goodsStateSum[on_ship],goodsStateSum[expired_ship],totalGoodsSum);
	fflush(stdout);
	write(1, string, numBytes);

	string=realloc(string,404);

	for(i=0; i<SO_MERCI; i++){
		numBytes=sprintf(string,"\nMERCE DI TIPO %d:\nMerce generata: %dton\nMerce ferma in porto: %dton\nMerce scaduta in porto: %dton\nMerce scaduta in nave: %dton\nMerce consegnata: %d\nIl porto che ne ha fatto più richiesta è il numero %d (%dton)\nIl porto che ne ha generato di più è %d (%dton)\n",(i+1), goodsReport[i].totalSum, goodsReport[i].inPort, goodsReport[i].expiredInPort, expiredGoods[i], goodsReport[i].delivered, goodsReport[i].maxRequestPortIndex, goodsReport[i].maxRequest, goodsReport[i].maxOfferPortIndex, goodsReport[i].maxOffer);
		fflush(stdout);
		write(1, string, numBytes);
	}



	string=realloc(string,143);
	numBytes=sprintf(string,"\n\n ==================>\t\tREPORT METEO\n\nNavi affondate dal vortice: %d\nPorti colpiti da mareggiata: %d\nNavi rallentate dalla tempesta: %d", sinked, swellPort,stormed);
	fflush(stdout);
	write(1, string, numBytes);


	free(goodsReport);
	free(goodsStateSum);

	free(string);
}

int elementInArray(int element, int array[], int limit) {
	int i;
	
	for (i = 0; i < limit; i++) {
		if (array[i] == element) {
			return 1;
		}
	}
	return 0;
}

void dailyReport(){
	int i,j = 0;
	int  freeDocks = 0;
	char *string;
	int numBytes;
	goods *g;
	struct request *r;
	int *stateSum;
    int *typeSum;
	int inPort;
	int shipped;
	int busyDocks=0;
	int chargedShips=0;
	int dischargedShips=0;
	struct sembuf sops;
	int totalRequest=0;
	int totalOffer=0;
	int sinked=0;
	int stormed=0;
	int swellPort=0;
	bzero(&sops, sizeof(struct sembuf));

	typeSum=calloc(SO_MERCI, sizeof(int));TEST_ERROR;
	stateSum=calloc(5, sizeof(int));TEST_ERROR;

	bzero(typeSum, SO_MERCI*sizeof(int));
	bzero(stateSum, 5*sizeof(int));

	string=malloc(220);TEST_ERROR;
	numBytes=sprintf(string,"\n\n========================================================================\n\t\tREPORT GIORNO %d:\n========================================================================\n\n\n==================>\t\tPORTI\n", pastDays);
	TEST_ERROR;

	fflush(stdout);
	write(1, string, numBytes);

	for(i=0; i< SO_PORTI; i++){
		g=shmat(sharedPortPositions[i].offersID, NULL, 0); TEST_ERROR;
		r=shmat(sharedPortPositions[i].requestID, NULL, 0); TEST_ERROR;
		inPort=0;
		shipped=0;

		while(j<SO_FILL && g[j].type!=0){
			if(g[j].state==in_port){
				totalOffer+=g[j].dimension;
				stateSum[in_port]+=g[j].dimension;
				inPort+=g[j].dimension;
			}else 
			if(g[j].state==on_ship){
				shipped+=g[j].dimension;
			}else 
			if(g[j].state==expired_port){
				stateSum[expired_port]+=g[j].dimension;
			}
			
			typeSum[g[j].type-1]+=g[j].dimension;
			
			j++;
		}

		freeDocks= semctl(sharedPortPositions[i].semID, 0, GETVAL);TEST_ERROR;

        decreaseSem(sops, sharedPortPositions[i].semID, REQUEST);
		totalRequest+=r->quantity-r->satisfied;
		stateSum[delivered]+=r->satisfied;

		decreaseSem(sops, sharedPortPositions[i].semID, SWELL);
		if(sharedPortPositions[i].swell) swellPort++;

		numBytes = sprintf(string, "Porto numero %d [%d] in posizione: (%.2f, %.2f)\nBanchine libere %d su %d\nMerci spedite: %d ton\nMerci generate ancora in porto: %d ton\nMerci ricevute: %d ton\nPorto colpito da mareggiata %d volte\n\n", (i+1), sharedPortPositions[i].pid, sharedPortPositions[i].coords.x, sharedPortPositions[i].coords.y, freeDocks, sharedPortPositions[i].docks, shipped, inPort, r->satisfied,sharedPortPositions[i].swell);
		increaseSem(sops, sharedPortPositions[i].semID, SWELL);
		increaseSem(sops,sharedPortPositions[i].semID, REQUEST);




		fflush(stdout);
		write(1, string, numBytes);

		shmdt(g);
		shmdt(r);
	}


	for(i=0; i< SO_NAVI; i++){

		if(shared_ship[i].sinked!=1){
			if(shared_ship[i].storm) stormed++;

			if(shared_ship[i].inDock){
				busyDocks++; 
			}
			else {
				j=0;
				g=shmat(shared_ship[i].goodsID, NULL, 0); TEST_ERROR;
				if(g[0].type!=0)
					chargedShips++;
				else 
					dischargedShips++;
				}

			while(g[j].type!=0 && j< SO_CAPACITY){
				decreaseSem(sops,shared_ship[i].semID, GOODS);
				if(g[j].state==on_ship && isExpired(g[j])){
					g[j].state=expired_ship;
					decreaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;
					expiredGoods[g[j].type-1]=g[j].dimension;
					increaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;
				}else{
					stateSum[on_ship]+=g[j].dimension;
				}
				increaseSem(sops,shared_ship[i].semID, GOODS);
				j++;
			}
		}else 
			sinked++;
	}


	decreaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;
	for(i=0; i<SO_MERCI; i++){
		stateSum[expired_ship]+=expiredGoods[i];
	}
	increaseSem(sops, sem_expired_goods_id, 0); TEST_ERROR;



	string=realloc(string,200);TEST_ERROR;
	numBytes=sprintf(string,"\n\n ==================>\t\tMERCI\n\n---------->\tPER STATI:\n\nMerce in porto: %dton\nMerce scaduta in porto: %dton\nMerce in nave: %dton\nMerce scaduta in nave: %d\nMerce consegnata: %dton", stateSum[in_port],stateSum[expired_port],stateSum[on_ship],stateSum[expired_ship],stateSum[delivered]);
	fflush(stdout);
	write(1, string, numBytes);


	string=realloc(string,33);TEST_ERROR;
	numBytes=sprintf(string,"\n\n---------->\tPER TIPOLOGIA:\n");
	fflush(stdout);
	write(1, string, numBytes);
	
	string=realloc(string,30);TEST_ERROR;

	for(i=0; i<SO_MERCI; i++){
		numBytes=sprintf(string,"\nMerce di tipo %d: %dton", (i+1), typeSum[i]);
		fflush(stdout);
		write(1, string, numBytes);
	}

	string=realloc(string,187);TEST_ERROR;
	numBytes=sprintf(string,"\n\n ==================>\t\tNAVI\n\nNavi affondate: %d\nNumero di navi facendo operazioni di carico/scarico: %d\nIn mare con un carico a bordo: %d\nIn mare senza carico a bordo: %d",sinked, busyDocks,chargedShips, dischargedShips);
	fflush(stdout);
	write(1, string, numBytes);


	string=realloc(string,143);
	numBytes=sprintf(string,"\n\n ==================>\t\tREPORT METEO\n\nNavi affondate dal vortice: %d\nPorti colpiti da mareggiata: %d\nNavi rallentate dalla tempesta: %d", sinked, swellPort,stormed);
	fflush(stdout);
	write(1, string, numBytes);


	string=realloc(string,89);TEST_ERROR;
	numBytes=sprintf(string,"\n\n============================= FINE REPORT GIORNO %d =============================\n\n", pastDays);
	fflush(stdout);
	write(1, string, numBytes);TEST_ERROR;
	

	if(sinked==SO_NAVI){
		finalReport();

		string=realloc(string,65);TEST_ERROR;
		numBytes=sprintf(string,"\n\nTUTTE LE NAVI SONO STATE AFFONDATE: SIMULAZIONE FINITA!\n");
		fflush(stdout);
		write(1, string, numBytes);TEST_ERROR;
		
		
		killChildren();
		cleanUp();
		exit(EXIT_SUCCESS);
	}else if(totalOffer==0 && chargedShips==0){

		finalReport();

		string=realloc(string,65);TEST_ERROR;
		numBytes=sprintf(string,"\n\nNON C'È PIÙ OFFERTA DI ALCUNA MERCE: SIMULAZIONE FINITA!\n");
		fflush(stdout);
		write(1, string, numBytes);TEST_ERROR;
		
		killChildren();
		cleanUp();
		exit(EXIT_SUCCESS);
	}else if(totalRequest==0){
		finalReport();

		string=realloc(string,60);TEST_ERROR;
		numBytes=sprintf(string,"\n\nNON C'È PIÙ RICHIESTA DI ALCUNA MERCE: SIMULAZIONE FINITA!\n");
		fflush(stdout);
		write(1, string, numBytes);TEST_ERROR;
		killChildren();
		cleanUp();
		exit(EXIT_SUCCESS);
	}


    free(string); TEST_ERROR;
	free(typeSum);TEST_ERROR;
	free(stateSum);TEST_ERROR;
}

void sendSignalToCasualPorts(){
	int i, n_ports, *port_idx, casual_idx;
	struct timespec now;
	struct sembuf sops;

	bzero(&sops, sizeof(struct sembuf));

    clock_gettime(CLOCK_REALTIME, &now);
    n_ports = (now.tv_nsec % SO_PORTI) + 1;
    port_idx = calloc(n_ports, sizeof(int));

	*sum_offer = n_ports;

    for (i = 0; i < n_ports; i++) {
    	do {
    		clock_gettime(CLOCK_REALTIME, &now);
    		casual_idx = now.tv_nsec % SO_PORTI;
    	}
    	while (elementInArray(casual_idx, port_idx, i));
		port_idx[i] = casual_idx;
    	kill(sharedPortPositions[port_idx[i]].pid, SIGUSR1); TEST_ERROR;
    }
    free(port_idx);
}

void sendDailySignal() {
	int i;
	for (i = 0; i < SO_PORTI; i++) {
		kill(sharedPortPositions[i].pid, SIGALRM); TEST_ERROR;
	}
	for(i = 0; i < SO_NAVI; i++) {
		if (shared_ship[i].sinked != 1) {
			kill(shared_ship[i].pid, SIGALRM); TEST_ERROR;
		}
	}

	kill(meteoPid, SIGUSR2); TEST_ERROR;
}



void handleSignal(int signal) {
	char *string;
	int numBytes;
	int prevErrno=errno;
	errno=0;

	switch(signal) {
		case SIGALRM:
			sendDailySignal();
			if(++pastDays==SO_DAYS){
				finalReport();
				killChildren();

			}else{
				sendSignalToCasualPorts();
				dailyReport();
				alarm(1);
			}
			break;

		case SIGINT:

			string=malloc(85);
			numBytes=sprintf(string,"\n\n\nINTERRUZIONE INASPETTATA DEL PROGRAMMA\nPulizia processi figli, IPCs, etc. ...\n");
			fflush(stdout);
			write(1, string, numBytes);

			killChildren();
			cleanUp();
			string=realloc(string,8);
			numBytes=sprintf(string,"Done!\n\n");
			fflush(stdout);
			write(1, string, numBytes);
	
			exit(EXIT_SUCCESS);
			break;
	}
	errno = prevErrno;
}

int main() {
	sigset_t set;
	struct sigaction sa;
	int i, j;
	char  *args[10], name_file[100], sem_sync_str[3 * sizeof(int) + 1], expiredGoods_STR[3 * sizeof(int) + 1],i_str[3 * sizeof(int) + 1], port_sharedMemoryID_STR[3*sizeof(int)+1], sum_requestID_STR[3 * sizeof(int) + 1], sem_request_str[3 * sizeof(int) + 1], msg_str[3 * sizeof(int) + 1], sum_offerID_STR[3 * sizeof(int) + 1], sem_expired_goods_str[3 * sizeof(int) + 1];
	pid_t fork_rst;
	char  shipSharedMemory_str[3 * sizeof(int) + 1];
	struct sembuf sops;
	goods *g;
	int idRequest;
	struct 	request *r;
	char *string;
	int numBytes;

	shipSharedMemoryID=shmget(IPC_PRIVATE, SO_NAVI*sizeof(struct ship_sharedMemory), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	shared_ship = shmat(shipSharedMemoryID, NULL, 0); TEST_ERROR;
	shmctl(shipSharedMemoryID, IPC_RMID, NULL); TEST_ERROR;

	expiredGoodsSharedMemoryID= shmget(IPC_PRIVATE, SO_MERCI*sizeof(int),S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	expiredGoods=shmat(expiredGoodsSharedMemoryID, NULL, 0);TEST_ERROR;
	shmctl(expiredGoodsSharedMemoryID, IPC_RMID, NULL);	TEST_ERROR;
	bzero(expiredGoods,SO_MERCI*sizeof(int));

	sharedPortPositions = calloc(SO_PORTI, sizeof(struct port_sharedMemory));
	sum_request = malloc(sizeof(int));

	bzero(&sharedPortPositions, sizeof(sharedPortPositions));
	bzero(&sa, sizeof(sa));
	bzero(&sops, sizeof(sops));
	
	sa.sa_handler = handleSignal;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	sem_sync_id = semget(IPC_PRIVATE, 4, 0600); TEST_ERROR;
	semctl(sem_sync_id, 0, SETVAL, SO_PORTI + SO_NAVI + 1); TEST_ERROR;
	semctl(sem_sync_id, 1, SETVAL, SO_PORTI); TEST_ERROR;
	semctl(sem_sync_id, 2, SETVAL, SO_NAVI); TEST_ERROR;
	semctl(sem_sync_id, 3, SETVAL, 1); TEST_ERROR;

	sem_sum_id = semget(IPC_PRIVATE, 2, 0600); TEST_ERROR;
	semctl(sem_sum_id, 0, SETVAL, 1); TEST_ERROR; 
	semctl(sem_sum_id, 1, SETVAL, SO_PORTI); TEST_ERROR;

	sem_expired_goods_id = semget(IPC_PRIVATE, 1, 0600); TEST_ERROR;
	semctl(sem_expired_goods_id, 0, SETVAL, 1); TEST_ERROR; 

	port_sharedMemoryID=shmget(IPC_PRIVATE, SO_PORTI*sizeof(struct port_sharedMemory),S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	sharedPortPositions=shmat(port_sharedMemoryID, NULL, 0); TEST_ERROR;
	shmctl(port_sharedMemoryID, IPC_RMID, NULL); TEST_ERROR;

	sum_requestID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	sum_request = shmat(sum_requestID, NULL, 0); TEST_ERROR;
	*sum_request = 0;
	shmctl(sum_requestID, IPC_RMID, NULL); TEST_ERROR;

	sum_offerID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	sum_offer = shmat(sum_offerID, NULL, 0); TEST_ERROR;
	*sum_offer = SO_PORTI;
	shmctl(sum_offerID, IPC_RMID, NULL); TEST_ERROR;

	msg_id = msgget(getpid(), IPC_CREAT | IPC_EXCL | 0600); TEST_ERROR;

	sprintf(name_file, "port");
	sprintf(sem_sync_str, "%d", sem_sync_id);
	sprintf(port_sharedMemoryID_STR, "%d", port_sharedMemoryID);
	sprintf(sum_requestID_STR, "%d", sum_requestID);
	sprintf(sem_request_str, "%d", sem_sum_id);
	sprintf(msg_str, "%d", msg_id);
	sprintf(sum_offerID_STR, "%d", sum_offerID);
	sprintf(expiredGoods_STR, "%d", expiredGoodsSharedMemoryID);	

	args[0] = name_file;
	args[1] = sem_sync_str;
	args[2] = port_sharedMemoryID_STR;
	args[3] = sum_requestID_STR;
	args[4] = sem_request_str;
	args[5] = msg_str;
	args[7] = sum_offerID_STR;
	args[8] = NULL;

	for (i = 0; i < SO_PORTI; i++) {
		switch(fork_rst = fork()) {
			case -1:
				TEST_ERROR;
				exit(1);

			case 0: 
				sprintf(i_str, "%d", i);
				args[6] = i_str;
				
				execv("./port", args);
				TEST_ERROR;
				exit(EXIT_FAILURE);
				
		}
	}

	sprintf(name_file, "ship");
	sprintf(shipSharedMemory_str, "%d",shipSharedMemoryID);
	sprintf(sem_expired_goods_str, "%d", sem_expired_goods_id);
	args[0] = name_file;
	args[3]=shipSharedMemory_str;
	args[5] = expiredGoods_STR;
	args[6] = sem_expired_goods_str;
	args[7] = NULL;
	
	
	for (i = 0; i < SO_NAVI; i++) {
		fork_rst = fork();
		TEST_ERROR;
		switch(fork_rst) {
			case -1:
				TEST_ERROR;
				exit(1);
				
			case 0:
				sprintf(i_str, "%d", i);
				args[4] = i_str;
				execv("./ship", args);
				TEST_ERROR;
				exit(1);

			default:
				shared_ship[i].pid = fork_rst;
				break; 
		}
	}

	sprintf(name_file, "meteo");
	args[0] = name_file;
	args[1]= sem_sync_str;
	args[2]=shipSharedMemory_str;
	args[3]=port_sharedMemoryID_STR;
	args[4]=NULL;

	meteoPid = fork();
	TEST_ERROR;
	switch(meteoPid) {
		case -1:
			TEST_ERROR;
			exit(1);
			
		case 0:
			execv("./meteo", args);
			TEST_ERROR;
			exit(1);

		default:
			break; 
	}

	waitForZero(sops, sem_sync_id, 0);

	alarm(1);
	
	while (pastDays < SO_DAYS) {
		pause();
		if (errno == 4) errno = 0;
		else TEST_ERROR;
	}

	killChildren();
	cleanUp();

	string=malloc(25);
	numBytes=sprintf(string,"\n\nSIMULAZIONE FINITA!!!\n\n");
	fflush(stdout);
	write(1, string, numBytes);
	exit(EXIT_SUCCESS);
}