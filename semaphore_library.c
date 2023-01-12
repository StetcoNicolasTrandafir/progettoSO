#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "macro.h"


void decreaseSem(struct sembuf sops,int semID, int semNum){
    
    int ris;
    sops.sem_num = semNum; 
	sops.sem_op = -1; 
    ris=semop(semID, &sops, 1);

    while(ris==-1) {
        /*printf("\n\n[%d] SEMOP FALLITA ,  SEMID=%d RIS= %d",getpid(),semID, ris);*/
        if(errno!=4){
            break;
        }
        else {
            errno=0;

        }
    }
    /*printf("\n\n[%d]USCITO DAL WHILE,SEMID=%d RIS= %d",getpid(),semID ,ris);*/

    
    if(errno!=4){TEST_ERROR;}
    else errno=0;

    
}

void increaseSem(struct sembuf sops,int semID, int semNum){
    sops.sem_num = semNum; 
	sops.sem_op = 1; 
    semop(semID, &sops, 1);
    TEST_ERROR;
}


void waitForZero(struct sembuf sops,int semID, int semNum){
    int ris;
    sops.sem_num = semNum; 
	sops.sem_op = 0; 
    ris=semop(semID, &sops, 1);

    while(ris==-1) {
        /*printf("\n\n[%d] SEMOP FALLITA ,  SEMID=%d RIS= %d",getpid(),semID, ris);*/
        if(errno!=4){
            
            break;
        }
        else {
            errno=0;

        }
    }
    /*printf("\n\n[%d]USCITO DAL WHILE,SEMID=%d RIS= %d",getpid(),semID ,ris);*/

    
    if(errno!=4){TEST_ERROR;}
    else errno=0;
}