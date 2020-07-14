#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#define NB_THREADS 3

void *travailUtile(void *arg)
{
	int i;
	double resultat=0.0;
	for (i=0; i<1000000; i++)
	{
		resultat = resultat + (double)(* (int *)arg);
	}
	printf("resultat= %e\n",resultat);
	pthread_exit((void *) 0);
}

int main (int argc, char *argv[])
{
	pthread_t thread[NB_THREADS];
	pthread_attr_t attr;
	int rc, t;
	void *status;
	size_t stacksize;
	int value;

	/* Initialisation et activation d’attributs*/
	pthread_attr_init(&attr);//valeur par défaut
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);//attente du thread possible
	
	if(pthread_attr_getstacksize(&attr,&stacksize) == 0){
		printf("La taille de stack des thread est : %lx Bytes\n", stacksize);
		stacksize += 0x100000;
		pthread_attr_setstacksize(&attr,stacksize);
		printf("La taille de stack des threads apres augmentation est : %lx Bytes\n", stacksize);
	}

	for(t=0; t<NB_THREADS; t++)
	{
		value = random();
		printf("Creation du thread %d\n", t);
		rc = pthread_create(&thread[t], &attr, travailUtile, (void *) &value); 
		if (rc)
		{
		printf("ERROR; le code de retour depthread_create() est%d\n", rc);
		exit(-1);
		}
	}

	/* liberation des attributs et attente de la terminaison desthreads*/
	pthread_attr_destroy(&attr);
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
	pthread_exit(NULL);

}



