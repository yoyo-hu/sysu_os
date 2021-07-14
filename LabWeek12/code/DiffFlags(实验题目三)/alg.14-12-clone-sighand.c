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
static char stack[1048576];
#define STACK_SIZE 1024*1024
void int_child_handler(){
     printf("int_child_handler:Get a SIGINT signal!\n"); 
}
void int_parent_handler(){
     printf("int_parent_handler:Get a SIGINT signal!\n"); 
}
static int child_func() {
	printf( "I am child, my tid = %ld, pid = %d\n", gettid(), getpid());
	printf("Set SIGINT to trigger the execution of int_child_handler()\n");
	signal( SIGINT, int_child_handler);
	return 0;
}
int main(int argc,char **argv) {
     printf("./a.out or ./a.out sighand\n");
     unsigned long flags = 0;
      /* set CLONE flags */
     if((argc > 1) && (!strcmp(argv[1], "sighand"))) {
        flags |= CLONE_SIGHAND;
     }
	printf("I am parent, my pid = %d\n", getpid());
	printf("Set SIGINT to trigger the execution of int_parent_handler()\n");
	signal( SIGINT, int_parent_handler);
	printf("input 'ctrl+c' to call handler\n");
	printf("input 'ctrl+\\' to quit\n");
	pause(); 
     printf("parent clone ...\n");
     pid_t child_pid = clone(child_func, stack+STACK_SIZE, flags| CLONE_VM | SIGCHLD, NULL);
	printf("parent waiting ... \n");
     waitpid(child_pid, NULL, 0);
     printf("child terminated!\n");
     printf("input 'ctrl+c' to call handler\n");
	printf("input 'ctrl+\\' to quit\n");
	pause();
     printf("parent terminated!\n");
     return 0;
}