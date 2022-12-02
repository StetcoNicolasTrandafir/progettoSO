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

#include "macro.h"
#include "coordinates.h"

#define PRINT_ERROR fprintf(stderr,				\
			    "%s:%d: Errore #%3d \"%s\"\n",	\
			    __FILE__, __LINE__, errno, strerror(errno));

void handleSignal(int signal) {
	switch(signal) {
		case SIGALRM:

	}
}

/*source setEnv.sh*/

int main() {
	struct sigaction sa;
	struct timespec now;
	int i, j, fifo_fd;
	struct coordinates coord_c;
	char name_fifo[100];
	struct coordinates *coord_port;
	pid_t fork_rst;
	pid_t *port_pids, *ship_pids;
	coord_port = calloc(SO_PORTI, sizeof(struct coordinates));
	port_pids = calloc(SO_PORTI, sizeof(pid_t));
	ship_pids = calloc(SO_NAVI, sizeof(pid_t));
	alarm(30);
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handleSignal;
	sigaction(SIGALRM, &sa, NULL);
	for (i = 0; i < SO_PORTI; i++) {
		fork_rst = fork();
		switch(fork_rst) {
			case -1:
				PRINT_ERROR
				exit(1);

			case 0: 
				execlp("./port" ,"port.c", NULL);
				PRINT_ERROR
				exit(EXIT_FAILURE);

			default:
				port_pids[i] =fork_rst;
				sprintf(name_fifo, "%d", fork_rst);
				if (mkfifo(name_fifo, S_IRUSR | S_IWUSR) == -1) {
					PRINT_ERROR
					exit(1);
				}
				fifo_fd = open(name_fifo, O_WRONLY);
				switch(i) {
					case 0:
						coord_c.x = 0.0;
						coord_c.y = 0.0;
						break;

					case 1:
						coord_c.x = (double) SO_LATO;
						coord_c.y = 0.0;
						break;

					case 2:
						coord_c.x = 0.0;
						coord_c.y = (double) SO_LATO;
						break;

					case 3:
						coord_c.x = (double) SO_LATO;
						coord_c.y = (double) SO_LATO;
						break;

					default:
						
						do {
							coord_c = getRandomCoords();
						}
						while (existCoords(coord_port, i, coord_c));
				}
				write(fifo_fd, &coord_c, sizeof(struct coordinates));
				coord_port[i] = coord_c;
				close(fifo_fd);
		}
	}
	for (i = 0; i < SO_NAVI; i++) {
		fork_rst = fork();
		switch(fork_rst) {
			case -1:
				PRINT_ERROR
				exit(1);
				
			case 0:
				execlp("./ship", "ship.c", NULL);
				PRINT_ERROR
				exit(1);

			default:
				ship_pids[i] =fork_rst;
				break; 
		}
	}
	while(wait(NULL) != -1);
}