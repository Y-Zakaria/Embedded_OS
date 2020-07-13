#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*cpuinfo*/

void cpu_info(){
  int count = 0;
  FILE *fptr;
  char *filename = "/proc/cpuinfo";
  char data[200];

  printf("cpuinfo:\n");
  fptr = fopen(filename,"r");
  while(fgets(data,200,fptr) != NULL){
	if((strstr(data,"model name") != NULL && count ==0) || 
 	   (strstr(data,"cpu MHz") != NULL && count ==1) || 
           (strstr(data,"cache size") != NULL && count ==2) || 
           (strstr(data,"address sizes") != NULL && count ==3) ){
	  printf("%s", data);
	  count++;}
  }
  fclose(fptr);
  count = 0;
}

/*meminfo*/

void memory_info(){
  FILE *fptr;
  char *filename = "/proc/meminfo";
  char data[200];

  fptr = fopen(filename,"r");
  printf("\nmeminfo:\n");
  while(fgets(data,200,fptr) != NULL){
	if((strstr(data,"MemTotal") != NULL ) || strstr(data,"MemAvailable") != NULL){
	  printf("%s", data);
	}
  }
  fclose(fptr);
}

/*partitions*/

void partitions_info(){
  int count = 0;
  FILE *fptr;
  char *filename= "/proc/partitions";
  char data[200];
  char *ftokenptr; 

  fptr = fopen(filename,"r");
  printf("\npartitions:\n");
  while(fgets(data,200,fptr) != NULL ){
	ftokenptr = strtok(data," ");
	while(ftokenptr != NULL){
	  if(++count > 2){
		printf("%s ",ftokenptr);
	  }
	  ftokenptr = strtok( NULL," ");
	}
	count = 0;	
  }
  fclose(fptr);
}

/*version*/

void os_info(){
  FILE *fptr;
  char *filename= "/proc/version";
  char data[200]; 

  fptr = fopen(filename,"r");
  printf("\nversion:\n");
  while(fgets(data,200,fptr) != NULL ){
	  printf("%s", data);
  }
  fclose(fptr);
}

/*** processus info ***/

void display_cmdline(char *pid){
	char filename[50] = "/proc/";
	FILE *fptr;
	char data[200];

	fptr = fopen(strcat(strcat(filename,pid),"/cmdline"),"r");
	if(fptr != NULL){
		while (fgets(data,200,fptr) != NULL){
			printf("la commande exécutée de pid:%s : %s\n",pid,data);
		}
		fclose(fptr);
	}
	else
		printf("File could not be opened\n");
}

void display_process_info(char pid[]){
	char filename[50] = "/proc/";
	char *status = "/status";
	FILE *fptr;
	char *tokenptr;
	char tempid[10];
	char data[200];
	printf("\nProcessus Info :\n");

	display_cmdline(pid);


	//scanf("%s\n",pid);
	/*while(filename[i] != '\0')
		i++;
	while ((filename[i++] = pid[j++]) != '\0');*/
	strcpy(filename,"/proc/");
	strcat(strcat(filename,pid),status);
	fptr = fopen(filename,"r");
	if(fptr != NULL){
		while (fgets(data,200,fptr) != NULL){
			if(strstr(data,"PPid") != NULL){
				printf("%s",data);
				tokenptr = strtok(data,"PPid:");
				strcpy(tempid,++tokenptr);
				tempid[strlen(tempid)-1] = '\0';
				//printf("size = %zu and ppid = %s", strlen(tempid), tempid);
				display_cmdline(tempid);
			}
			if((strstr(data,"Pid") != NULL && strstr(data,"TracerPid") == NULL && strstr(data,"PPid") == NULL) ||
				(strstr(data,"Tgid") != NULL) ||
				(strstr(data,"Threads") != NULL) ||
				(strstr(data,"Uid") != NULL) ||
				(strstr(data,"Gid") != NULL) ){
				printf("%s",data);
			}	
		}
		fclose(fptr);
	}
	else
		printf("File could not be opened\n");
}

/*int main(){
  char pid[10] = "\0";
  memory_info();
  partitions_info();
  os_info();

  printf("enter the processus pid:");
  scanf("%s",pid);
  display_process_info(pid);

  return 0;
}*/


int main (int argc, char **argv)
{
  char *pid = NULL;
  int index;
  int c;

  opterr = 0;


  while ((c = getopt (argc, argv, "cmbvp:")) != -1)
    switch (c)
      {
      case 'c':
        cpu_info();
        break;
      case 'm':
	memory_info();
        break;
      case 'b':
	partitions_info();
        break;
      case 'v':
	os_info();
        break;
      case 'p':
        pid = optarg;
	display_process_info(pid);
        break;
      case '?':
        if (optopt == 'p')
          fprintf (stderr, "Option -%c requires an argument pid\n", optopt);
        else if (isprint (optopt)) // check for any printable character including space
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }

  for (index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);
  return 0;
}

/*major minor  #blocks  name

   8        0  488386584 sda
   8        1     460800 sda1
   8        2     102400 sda2
   8        3      16384 sda3
   8        4  204188719 sda4
   8        5     990208 sda5
   8        6       1024 sda6
   8        7  187051152 sda7
   8        8   91458560 sda8
   8        9    4112384 sda9
  11        0    1048575 sr0*/


