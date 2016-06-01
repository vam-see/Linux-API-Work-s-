
/* 
# Author: Vamsee
# Title : Process Messaging
# Syracuse University 
*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>

#define SHMSIZE BUFSIZ

int semr = 0;
int shm_id, sem_id;
key_t shm_key = 1357;
key_t sem_key = 7896;
char *shm, *s;
char *in_line = NULL;
size_t line_capacity = 0;
char msg[2048];
char *exit_code = "exit\n";
struct shmid_ds a;

void* writer(void* arg)
{
	int i;
	while(1)
	{
		if (getline(&in_line, &line_capacity, stdin) != -1)
		{
			if (strcmp(in_line, exit_code) == 0)
			{
				if (shmdt(shm) < 0)
				{
					perror("Error detaching shared memory");
					exit(0);
				}
				
				if (shmctl(shm_id, IPC_STAT, &a) < 0)
				{
					perror("Error getting attached processes");
					exit(0);
				}

				if (a.shm_nattch == 0)
				{
					shmctl(shm_id, IPC_RMID, NULL);
					semctl(sem_id, 0, IPC_RMID, 0);
					exit(1);
				}
				exit(1);
			}

			struct sembuf w = {0, -1, 0};
			if (semop(sem_id, &w, 1) == -1)
			{
				perror("Error while write lock");
				exit(0);
			}

			// printf("Writing to shared memory \n");

			sprintf(msg, "Process %ld says : %s", (long)getpid(), in_line);
			memcpy(s, msg, strlen(msg));
			semr += strlen(msg);
			
			// printf("Finished writing to shared memory \n");

			w.sem_op = 1;
			if (semop(sem_id, &w, 1) == -1)
			{
				perror("Error while write unlock");
				exit(0);
			}
		}
	}
}

void* reader(void* arg)
{
	
	for (s = shm; *s != '\0'; s++)
	{
		printf("%c", *s);
	}
	while(1)
	{
		struct sembuf r = {0, -1, 0};
		if (semop(sem_id, &r, 1) == -1)
		{
			perror("Error while read lock");
			exit(0);
		}
		for (s = shm + semr; *s != '\0'; s++)
		{
			printf("%c", *s);
			semr++;
		}

		r.sem_op = 1;
		if (semop(sem_id, &r, 1))
		{
			perror("Error while read unlock");
			exit(0);
		}

		sleep(1);			
	}
	
}

int main(int argc, char const *argv[])
{
	pthread_t write_th, read_th;
	shm_id = shmget(shm_key, SHMSIZE, IPC_CREAT | S_IRUSR | S_IWUSR);

	if (shm_id < 0)
	{
		perror("Error creating shared memory \n");
		exit(0);
	}

	sem_id = semget(sem_key, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

	if (sem_id < 0)
	{
		perror("Error creating semaphore \n");
		exit(0);
	}

	shm = shmat(shm_id, NULL, 0);

	if (shm == (char *) -1)
	{
		perror("Error attaching shared memory \n");
		exit(0);
	}
	
	s = shm;

	semctl(sem_id, 0, SETVAL, 1);

	if (pthread_create(&write_th, NULL, writer, NULL))
	{
		perror("Error creating write thread");
	}

	if (pthread_create(&read_th, NULL, reader, NULL))
	{
		perror("Error creating read thread");
	}

	pthread_join(write_th, NULL);
	pthread_join(read_th, NULL);

	return 0;
}