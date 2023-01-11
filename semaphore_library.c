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
    TEST_ERROR;
    if(semop(semID, &sops, 1)==-1){
        if(errno==EINTR){
            errno=0;
            decreaseSem(sops, semID, semNum);
            
        }
        else{
            TEST_ERROR;
        }
    }
    TEST_ERROR;
}

void increaseSem(struct sembuf sops,int semID, int semNum){
    TEST_ERROR;
    sops.sem_num = semNum; 
	sops.sem_op = 1; 
    semop(semID, &sops, 1);
    TEST_ERROR;

}


void waitForZero(struct sembuf sops,int semID, int semNum){
    sops.sem_num = semNum; 
	sops.sem_op = 0; 

    if(semop(semID, &sops, 1)==-1){
        if(errno==EINTR){
            errno=0;
            waitForZero(sops, semID, semNum);
            errno=0;
        }
        else{
            TEST_ERROR;
        }
    }
    errno=0;
}