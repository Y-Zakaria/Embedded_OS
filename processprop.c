#include <stdio.h>
#include <string.h>

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


int main(){
	char pid[10] = "\0";
	//int pid = 0;
	printf("enter the processus pid:");
	scanf("%s",pid);
	display_process_info(pid);

	return 0;
}
