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
#include "types_module.h"
#include "utility_coordinates.h"
#include "utility_port.h"


#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s: a riga %d PID=%5d: Error %d: %s\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

/*
SEMAFORI:
-uno per ogni porto
-uno per ogni memoria condivisa
-uno per la sincronizzazione iniziale
*/

int pastDays = 0, *sum_request;
int sem_sync_id, sem_request_id;
int port_sharedMemoryID, sum_requestID;
int msg_id;
pid_t *port_pids, *ship_pids;
struct port_sharedMemory *sharedPortPositions;
pid_t meteoPid;

void finalReport(){
	int i, j;
	int maxRequestPortIndex=-1, maxRequest=0;
	int maxOfferPortIndex=-1, maxOfferSum=0;
	int *goodsStateSum;
	int totalGoodsSum=0;
	int inPortGoods=0;
	int shippedGoods=0;
	int *offerSum;
	int shipsInDock;
	struct goodsTypeReport *goodsReport;
	goods *g;
	struct request *r;

	
	goodsReport=calloc(SO_MERCI, sizeof(struct goodsTypeReport));
	offerSum=calloc(SO_MERCI, sizeof(int));
	goodsStateSum=calloc(5, sizeof(int));

	bzero(goodsReport, SO_MERCI*sizeof(struct goodsTypeReport));
	bzero(goodsStateSum, 5*sizeof(int));

	printf("\n==================>\t\tPORTI\n");
	
	for(i=0; i<SO_PORTI; i++){
		g=shmat(sharedPortPositions[i].offersID, NULL, 0);
		r=shmat(sharedPortPositions[i].requestID, NULL, 0);

		bzero(offerSum, SO_MERCI* sizeof(int));

		inPortGoods=0;
		shippedGoods=0;
		

		for(j=0; j<SO_DAYS && g[j].type!=0; j++){

			totalGoodsSum+=g[j].dimension;
			offerSum[(g[j].type-1)]+=g[j].dimension;

			goodsReport[(g[j].type-1)].totalSum+=g[j].dimension;
			goodsReport[(g[j].type-1)].inPort+=g[j].dimension-g[j].satisfied;

			if(g[j].state==expired_port)
				goodsReport[(g[j].type-1)].expiredInPort+=g[j].dimension-g[j].satisfied;

			goodsStateSum[g[j].state]+=g[j].dimension;

			inPortGoods+=g[j].dimension-g[j].satisfied;
			shippedGoods+=g[j].satisfied;
		}

		goodsReport[(r->goodsType-1)].delivered+=r->satisfied;

		for(j=0; j<SO_MERCI; j++){
			if(goodsReport[j].maxOffer<offerSum[j]){
				goodsReport[j].maxOffer=offerSum[j];
				goodsReport[j].maxOfferPortIndex=i;
			}
		}
		
		if(goodsReport[(r->goodsType-1)].maxRequest<r->quantity){
			goodsReport[(r->goodsType-1)].maxRequest=r->quantity;
			goodsReport[(r->goodsType-1)].maxOfferPortIndex=i;
		}

		printf("\n\nPORTO[%d] NUMERO %d (%.2f,%.2f):\nMerce in porto:%d\nMerce spedita:%d\nMerce ricevuta:%d\n",  sharedPortPositions[i].pid,(i+1),sharedPortPositions[i].coords.x,sharedPortPositions[i].coords.y, inPortGoods,shippedGoods,r-> satisfied);

		shmdt(g);
		shmdt(r);


	}


/*semval= valore semfaforo(numero banchine)

freeDocks= semctl(portSemaphoreID, 0, GETVAL);

come stracazzo mi trovo il valore iniziale del semaforo
*/


	/*TODO ships handling*/

	printf("\n\n ==================>\t\tMERCI\n");

	printf("\n\n\nTOTALE MERCE GENERATA: %dton", totalGoodsSum);
	for(i=0; i<SO_MERCI; i++){
		printf("\nMERCE DI TIPO %d:\nMerce generata: %dton\nMerce ferma in porto: %dton\nMerce scaduta in porto: %dton\nMerce scaduta in nave: %dton\nMerce consegnata: %d\nIl porto che ne ha fatto più richiesta è il numero %d (%dton)\nIl porto che ne ha generato di più è %d (%dton)\n",(i+1), goodsReport[i].totalSum, goodsReport[i].inPort, goodsReport[i].expiredInPort, -1, goodsReport[i].delivered, goodsReport[i].maxRequestPortIndex, goodsReport[i].maxRequest, goodsReport[i].maxOfferPortIndex, goodsReport[i].maxOffer);
	}


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

void sendSignalToCasualPorts(){

	int i, n_ports, *port_idx, casual_idx;
	struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    n_ports = (now.tv_nsec % SO_PORTI)+1;
    port_idx = calloc(n_ports, sizeof(int));
    for (i = 0; i < n_ports; i++) {
    	do {
    		clock_gettime(CLOCK_REALTIME, &now);
    		casual_idx = now.tv_nsec % SO_PORTI;
    	}
    	while (elementInArray(casual_idx, port_idx, i));
		port_idx[i] = casual_idx;
    	kill(port_pids[port_idx[i]], SIGUSR1); TEST_ERROR;
    }
	for (i = 0; i < SO_PORTI; i++) {
		if (!elementInArray(i, port_idx, n_ports)) {
    		kill(port_pids[i], SIGUSR2); TEST_ERROR;
    	}
	}
    free(port_idx);
}

void cleanUp(){
	int i;

	for(i=0; i< SO_PORTI; i++){
		kill(port_pids[i], SIGKILL);TEST_ERROR;
	}

	for(i=0; i< SO_NAVI; i++){
		kill(ship_pids[i], SIGKILL);TEST_ERROR;
	}

	semctl(sem_sync_id, 0, IPC_RMID); TEST_ERROR;
	shmdt(sharedPortPositions); TEST_ERROR;
	shmctl(port_sharedMemoryID, IPC_RMID, NULL); TEST_ERROR;
	shmdt(sum_request); TEST_ERROR;
	shmctl(sum_requestID, IPC_RMID, NULL); TEST_ERROR;
	semctl(sem_request_id, 0, IPC_RMID); TEST_ERROR;
	msgctl(msg_id, IPC_RMID, NULL); TEST_ERROR;
}

void handleSignal(int signal) {
	switch(signal) {
		case SIGALRM:
			if(++pastDays==SO_DAYS){
				printf("\n\n========================================================================\n\t\tREPORT FINALE:\n========================================================================\n\n");
				
				finalReport();
				
			}else{

				printf("\n\nREPORT GIORNALIERO (%d): \n", pastDays);
				sendSignalToCasualPorts();		
				kill(meteoPid, SIGUSR1); TEST_ERROR;
				alarm(1);
			}
			break;
		case SIGINT:
			printf("\n\n\nINTERRUZIONE INASPETTATA DEL PROGRAMMA\n");
			printf("Pulizia processi figli, IPCs, etc. ...\n");
			cleanUp();
			printf("Done!\n\n");
			exit(EXIT_FAILURE);
			break;
	}
}

/*source setEnv.sh*/

int main() {
	struct sigaction sa;
	int i, j;
	coordinates coord_c;
	char  *args[8], name_file[100], sem_sync_str[3 * sizeof(int) + 1], i_str[3 * sizeof(int) + 1], port_sharedMemoryID_STR[3*sizeof(int)+1], sum_requestID_STR[3 * sizeof(int) + 1], sem_request_str[3 * sizeof(int) + 1], msg_str[3 * sizeof(int) + 1];
	pid_t fork_rst;
	
	struct sembuf sops;
	struct shared_port *port_coords;
	goods *g;
	int idRequest;
	struct 	request *r;

	sharedPortPositions = calloc(SO_PORTI, sizeof(struct port_sharedMemory));
	sum_request = malloc(sizeof(int));
	port_pids = calloc(SO_PORTI, sizeof(pid_t));
	ship_pids = calloc(SO_NAVI, sizeof(pid_t));

	bzero(&sharedPortPositions, sizeof(sharedPortPositions));
	bzero(&sa, sizeof(sa));
	bzero(&sops, sizeof(sops));
	
	sa.sa_handler = handleSignal;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	sem_sync_id = semget(IPC_PRIVATE, 1, 0600); TEST_ERROR;
	semctl(sem_sync_id, 0, SETVAL, SO_PORTI + SO_NAVI); TEST_ERROR;

	sem_request_id = semget(IPC_PRIVATE, 3, 0600); TEST_ERROR;
	semctl(sem_request_id, 0, SETVAL, 1); TEST_ERROR; /*write*/
	semctl(sem_request_id, 1, SETVAL, SO_PORTI); TEST_ERROR; /*read*/
	semctl(sem_request_id, 2, SETVAL, 1); TEST_ERROR; /*controllo SO_FILL*/

	port_sharedMemoryID=shmget(IPC_PRIVATE, SO_PORTI*sizeof(struct port_sharedMemory),S_IRUSR | S_IWUSR | IPC_CREAT);
	TEST_ERROR;
	sharedPortPositions=shmat(port_sharedMemoryID, NULL, 0);
	TEST_ERROR;

	sum_requestID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR | S_IWUSR | IPC_CREAT); TEST_ERROR;
	sum_request = shmat(sum_requestID, NULL, 0); TEST_ERROR;
	sum_request = 0;
	/*shmdt(sum_request); TEST_ERROR;*/

	msg_id = msgget(getpid(), IPC_CREAT | IPC_EXCL | 0600); TEST_ERROR;
	printf("\n\nID CODA MESSAGGI (master) %d", msg_id);


	sprintf(name_file, "port");
	sprintf(sem_sync_str, "%d", sem_sync_id);
	sprintf(port_sharedMemoryID_STR, "%d", port_sharedMemoryID);
	sprintf(sum_requestID_STR, "%d", sum_requestID);
	sprintf(sem_request_str, "%d", sem_request_id);
	sprintf(msg_str, "%d", msg_id);

	args[0] = name_file;
	args[1] = sem_sync_str;
	args[2] = port_sharedMemoryID_STR;
	args[3] = sum_requestID_STR;
	args[4] = sem_request_str;
	args[5] = msg_str;
	args[7] = NULL;

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
				

			default:
				port_pids[i] = fork_rst;
				break;
		}
	}

	sprintf(name_file, "ship");
	args[0] = name_file;
	
	for (i = 0; i < SO_NAVI; i++) {
		fork_rst = fork();
		TEST_ERROR;
		switch(fork_rst) {
			case -1:
				TEST_ERROR;
				exit(1);
				
			case 0:
				execv("./ship", args);
				TEST_ERROR;
				exit(1);

			default:
				ship_pids[i] = fork_rst;
				break; 
		}
	}

	sprintf(name_file, "meteo");
	args[0] = name_file;
	args[1]=NULL;

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
			printf("\nFa brutto...==> %d\n", meteoPid);
			break; 
	}


	sops.sem_num = 0;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1);
	TEST_ERROR;

	alarm(1);
	sleep(31); /*Lo toglieremo , ma se lo tolgo ora, da un errore perchè eliminiamo il semaforo prima che l'ultimo processo abbia fatto il semop per aspettare tutti i processi*/

	for(i = 0; i < SO_NAVI + SO_PORTI; i++) wait(NULL);
	/*TEST_ERROR;*/


/*
	printf("\n\n[%d] LETTURA MEMORIA CONDIVISA DAL MASTER:\n", getpid());
	for(i=0; i< SO_PORTI; i++){
		
		printf("\n\nPORTO[%d] NUMERO %d:\n", sharedPortPositions[i].pid, (1+i));
		g= shmat(sharedPortPositions[i].offersID, NULL, 0);
		printf("offre merce di tipo %d in quantità %d ton\n", g[0].type, g[0].dimension );
		shmdt(g);
		shmctl(sharedPortPositions[i].offersID, IPC_RMID, NULL); TEST_ERROR;
		
		r=shmat(sharedPortPositions[i].requestID, NULL, 0); TEST_ERROR;
		printf("richiesta di tipo %d in quantità %d", r->goodsType, r->quantity);
		shmdt(r);
	}
*/
		
	semctl(sem_sync_id, 0, IPC_RMID); TEST_ERROR;
	semctl(sem_request_id, 0, IPC_RMID); TEST_ERROR;
	shmctl(sum_requestID, IPC_RMID, NULL); TEST_ERROR;
	msgctl(msg_id, IPC_RMID, NULL); TEST_ERROR;
	shmctl(port_sharedMemoryID, IPC_RMID, NULL); TEST_ERROR;
	
	printf("\n\nSIMULAZIONE FINITA!!!\n\n");
	exit(EXIT_FAILURE);
}