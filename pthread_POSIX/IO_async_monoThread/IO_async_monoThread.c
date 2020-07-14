#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <aio.h>
#include <signal.h>

#define BUF_SIZE 20     /* Size of buffers for read operations */

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define errMsg(msg)  do { perror(msg); } while (0)

struct ioRequest {      /* Application-defined structure for tracking
                            I/O requests */
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

int main(int argc, char *argv[])
{
    struct ioRequest *ioList;
    struct aiocb *aiocbList;
    struct sigaction sa;
    int s, j;
    int numReqs;        /* Total number of queued I/O requests */
    int openReqs;       /* Number of I/O requests still in progress */
    int Filenum = 1;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pathname> <pathname>...\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    numReqs = argc;

    /* Allocate our arrays */

    ioList = calloc(numReqs, sizeof(struct ioRequest));
    if (ioList == NULL)
        errExit("calloc");

    aiocbList = calloc(numReqs, sizeof(struct aiocb));
    if (aiocbList == NULL)
        errExit("calloc");

    /* The I/O completion signal */
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = aioSigHandler;
    if (sigaction(IO_SIGNAL, &sa, NULL) == -1)
        errExit("sigaction");

    /*Lire et écrire dans les deux fichiers **/
    for (j = 0; j < numReqs; j++){
        if(j == 2)
            Filenum = 2;
        ioList[j].reqNum = j;
        ioList[j].status = EINPROGRESS;
        ioList[j].aiocbp = &aiocbList[j];
        ioList[j].aiocbp->aio_fildes = open(argv[Filenum], O_RDWR);
        if (ioList[j].aiocbp->aio_fildes == -1)
            errExit("open");
        printf("opened %s on descriptor %d\n", argv[Filenum],
                ioList[j].aiocbp->aio_fildes);

        ioList[j].aiocbp->aio_buf = malloc(BUF_SIZE);
        if (ioList[j].aiocbp->aio_buf == NULL)
            errExit("malloc");

        ioList[j].aiocbp->aio_nbytes = BUF_SIZE;
        ioList[j].aiocbp->aio_reqprio = 0;
        ioList[j].aiocbp->aio_offset = (j%2)*BUF_SIZE;
        ioList[j].aiocbp->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
        ioList[j].aiocbp->aio_sigevent.sigev_signo = IO_SIGNAL;
        ioList[j].aiocbp->aio_sigevent.sigev_value.sival_ptr =
                                &ioList[j];
        if (j == 1){
            sprintf((char *)ioList[j].aiocbp->aio_buf,"%s","   First WR to file1");
            s = aio_write(ioList[j].aiocbp);
            if (s == -1)
                errExit("aio_write");
        }
        else{
            s = aio_read(ioList[j].aiocbp);
            if (s == -1)
            errExit("aio_read");
        }
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
        ssize_t s;

        s = aio_return(ioList[j].aiocbp);
        printf("    for request %d (descriptor %d), size is %zd and ",
                j, ioList[j].aiocbp->aio_fildes, s);
        if (s > -1){
            if (j == 1){
                        printf("Succeeded writing of %s\n",(char *)ioList[j].aiocbp->aio_buf);
                }
            else
                printf("Result of reading is : %s\n",(char *)ioList[j].aiocbp->aio_buf);
        }
    }

    exit(EXIT_SUCCESS);
}