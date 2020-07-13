#include <stdio.h>
//#include <stdlib.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>   /* For constants ORIG_EAX etc */

int main()
{   pid_t child;
    long systemCall_number; //orig_eax;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    }
    else {
        wait(NULL);
	printf("Start Tracing...\n");
        systemCall_number = ptrace(PTRACE_PEEKUSER,   // /usr/include/x86_64-linux-gnu/asm/unist_64.h
                          child, 8 * ORIG_RAX,
                          NULL);
        printf("I am the parent and the child made its first system call: %ld\n", systemCall_number);
	printf("Tracing is finished\n");
        ptrace(PTRACE_CONT, child, NULL, NULL);
    }
    return 0;
}
