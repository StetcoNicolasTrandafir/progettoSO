#define LOCK					\
	sops.sem_num = 1;			\
	sops.sem_op = -1;			\
	sops.sem_flg = 0;			\
	semop(sem_id, &sops, 1);
#define UNLOCK					\
	sops.sem_num = 1;			\
	sops.sem_op = 1;			\
	sops.sem_flg = 0;			\
	semop(sem_id, &sops, 1);	
