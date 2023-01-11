#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "macro.h"


void decreaseSem(struct sembuf sops,int semID, int semNum){
    
    sops.sem_num = semNum; 
	sops.sem_op = -1; 

    while((semop(semID, &sops, 1))==-1 && errno == EINTR) {
        if(errno!=4){
            TEST_ERROR;
        }
        else errno=0;
    }
    errno=0;
}

void increaseSem(struct sembuf sops,int semID, int semNum){
    sops.sem_num = semNum; 
	sops.sem_op = 1; 
    semop(semID, &sops, 1);
}


void waitForZero(struct sembuf sops,int semID, int semNum){
    sops.sem_num = semNum; 
	sops.sem_op = 0; 

    while((semop(semID, &sops, 1))==-1 && errno == EINTR) {
        if(errno!=4){
            TEST_ERROR;
        }
    }
    errno=0;
}