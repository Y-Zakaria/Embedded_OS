#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>


#define SIG SIGRTMIN
#define CLOCKID CLOCK_REALTIME

struct timespec tp;

unsigned long sig_triger_time[100];
int count = 0;

long period;

static void handler()
{
	if (clock_gettime(CLOCKID, &tp) == 0){
		//sig_triger_time[count] = (double) tp.tv_nsec/100000 + (double) tp.tv_sec*1000;
		sig_triger_time[count] = tp.tv_sec*1000000000 + tp.tv_nsec;	// ns		
		//printf("temps %d : %ld\n",count,sig_triger_time[count]);
		count++;
	}
}

void time_difference_analyse(){
	int j =0;
	unsigned long difference_time[99];
	double moyenne = 0.0;
	double ecart_type = 0.0;
	FILE *fptr ;
    	fptr = fopen("jitter_mesure.txt", "a+") ;
	
      
    	if ( fptr == NULL ) 
    	{ 
        	printf( "failed to open test.txt." ) ; 
    	} 
    	else
    	{
		fprintf(fptr,"********* mesure de gigue pour une periode de %ld ****************\n",period);
		for (j=0; j<99; j++){
			difference_time[j] = sig_triger_time[j+1] - sig_triger_time[j];
			moyenne += difference_time[j];
			//fprintf(fptr, "difference de lancement entre handler %d et %d est : %ld\n",j+1,j,difference_time[j]);
			fprintf(fptr, "%ld\n",difference_time[j]);
		} 

		moyenne = moyenne/99;
		
		for (j=0; j<99; j++){
			ecart_type += pow((difference_time[j] - moyenne),2);
		} 
		ecart_type = sqrt(ecart_type/99);
		fprintf(fptr, "la moyenne des differences de lancement est : %f\n",moyenne);
		fprintf(fptr, "l'ecart type  des differences de lancement est : %f\n\n",ecart_type);
        	fclose(fptr);
	}
	printf("la moyenne des differences de lancement est : %f\n",moyenne);	
	printf("l'ecart type  des differences de lancement est : %f\n",ecart_type);
}


int main(int argc, char *argv[]){
	cpu_set_t set;
	struct sched_param param = { .sched_priority = 10 };
 	pid_t pid = getpid();
	char data[200];

	clockid_t clockid;
	struct sigevent evp;
	timer_t timerid;
	
	struct itimerspec itimsp;

	sigset_t mask;
        struct sigaction sa;

        if (argc < 2)
               fprintf(stderr, "passez une periode en argument!\n");



	/*int prio = getpriority(PRIO_PROCESS,pid);
	printf("priorite statique = %d\n", prio);
	

	switch (sched_getscheduler(pid)){
		case 0 : printf("la politique d'ordonancemet est 0 (SCHED_OTHER)\n"); break;
		case 1 : printf("la politique d'ordonancemet est 1 (SCHED_FIFO)\n");  break;
		case 2 : printf("la politique d'ordonancemet est 2 (SCHED_RR)\n");    break;
		default : abort();
	}

 	if (setpriority(PRIO_PROCESS,pid,-2) == 0){
		printf("la priorite devient = %d\n", getpriority(PRIO_PROCESS,pid));
	}

	char filename[50] = "/proc/";
	FILE *fptr;


	fptr = fopen("/proc/self/sched","r");
	if(fptr != NULL){
		printf("en explorant /self/sched:\n");
		while (fgets(data,200,fptr) != NULL){
			if(strstr(data,"prio") != NULL || strstr(data,"policy") != NULL)
				printf(" %s\n", data);
		}
		fclose(fptr);
	}
	else
		printf("impossible d'ouvrir le fichier\n");

	/* CPUs in which it is elligebal to run the process */
       // if(sched_getaffinity(pid, sizeof(set), &set) == 0)

	/*real-time process*/
	/*if(sched_setscheduler(pid,SCHED_FIFO,&param) == -1) // "/bits/sched.h"
    		printf("une erreur de type : \" %s \"\n", strerror(errno));	
	else
		printf("la priorite de process temps reel est : %d\n",param.sched_priority);

/**** mesurer le temps gigue *********/

  /***** creation de timer ******/
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIG;
	//evp.sigev_value.sival_ptr = &timerid;
	
	if(timer_create(CLOCKID,&evp,&timerid) == -1)
		printf("erreur en creation de timer\n");	
	else
		printf("timer id = %lx\n", (long) timerid);

	period = atoll(argv[1]);
	itimsp.it_value.tv_sec = 0;
	itimsp.it_value.tv_nsec = period;

	itimsp.it_interval.tv_sec = itimsp.it_value.tv_sec;
	itimsp.it_interval.tv_nsec = itimsp.it_value.tv_nsec;

	if(timer_settime(timerid,0,&itimsp,NULL) == -1)
		printf("la configuration de timer a échoué\n");


   	/***** signal handler ******/
        sa.sa_sigaction = handler;
        if (sigaction(SIG, &sa, NULL) == -1)
               	printf("sigaction error\n"); 

	while (count <100);
	time_difference_analyse();


	return 0;	
}
