#define _GNU_SOURCE
#define PERM S_IRUSR|S_IWUSR|IPC_CREAT
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
  /* wrap the system call syscall(__NR_gettid), __NR_gettid = 224 */
#define gettidv2() syscall(SYS_gettid) /* a traditional wrapper */
#define STACK_SIZE 1024*1024 /* 1Mib. question: what is the upperbound of STACK_SIZE */
static int child_func(void *arg)
{
    sleep(1);
    printf("I am child_func, my tid = %ld, pid = %d\n", gettid(), getpid());
    //使用ipcs -q打印出使用消息队列进行进程间通信的信息
    system("ipcs -q");
    sleep(1);
    return 0;
}
#define TEXT_SIZE 512
struct msg_struct {//消息的结构
    long int msg_type;//消息的类型
    char mtext[TEXT_SIZE]; /* binary data */
    //消息数据
};
int main(int argc, char *argv[])
{
    printf("./a.out or ./a.out newipc\n");
    char pathname[80];
    struct stat fileattr;
    key_t key;
    struct msg_struct data;
    long int msg_type=1;//表示消息的类型
    int msqid, ret;
    //为子进程申请系统堆栈
    char *stack = malloc(STACK_SIZE*sizeof(char)); /* allocating from heap, safer than stack[STACK_SIZE] */
    pid_t chdtid;
    if(!stack) {
        perror("malloc()");
        exit(1);
    }
    unsigned long flags = 0;
    /* set CLONE flags */
    if((argc > 1) && (!strcmp(argv[1], "newipc"))) {
       flags |= CLONE_NEWIPC;
    }
    printf("I am parent, my pid = %d\n", getpid());
    strcpy(pathname, "1");
    if(stat(pathname, &fileattr) == -1) {
        ret = creat(pathname, O_RDWR);
        if (ret == -1) {
            ERR_EXIT("creat()");
        }
        printf("shared file object created\n");
    }
    key = ftok(pathname, 0x27); /* project_id can be any nonzero integer */
    if(key < 0) {
        ERR_EXIT("ftok()");
    }
    printf("\nIPC key = 0x%x\n", key);	
    printf("parent clone ...\n");
    /* creat child thread, top of child stack is stack+STACK_SIZE */
    chdtid = clone(child_func, stack + STACK_SIZE, flags | SIGCHLD, NULL); /* what happened if without SIGCHLD */
    if(chdtid == -1) {
        perror("clone()");
        exit(1);
    }
    //建立消息队列
    msqid = msgget((key_t)key, 0666 | IPC_CREAT);//IPC_CREAT表示创建一个新的消息队列，0666表示给全部的读写权限
    if(msqid == -1) {
        //消息队列建立失败
        ERR_EXIT("msgget()");
    }
    struct msqid_ds msqattr;//msgid_ds结构:消息队列的内部数据结构
    //把msgid_ds结构中的数据设置为消息队列的当前关联值
    ret = msgctl(msqid, IPC_STAT, &msqattr);
    //打印当前处于队列中的消息数目，以及可以插入的剩余消息数目
    printf("number of messages remainded = %ld, empty slots = %ld\n", msqattr.msg_qnum, 16384/TEXT_SIZE-msqattr.msg_qnum);
    printf("Blocking Sending ... \n");             
    data.msg_type = msg_type;
    strcpy(data.mtext, "hello world!");
	//表示向标识符为msqid的消息队列发送大小为TEXT_SIZE的消息data
    ret = msgsnd(msqid, (void *)&data, TEXT_SIZE, 0); /* 0: blocking send, waiting when msg queue is full */    
    if(ret == -1) {
        ERR_EXIT("msgsnd()");
    }
   	//使用ipcs -q打印出使用消息队列进行进程间通信的信息
    system("ipcs -q");
    int status = 0;
    if(waitpid(-1, &status, 0) == -1) { /* wait for any child existing, may leave some child defunct */
        perror("wait()");
    }
    sleep(1);
    //删除队列
    if(msgctl(msqid, IPC_RMID, 0) == -1)
        perror("msgctl(IPC_RMID)");
        printf("parent waiting ... \n");
    free(stack);
    stack = NULL;
    return 0;
}