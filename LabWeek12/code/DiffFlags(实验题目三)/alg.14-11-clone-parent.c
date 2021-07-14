#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <unistd.h>
#define gettid() syscall(__NR_gettid)
  /* wrap the system call syscall(__NR_gettid), __NR_gettid = 224 */
#define gettidv2() syscall(SYS_gettid) /* a traditional wrapper */

#define STACK_SIZE 1024*1024 /* 1Mib. question: what is the upperbound of STACK_SIZE */

static int child_func()
{
    printf("I am child_func, my parent PID = %ld\n", (long)getppid());
    sleep(1);
    return 0;
}

int main(int argc,char **argv)
{
    printf("./a.out or ./a.out parent");
    //为子进程申请系统堆栈
    char *stack = malloc(STACK_SIZE*sizeof(char)); /* allocating from heap, safer than stack[STACK_SIZE] */
    if(!stack) {
        perror("malloc()");
        exit(1);
    }

    unsigned long flags = 0;
    /* set CLONE flags */
    if((argc > 1) && (!strcmp(argv[1], "parent"))) {
        flags |= CLONE_PARENT;//子进程与父进程运行于相同的内存空间
    }

    printf("I am parent, my parent PID = %ld\n", (long)getppid());
    sleep(1);
    printf("parent clone ...\n");
    
    /* creat child thread, top of child stack is stack+STACK_SIZE */
    pid_t chdtid = clone(child_func, stack + STACK_SIZE, flags | SIGCHLD, NULL); /* what happened if without SIGCHLD */
    if(chdtid == -1) {
        perror("clone1()");
        exit(1);
    }

    printf("parent waiting ... \n");

    int status = 0;
    waitpid(-1, &status, 0);

    free(stack);
    stack = NULL;
    return 0;
}

