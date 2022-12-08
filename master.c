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




int pastDays=0;
int sem_sync_id;
int sem_report_id;
int port_sharedMemoryID;
pid_t *port_pids, *ship_pids;
struct port_sharedMemory *sharedPortPositions;

void sendSignalToAllPorts(){
	int i;
	for(i=0; i<SO_PORTI; i++){
		printf("\nSending signal to [%d]", port_pids[i]);
		if(kill(port_pids[i], SIGUSR1)) TEST_ERROR;
	}
}

void cleanUp(){
	int i;

	for(i=0; i< SO_PORTI; i++){
		kill(port_pids[i], SIGKILL);
	}

	for(i=0; i< SO_NAVI; i++){
		kill(ship_pids[i], SIGKILL);
	}

	semctl(sem_sync_id, 0, IPC_RMID); TEST_ERROR;
	semctl(sem_report_id, 0, IPC_RMID); TEST_ERROR;
	shmdt(sharedPortPositions); TEST_ERROR;
	shmctl(port_sharedMemoryID, IPC_RMID, NULL); TEST_ERROR;
}

void handleSignal(int signal) {
	switch(signal) {
		case SIGALRM:
			if(pastDays==SO_DAYS){
				printf("\nREPORT FINALE:\n");
				/*finalReport();*/
				cleanUp();
			}else{

				printf("\n\nREPORT GIORNALIERO: \n");
				sendSignalToAllPorts();				
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
	char  *args[6], name_file[100], sem_sync_str[3 * sizeof(int) + 1], sem_report_str[3 * sizeof(int) + 1], i_str[3 * sizeof(int) + 1], port_sharedMemoryID_STR[3*sizeof(int)+1];
	coordinates *coord_port;
	pid_t fork_rst;
	struct sembuf sops;
	struct shared_port *port_coords;
	struct port_sharedMemory *sharedPortPositions;
	
	coord_port = calloc(SO_PORTI, sizeof(coordinates));
	sharedPortPositions = calloc(SO_PORTI, sizeof(struct port_sharedMemory));
	port_pids = calloc(SO_PORTI, sizeof(pid_t));
	ship_pids = calloc(SO_NAVI, sizeof(pid_t));

	bzero(&sharedPortPositions, sizeof(sharedPortPositions));
	bzero(&sa, sizeof(sa));
	bzero(&sops, sizeof(sops));
	
	sa.sa_handler = handleSignal;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	

	sem_sync_id = semget(IPC_PRIVATE, 1, 0600);
	TEST_ERROR;
	
	semctl(sem_sync_id, 0, SETVAL, SO_PORTI + SO_NAVI);
	TEST_ERROR;

	sem_report_id = semget(IPC_PRIVATE, 1, 0600);
	TEST_ERROR;

	semctl(sem_report_id, 0, SETVAL, 1);
	TEST_ERROR;

	sprintf(name_file, "port");
	sprintf(sem_sync_str, "%d", sem_sync_id);

	args[0] = name_file;
	args[1] = sem_sync_str;
	args[4] = sem_report_str;
	args[5] = NULL;

	port_sharedMemoryID=shmget(IPC_PRIVATE, SO_PORTI*sizeof(struct port_sharedMemory),S_IRUSR | S_IWUSR | IPC_CREAT);
	TEST_ERROR;
	sharedPortPositions=shmat(port_sharedMemoryID, NULL, 0);
	TEST_ERROR;

	for (i = 0; i < SO_PORTI; i++) {
		switch(fork_rst = fork()) {
			case -1:
				TEST_ERROR;
				exit(1);

			case 0: 
				sprintf(i_str, "%d", i);
				sprintf(port_sharedMemoryID_STR, "%d", port_sharedMemoryID);
				args[3] = i_str;
				args[2]= port_sharedMemoryID_STR;
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
				ship_pids[i] =fork_rst;
				break; 
		}
	}	

	sops.sem_num = 0;
	sops.sem_op = 0;
	semop(sem_sync_id, &sops, 1);
	TEST_ERROR;

	sleep(1); /*Lo toglieremo , ma se lo tolgo ora, da un errore perchè eliminiamo il semaforo prima che l'ultimo processo abbia fatto il semop per aspettare tutti i processi*/
	
	for(i=0; i< SO_NAVI+SO_PORTI; i++) wait(NULL);
	TEST_ERROR;

	semctl(sem_sync_id,0, IPC_RMID); TEST_ERROR;
	semctl(sem_report_id,0, IPC_RMID); TEST_ERROR;
	
	printf("\n\nSIMULAZIONE FINITA!!!\n\n");
}


