#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

#include <mqueue.h>
#include <errno.h>
#include <signal.h>

#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define NB_THREADS 5
#define V_LENGTH 100

#define MSG_SIZE 1024
#define MSG_QUEUE_NAME "/lafile"


int X[V_LENGTH*4]; //= {1,2,3,4,5,6,7,8};
int Y[V_LENGTH*4]; //= {1,2,3,4,5,6,7,8};


void *produitScalaire(void *arg)
{
	int i;
	int x,y;
    int resultat;
    int thread_number;
	char buf[MSG_SIZE];
	//char *ptr;
    mqd_t mqdes;

    thread_number = *(int *)arg;
    //memset(buf, 0, MSG_SIZE);
    //printf("buf in start = %s\n",buf);
    mqdes = mq_open(MSG_QUEUE_NAME, O_WRONLY);
	for (i=0; i<V_LENGTH; i++)
	{
		x = *(X + (thread_number)*V_LENGTH + i);
		y = *(Y + (thread_number)*V_LENGTH + i);
		resultat = resultat + x*y;
	}
	printf("resultat de travail de Thread %d est %d\n", thread_number, resultat);
    sprintf (buf, "%d", resultat);
    //printf("result as string = %s\n",*(buf + thread_number));
	//ptr = strdup(buf);
    if (mq_send (mqdes, buf, MSG_SIZE, 0) == -1){
            perror ("Thread: Not able to send message");
    }
   pthread_exit((void *) 0);
	//return NULL;
}

void *print_ScalProduct(void *null){
    int i,prod_scalaire = 0;
    mqd_t mqdes;
	char buf[MSG_SIZE];

    mqdes = mq_open(MSG_QUEUE_NAME, O_RDONLY);
    for (i = 0; i<NB_THREADS-1; i++){
        memset(buf, 0, MSG_SIZE);
        if (mq_receive (mqdes, buf, MSG_SIZE, 0) == -1) {
          perror ("Thread 5: Not able to receive message");  
        }
        //printf("result when string received = %s\n",buf);
        prod_scalaire += atoi(buf);
    }
	printf(" le produit scalaire de X et Y est %d\n", prod_scalaire);

	pthread_exit((void *) 0);
	//pthread_mutex_unlock(&verrou);
}

int main (int argc, char *argv[])
{
	pthread_t thread[NB_THREADS];
    int thread_number[NB_THREADS-1] = {0};
	int t,rc, i;
	void *status; 

    struct mq_attr mqattr;
    mqd_t mqdes; // descripteur de file de msg

    mqattr.mq_maxmsg = 5;
    mqattr.mq_msgsize = MSG_SIZE;
    mqattr.mq_flags = 0;

    mqdes = mq_open (MSG_QUEUE_NAME, O_RDWR |O_CREAT,0664, &mqattr);

    for(i=0; i<V_LENGTH*4;i++){
		X[i] = i+1;
		Y[i] = V_LENGTH*4 - i;
	}


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

    mq_close (mqdes);
	mq_unlink(MSG_QUEUE_NAME);
    return 0;
}