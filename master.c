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
#include "utility_coordinates.h"
#include "utility_goods.h"
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

int validIdx(int v_casual_idx, int v_port_idx[], int v_idx) {
	int i;
	for (i = 0;i < v_idx;i++) {
		if (v_port_idx[i] == v_casual_idx) {
			return 0;
		}
	}
	return 1;
}

void sendSignalToCasualPorts(){

	int i, s_n_ports, *s_port_idx, s_casual_idx;
	struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    s_n_ports = (now.tv_nsec % SO_PORTI)+1;
    s_port_idx = calloc(s_n_ports, sizeof(int));
    for (i = 0; i < s_n_ports; i++) {
    	do {
    		clock_gettime(CLOCK_REALTIME, &now);
    		s_casual_idx = now.tv_nsec % SO_PORTI;
    	}
    	while (!validIdx(s_casual_idx, s_port_idx, i));
		s_port_idx[i]=s_casual_idx;
    	kill(port_pids[i], SIGUSR1); TEST_ERROR;
    }
	printf("\n\nOGGI GENERERANNO %d PORTI, QUESTI SONO:\n", s_n_ports);
	for(i=0; i<s_n_ports; i++){
		printf("\n%d, il porto con pid %d", s_port_idx[i], port_pids[s_port_idx[i]]);
	}
    free(s_port_idx);
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
	char  *args[7], name_file[100], sem_sync_str[3 * sizeof(int) + 1], i_str[3 * sizeof(int) + 1], port_sharedMemoryID_STR[3*sizeof(int)+1], sum_requestID_STR[3 * sizeof(int) + 1], sem_request_str[3 * sizeof(int) + 1];
	pid_t fork_rst;
	struct sembuf sops;
	struct shared_port *port_coords;

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

	sprintf(name_file, "port");
	sprintf(sem_sync_str, "%d", sem_sync_id);
	sprintf(port_sharedMemoryID_STR, "%d", port_sharedMemoryID);
	sprintf(sum_requestID_STR, "%d", sum_requestID);
	sprintf(sem_request_str, "%d", sem_request_id);

	args[0] = name_file;
	args[1] = sem_sync_str;
	args[2] = port_sharedMemoryID_STR;
	args[3] = sum_requestID_STR;
	args[4] = sem_request_str;
	args[6] = NULL;

	for (i = 0; i < SO_PORTI; i++) {
		switch(fork_rst = fork()) {
			case -1:
				TEST_ERROR;
				exit(1);

			case 0: 
				sprintf(i_str, "%d", i);
				args[5] = i_str;
				
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

	msg_id = msgget(getpid(), IPC_CREAT | IPC_EXCL | 0600); TEST_ERROR;

	sops.sem_num = 0;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1);
	TEST_ERROR;



	alarm(1);
	sleep(31); /*Lo toglieremo , ma se lo tolgo ora, da un errore perchè eliminiamo il semaforo prima che l'ultimo processo abbia fatto il semop per aspettare tutti i processi*/

	for(i = 0; i < SO_NAVI + SO_PORTI; i++) wait(NULL);
	TEST_ERROR;
	
	semctl(sem_sync_id, 0, IPC_RMID); TEST_ERROR;
	semctl(sem_request_id, 0, IPC_RMID); TEST_ERROR;
	shmctl(sum_requestID, IPC_RMID, NULL); TEST_ERROR;
	msgctl(msg_id, IPC_RMID, NULL); TEST_ERROR;
	shmctl(port_sharedMemoryID, IPC_RMID, NULL); TEST_ERROR;
	
	printf("\n\nSIMULAZIONE FINITA!!!\n\n");
}