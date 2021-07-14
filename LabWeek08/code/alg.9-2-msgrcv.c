#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/msg.h>

#include <sys/stat.h>



#include "alg.9-0-msgdata.h" 



int main(int argc, char *argv[]) /* Usage: ./b.out pathname msg_type */

{


    key_t key;

    struct stat fileattr;

    char pathname[80];

    int msqid, ret, count = 0;

    struct msg_struct data;

    long int msgtype = 0;   /* 0 - type of any messages */



    if(argc < 2) {

        printf("Usage: ./b.out pathname msg_type\n");

        return EXIT_FAILURE;

    }

    strcpy(pathname, argv[1]);

    if(stat(pathname, &fileattr) == -1) {

        ERR_EXIT("shared file object stat error");

    }

    if((key = ftok(pathname, 0x27)) < 0) {

        ERR_EXIT("ftok()");

    }

    printf("\nIPC key = 0x%x\n", key);



//    msqid = msgget((key_t)key, 0666 | IPC_CREAT);

    msqid = msgget((key_t)key, 0666); /* do not create a new msg queue */

    if(msqid == -1) {

        ERR_EXIT("msgget()");

    }

 

    if(argc < 3)

        msgtype = 0;

    else {

        msgtype = atol(argv[2]);

        if (msgtype < 0)

            msgtype = 0;

    }    /* determin msgtype (class number) */

    printf("Selected message type = %ld\n", msgtype);



    while (1) {
	while(data.lock==1){
	 sleep(1);
	}
	wait(0);

        ret = msgrcv(msqid, (void *)&data, TEXT_SIZE, msgtype, IPC_NOWAIT); /* Non_blocking receive */

        if(ret == -1) { /* end of this msgtype */

            printf("number of received messages = %d\n", count);

            break;

        }

        

        printf("%ld %s\n", data.msg_type, data.mtext);

        count++;
	printf("reserve the %dth message\n", count);

	system("ipcs -q");
	//休眠4s，尽量避免多进程冲突，同时方便测试
        sleep(4);//设置不同的休眠数值还可以有效的区分开收发的速度
        //将逻辑值设为0，其它进程可进入共享内存执行
        data.lock = 0;



    }

    

    struct msqid_ds msqattr;

    ret = msgctl(msqid, IPC_STAT, &msqattr);

    printf("number of messages remainding = %ld\n", msqattr.msg_qnum); 



    if(msqattr.msg_qnum == 0) {

        printf("do you want to delete this msg queue?(y/n)");

        if(getchar() == 'y') {

            if(msgctl(msqid, IPC_RMID, 0) == -1)

                perror("msgctl(IPC_RMID)");

        }

    }

   

    system("ipcs -q");

    exit(EXIT_SUCCESS);

}