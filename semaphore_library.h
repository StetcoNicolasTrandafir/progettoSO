#ifndef _SEMAPHORE_LIBRARY_H
#define _SEMAPHORE_LIBRARY_H


void decreaseSem(struct sembuf sops,int semID, int semNum);

void increaseSem(struct sembuf sops,int semID, int semNum);

void waitForZero(struct sembuf sops,int semID, int semNum);

#endif /*_SEMAPHORE_LIBRARY_H*/