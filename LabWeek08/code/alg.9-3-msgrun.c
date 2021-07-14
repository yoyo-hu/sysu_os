#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/msg.h>

#include <sys/stat.h>

#include <fcntl.h>

 

#include "alg.9-0-msgdata.h"

int main(int argc, char *argv[])
{
    char pathname[80];
    struct stat fileattr;
    key_t key;
    struct msg_struct data;
    long int msg_type;
    char buffer[TEXT_SIZE];
    int msqid, ret, count = 0;
    FILE *fp;

    if(argc < 2) {
        printf("Usage: ./a.out pathname\n");
        return EXIT_FAILURE;
    }
    strcpy(pathname, argv[1]);

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
    
    msqid = msgget((key_t)key, 0666 | IPC_CREAT);
    if(msqid == -1) {
        ERR_EXIT("msgget()");
    }
 
    pid_t childpid1, childpid2;
    data.lock=1;//表示进程可访问
    char *argv1[] = {" ", argv[1], 0};
    childpid1 = vfork();//创立子进程
    if(childpid1 < 0) {
        ERR_EXIT("shmpthreadcon: 1st vfork()");
    } 
    else if(childpid1 == 0) {
        execv("./alg.9-1-msgsnd.o", argv1); /* call shmproducer with filename */
        //子进程则调用生产者函数
    }
    else {
        childpid2 = vfork();
        if(childpid2 < 0) {
            ERR_EXIT("shmpthreadcon: 2nd vfork()");
        }
        else if (childpid2 == 0) {
            execv("./alg.9-2-msgrcv.o", argv1); 
            //子进程则调用消费者函数
            /* call shmconsumer with filename */
        }
        else {
            wait(&childpid1);
            wait(&childpid2);

        }
    }
    exit(EXIT_SUCCESS);
}


