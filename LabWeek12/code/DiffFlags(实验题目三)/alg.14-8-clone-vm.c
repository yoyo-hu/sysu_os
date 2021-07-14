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
static int child_func(void *arg)
{
    char *chdbuf = (char*)arg; /* type casting */
    printf("child_func read buf: %s\n", chdbuf);

    sprintf(chdbuf, "I am child_func, my tid = %ld, pid = %d", gettid(), getpid());
    printf("child_func set buf: %s\n", chdbuf);
    sleep(1);
    return 0;
}
int main(int argc,char **argv)
{
    printf("./a.out or ./a.out vm\n");
    //为子进程申请系统堆栈
    char *stack = malloc(STACK_SIZE*sizeof(char)); /* allocating from heap, safer than stack[STACK_SIZE] */
    pid_t chdtid;
    char buf[100]; /* a global variable has the same behavior */
    if(!stack) {
        perror("malloc()");
        exit(1);
    }

    unsigned long flags = 0;
      /* set CLONE flags */
    if((argc > 1) && (!strcmp(argv[1], "vm"))) {
        flags |= CLONE_VM;//子进程与父进程运行于相同的内存空间
    }

    sprintf(buf,"I am parent, my pid = %d", getpid());
    printf("parent set buf: %s\n", buf);
    sleep(1);

    printf("parent clone ...\n");
      /* creat child thread, top of child stack is stack+STACK_SIZE */
    chdtid = clone(child_func, stack + STACK_SIZE, flags | SIGCHLD, buf); /* what happened if without SIGCHLD */
    if(chdtid == -1) {
        perror("clone1()");
        exit(1);
    }

    printf("parent waiting ... \n");
    int status = 0;
    if(waitpid(-1, &status, 0) == -1) { /* wait for any child existing, may leave some child defunct */
        perror("wait()");
    }
    
    sleep(1);
    printf("parent read buf: %s\n", buf);
    free(stack);
    stack = NULL;
    return 0;
}
