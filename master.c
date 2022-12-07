#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>	
#include <stdlib.h>
#include <stdio.h>
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
#include "utility_shm.h"

#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s: a riga %d: PID=%5d: Error %d (%s)\n", \
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




int pastDays=0, sem_sync_id, sem_report_id, shm_id;
pid_t *port_pids, *ship_pids;

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
	shmctl(shm_id, IPC_RMID, NULL); TEST_ERROR;
}

void handleSignal(int signal) {
	switch(signal) {
		case SIGALRM:
			if(pastDays==SO_DAYS){
				printf("\nREPORT FINALE:\n");
				/*finalReport();*/
				/*killAllChildren();*/
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
	char  *args[6], name_file[100], sem_report_str[sizeof(sem_report_id) + 1], sem_sync_str[sizeof(sem_sync_id) + 1], shm_id_str[sizeof(shm_id) + 1], i_str[sizeof(i) + 1];
	pid_t fork_rst;
	struct sembuf sops;
	struct shared_port *port_coords;

	port_pids = calloc(SO_PORTI, sizeof(pid_t));
	ship_pids = calloc(SO_NAVI, sizeof(pid_t));
	alarm(30);
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handleSignal;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	bzero(&sops, sizeof(sops));
	sem_sync_id = semget(IPC_PRIVATE, 2, 0600);
	TEST_ERROR;
	semctl(sem_sync_id, 0, SETVAL, SO_PORTI + SO_NAVI);
	TEST_ERROR;
	sem_report_id = semget(IPC_PRIVATE, 1, 0600);
	TEST_ERROR;
	semctl(sem_report_id, 0, SETVAL, 1);
	TEST_ERROR;
	sprintf(name_file, "port");
	args[0] = name_file;
	sprintf(sem_sync_str, "%d", sem_sync_id);
	args[1] = sem_sync_str;
	args[5] = NULL;
	shm_id = shmget(IPC_PRIVATE, (sizeof(int) + (sizeof(pid_t) * SO_PORTI) + (sizeof(coordinates) * SO_PORTI)), 0600);
	TEST_ERROR;
	sprintf(shm_id_str, "%d", shm_id);
	args[2] = shm_id_str;
	sprintf(sem_report_str, "%d", sem_report_id);
	args[4] = sem_report_str;

	port_coords = shmat(shm_id, NULL, 0);
	TEST_ERROR;
	/*port_coords -> pid = calloc(SO_PORTI, sizeof(pid_t));
	port_coords -> coords = calloc(SO_PORTI, sizeof(coordinates));*/
	port_coords -> cur_idx = 4;
	port_coords -> coords[0].x = 0.0;
	port_coords -> coords[0].y = 0.0;
	port_coords -> coords[1].x = SO_LATO;
	port_coords -> coords[1].y = 0.0;
	port_coords -> coords[2].x = 0.0;
	port_coords -> coords[2].y = SO_LATO;
	port_coords -> coords[3].x = SO_LATO;
	port_coords -> coords[3].y = SO_LATO;

	semctl(sem_sync_id, 1, SETVAL, 1);
	TEST_ERROR;
	for (i = 0; i < SO_PORTI; i++) {
		switch(fork_rst = fork()) {
			case -1:
				TEST_ERROR;
				exit(1);


			case 0: 
				sprintf(i_str, "%d", i);
				args[3] = i_str;
				execv("./port", args);
				TEST_ERROR;
				exit(EXIT_FAILURE);

			default:
				port_pids[i] = fork_rst;
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
	shmctl(shm_id, IPC_RMID, NULL);
	TEST_ERROR;
	sleep(1); /*Lo toglieremo , ma se lo tolgo ora, da un errore perchè eliminiamo il semaforo prima che l'ultimo processo abbia fatto il semop per aspettare tutti i processi*/
	semctl(sem_sync_id, 0, IPC_RMID);
	TEST_ERROR;
	
	while(wait(NULL) != -1);

	semctl(sem_report_id, 0, IPC_RMID);
	TEST_ERROR;
	
	sleep(1);
	
	printf("\n\nSIMULAZIONE FINITA!!!\n\n");
}


