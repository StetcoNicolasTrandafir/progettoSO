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
-
*/

int pastDays = 0, *sum_request;
int sem_sync_id, sem_request_id;
int port_sharedMemoryID, sum_requestID;
int msg_id;
pid_t *port_pids, *ship_pids;
struct port_sharedMemory *sharedPortPositions;
pid_t meteoPid;

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
			if(pastDays++==SO_DAYS){
				printf("\nREPORT FINALE:\n");
				/*finalReport();*/
				
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

	msg_id = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0600); TEST_ERROR;

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
	TEST_ERROR;


	
	printf("[%d] LETTURA MEMORIA CONDIVISA DAL MASTER:\n", getpid());

	for(i=0; i< SO_PORTI; i++){
		g= shmat(sharedPortPositions[i].offersID, NULL, 0);
		printf("Porto [%d] in posizione (%f,%f) offre merce di tipo %d in quantità %d ton\n", sharedPortPositions[i].pid,sharedPortPositions[i].coords.x,sharedPortPositions[i].coords.y, g[0].type, g[0].dimension );
		shmdt(g);
		shmctl(sharedPortPositions[i].offersID, IPC_RMID, NULL); TEST_ERROR;
	}
	
		
	semctl(sem_sync_id, 0, IPC_RMID); TEST_ERROR;
	semctl(sem_request_id, 0, IPC_RMID); TEST_ERROR;
	shmctl(sum_requestID, IPC_RMID, NULL); TEST_ERROR;
	msgctl(msg_id, IPC_RMID, NULL); TEST_ERROR;
	shmctl(port_sharedMemoryID, IPC_RMID, NULL); TEST_ERROR;
	
	printf("\n\nSIMULAZIONE FINITA!!!\n\n");
}