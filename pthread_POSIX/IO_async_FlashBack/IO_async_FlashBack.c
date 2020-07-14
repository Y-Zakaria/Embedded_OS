#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <aio.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/resource.h>


#define NB_THREADS 3
#define V_LENGTH 200
#define BUF_SIZE V_LENGTH*2


pthread_mutex_t verrou;
pthread_cond_t finish;

int prd_scalaire = 0;
int thread_number = 0;
int X[V_LENGTH*2];
int Y[V_LENGTH*2];

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define errMsg(msg)  do { perror(msg); } while (0)

struct ioRequest {      
    int           reqNum;
    int           status;
    struct aiocb *aiocbp;
};


#define IO_SIGNAL SIGUSR1   /* Signal used to notify I/O completion */

static void aioSigHandler(int sig, siginfo_t *si, void *ucontext)    /* Handler for I/O completion signal */
{
    if (si->si_code == SI_ASYNCIO) {
        write(STDOUT_FILENO, "I/O completion signal received\n", 31);
    }
}


/* Threads job*/
void *produitScalaire(void *null)
{
	int i;
	int x,y,resultat[NB_THREADS] = {0};
	pthread_mutex_lock(&verrou);
	//int part = * (int *)t;
	for (i=0; i<V_LENGTH; i++)
	{
		x = *(X + (thread_number)*V_LENGTH + i);
		//printf("X[%d] = %d\n",i,x);
		y = *(Y + (thread_number)*V_LENGTH + i);
		resultat[thread_number] = resultat[thread_number] + x*y;
	}
	printf("resultat de travail de Thread %d est %d\n", thread_number, resultat[thread_number]);
	prd_scalaire += resultat[thread_number];
	thread_number++;
	if (thread_number == NB_THREADS -1) pthread_cond_signal(&finish);
	pthread_mutex_unlock(&verrou);
	
	return NULL;
}

void *print_ScalProduct(void *null){
	pthread_mutex_lock(&verrou);
	if (thread_number != NB_THREADS -1) pthread_cond_wait(&finish,&verrou);
    printf(" le produit scalaire de X et Y est %d\n", prd_scalaire);
	pthread_mutex_unlock(&verrou);

    return NULL;
}



int main(int argc, char *argv[])
{
    /* threads variables*/
    pthread_t thread[NB_THREADS];
	int t,rc;
	void *status;
	pthread_cond_init(&finish,0); //initialize the condition variable

    /*I/O reqs variables */
    struct ioRequest *ioList;
    struct aiocb *aiocbList;
    struct sigaction sa;
    int s, j, i = 0;
    int numReqs = 4;        /* Total number of queued I/O requests */
    int openReqs;       /* Number of I/O requests still in progress */
    int Filenum = 1;
    char *stokenptr; 

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pathname> <pathname>...\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Allocate our arrays */

    ioList = calloc(numReqs, sizeof(struct ioRequest));
    if (ioList == NULL)
        errExit("calloc");

    aiocbList = calloc(numReqs, sizeof(struct aiocb));
    if (aiocbList == NULL)
        errExit("calloc");

    /* Establish handlers for the I/O completion signal */

    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = aioSigHandler;
    if (sigaction(IO_SIGNAL, &sa, NULL) == -1)
        errExit("sigaction");

    /* Open vector files specified on the command line, and queue
         read requests on the resulting file descriptors */

    for (j = 0; j < numReqs; j++){
        if (j > 1)
            Filenum = 2;
        ioList[j].reqNum = j;
        ioList[j].status = EINPROGRESS;
        ioList[j].aiocbp = &aiocbList[j];
        ioList[j].aiocbp->aio_fildes = open(argv[Filenum], O_RDONLY);
        if (ioList[j].aiocbp->aio_fildes == -1)
            errExit("open");
        printf("opened %s on descriptor %d\n", argv[1],
                ioList[j].aiocbp->aio_fildes);

        ioList[j].aiocbp->aio_buf = malloc(BUF_SIZE);
        if (ioList[j].aiocbp->aio_buf == NULL)
            errExit("malloc");

        ioList[j].aiocbp->aio_nbytes = BUF_SIZE;
        ioList[j].aiocbp->aio_reqprio = 0;
        ioList[j].aiocbp->aio_offset = (j%2)*BUF_SIZE; // 0,2,4,6  => 0,3,6,9
        ioList[j].aiocbp->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
        ioList[j].aiocbp->aio_sigevent.sigev_signo = IO_SIGNAL;
        ioList[j].aiocbp->aio_sigevent.sigev_value.sival_ptr =
                                &ioList[j];

        s = aio_read(ioList[j].aiocbp);
        //printf("size = %ld\n",strlen((char *)ioList[j].aiocbp->aio_buf));
        if (s == -1)
            errExit("aio_read");
    }

    openReqs = numReqs;

    /* Loop, monitoring status of I/O requests */

    while (openReqs > 0) {
        sleep(3);       /* Delay between each monitoring step */

        /* Check the status of each I/O request that is still
            in progress */

        printf("aio_error():\n");
        for (j = 0; j < numReqs; j++) {
            if (ioList[j].status == EINPROGRESS) {
                printf("    for request %d (descriptor %d): ",
                        j, ioList[j].aiocbp->aio_fildes);
                ioList[j].status = aio_error(ioList[j].aiocbp);

                switch (ioList[j].status) {
                case 0:
                    printf("I/O succeeded\n");
                    break;
                case EINPROGRESS:
                    printf("In progress\n");
                    break;
                case ECANCELED:
                    printf("Canceled\n");
                    break;
                default:
                    errMsg("aio_error");
                    break;
                }

                if (ioList[j].status != EINPROGRESS)
                    openReqs--;
            }
        }
    }

    printf("All I/O requests completed\n");

    /* Check status return of all I/O requests */

    printf("aio_return():\n");
    for (j = 0; j < numReqs; j++) {
        ssize_t size;

        size = aio_return(ioList[j].aiocbp);
        printf("    for request %d (descriptor %d), size is %zd and ",
               j, ioList[j].aiocbp->aio_fildes, size);
        if (size > -1){
            //printf("Result of reading is : %s",(char *)ioList[j].aiocbp->aio_buf);
            stokenptr = strtok((char *)ioList[j].aiocbp->aio_buf,"\n");
            while(stokenptr != NULL && i < V_LENGTH){
                if (j<2){
                    X[i + (j%2)*200] = atoi(stokenptr);
                    stokenptr = strtok( NULL,"\n");
                }
                else{
                    Y[i+(j%2)*200] = atoi(stokenptr);
                    //printf("Y[%d] = %d\n",i + (j%2)*200,Y[i + (j%2)*200]);
                    stokenptr = strtok( NULL,"\n");
                }
                i++;
            }
            i = 0;
        }
         printf("\n");
    }

    for(t=0; t<NB_THREADS - 1; t++)
	{
		rc = pthread_create(&thread[t], NULL, produitScalaire, NULL);
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

    exit(EXIT_SUCCESS);
}