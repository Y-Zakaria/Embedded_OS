#include <stdio.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <x86_64-linux-gnu/sys/user.h>
#include <sys/syscall.h>   /* For SYS_write etc */
int main()
{   pid_t child;
    long orig_eax, eax;
    long params[3];
    int status;
    int insyscall = 0;
    struct user_regs_struct regs;

    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/home/zakaria2/Desktop/TP1/writeToFile", "writeToFile", NULL);
	//execl("/bin/ls", "ls", NULL);
    }
    else {
       while(1) {
          wait(&status);
          if(WIFEXITED(status))
              break;
          orig_eax = ptrace(PTRACE_PEEKUSER,
                     child, 8 * ORIG_RAX, NULL);
          if(orig_eax == SYS_write) { //SYS_execve  -- /usr/include/x86_64-linux-gnu/bits/syscall.h
             if(insyscall == 0) {
                /* Syscall entry */
                insyscall = 1;
		ptrace(PTRACE_GETREGS, child,
                        NULL, &regs);

                /*params[0] = ptrace(PTRACE_PEEKUSER,
                                   child, 8 * RBX,
                                   NULL);
                params[1] = ptrace(PTRACE_PEEKUSER,
                                   child, 8 * RCX,
                                   NULL);
                params[2] = ptrace(PTRACE_PEEKUSER,
                                   child, 8 * RDX,
                                   NULL);*/

                printf("Write called with "
                       "%llu, %llu, %llu\n",
                       regs.rbx, regs.rcx,
                       regs.rdx);
                }
          else { /* Syscall exit */
                eax = ptrace(PTRACE_PEEKUSER,
                             child, 8 * RAX, NULL);
                    printf("Write returned "
                           "with %ld\n", eax);
                    insyscall = 0;
                }
            }
          
            ptrace(PTRACE_SYSCALL,
                   child, NULL, NULL);
        }
    }
    return 0;
}
