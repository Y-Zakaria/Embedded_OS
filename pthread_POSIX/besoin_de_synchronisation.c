#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/mman.h> 
#include <sys/stat.h> 
#include <sys/types.h>

#define NB_THREADS 5
#define V_LENGTH 100

struct data{
    int X[V_LENGTH*4]; //= {1,2,3,4,5,6,7,8};
    int Y[V_LENGTH*4]; //= {1,2,3,4,5,6,7,8};
    int resultat[NB_THREADS];
    int prd_scalaire;
    int counter;
};
struct data *shdata;

pthread_mutex_t verrou;
pthread_cond_t finish;


void *produitScalaire(void *arg)
{
	int i;
	int x,y;
    int thread_number;
    thread_number = *(int *)arg;
	for (i=0; i<V_LENGTH; i++)
	{
		x = *(shdata->X + (thread_number)*V_LENGTH + i);
		y = *(shdata->Y + (thread_number)*V_LENGTH + i);
		shdata->resultat[thread_number] = shdata->resultat[thread_number] + x*y;
	}
	printf("resultat de travail de Thread %d est %d\n", thread_number, shdata->resultat[thread_number]);
    pthread_mutex_lock(&verrou);
	shdata->prd_scalaire += shdata->resultat[thread_number];
	shdata->counter++;
	if (shdata->counter == NB_THREADS -1) pthread_cond_signal(&finish);
	pthread_mutex_unlock(&verrou);
	
	return NULL;
}

void *print_ScalProduct(void *null){
	pthread_mutex_lock(&verrou);
	// attendre que le thread 3 relâche la variable conditionnelle
	if (shdata->counter != NB_THREADS -1) pthread_cond_wait(&finish,&verrou);
	printf(" le produit scalaire de X et Y est %d\n", shdata->prd_scalaire);
	pthread_mutex_unlock(&verrou);

	return NULL;
}

int main (int argc, char *argv[])
{
	pthread_t thread[NB_THREADS];
    int thread_number[NB_THREADS-1] = {0};
	int t,rc, i;
	void *status; 

    /* shared memory */
    int length = sizeof(*shdata); // size of shared memory in bytes
    const char* name = "prScal_shm";  
    int shm_fd; //shared memory file descriptor
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666); //create the shared memory
    if (ftruncate(shm_fd, length) == -1) // configure the size of the shared memory
		printf("ftruncate error");
    shdata = mmap(0, length, PROT_WRITE, MAP_SHARED, shm_fd, 0); //map the shared memory to the shared data

    /** initialize shared data **/
    shdata->prd_scalaire = 0;
    shdata->counter = 0;
    
    for(i=0; i<NB_THREADS*4;i++){
		shdata->resultat[i] = 0;
	}

	for(i=0; i<V_LENGTH*4;i++){
		shdata->X[i] = i+1;
		shdata->Y[i] = V_LENGTH*4 - i;
	}

	/** initialize the condition variable **/
	pthread_cond_init(&finish,0);

	for(t=0; t<NB_THREADS - 1; t++)
	{
        thread_number[t] = t;
		rc = pthread_create(&thread[t], NULL, produitScalaire, (void *)&thread_number[t]);
		if (rc)
		{
            printf("ERROR; le code de retour de pthread_create() est %d\n", rc);
            exit(-1);
		}
	}

	rc = pthread_create(&thread[NB_THREADS-1], NULL, print_ScalProduct, NULL);
	if (rc)
	{
    	printf("ERROR; le code de retour de pthread_create() est %d\n", rc);
        exit(-1);
	}

	for(t=0; t<NB_THREADS; t++)
	{
		rc = pthread_join(thread[t], &status);
		if (rc)
		{
			printf("ERROR; le code de retour dupthread_join() est %d\n", rc);
			exit(-1);
		}
		printf("le join a fini avec le thread %d et a donne le status= %ld\n",t,(long)status);
	}
	pthread_mutex_destroy(&verrou);

    if (close(shm_fd) == -1)
        printf("error occured during closing shared memory file decriptor\n");

    if (shm_unlink(name) == -1) // destroy the shared memory
        printf("error occured during destroy the shared memory\n");

    return 0;
}