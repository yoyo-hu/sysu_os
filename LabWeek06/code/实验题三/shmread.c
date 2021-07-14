#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/shm.h>

#include "shmdata.h"
int main(int argc, char *argv[])
{
    void *shmptr = NULL;
    struct shared_struct *shared;
    int shmid;
    key_t key;
 
    sscanf(argv[1], "%x", &key);
    printf("%*sshmread: IPC key = %x\n", 30, " ", key);
    
    shmid = shmget((key_t)key, TEXT_NUM*sizeof(struct shared_struct), 0666|PERM);
    if (shmid == -1) {
        ERR_EXIT("shread: shmget()");
    }

    shmptr = shmat(shmid, 0, 0);
    if(shmptr == (void *)-1) {
        ERR_EXIT("shread: shmat()");
    }
    printf("%*sshmread: shmid = %d\n", 30, " ", shmid);    
    printf("%*sshmread: shared memory attached at %p\n", 30, " ", shmptr);
    printf("%*sshmread process ready ...\n", 30, " ");
    
    //shared = (struct shared_struct *)shmptr;
    //将内存地址赋值给队列  
    Queue *q= (Queue*)shmptr;   
    char ret[TEXT_SIZE];
    //持续的读，等待读
    while (1) {
        //当不能读或者信息还没有准备好的时候，等待信息到来
        while (q->readlock!=1||q->fullnum==0) {
            sleep(1); /* message not ready, waiting ... */
        }
        dequeue(q, ret); //消费者将内容从队头读出
        //读到结束标志
        if (strncmp(ret, "end", 3) == 0) {
            q->readlock=1; // 释放读的临界区内存空间
            q->fullnum=q->fullnum-1;//队列减少一个存放数据的内存单元
            break;
        }
        printf("\nYou wrote: %s\n",  ret);
        q->readlock=1; // 释放读的临界区内存空间
        q->emptynum=q->emptynum+1;
	q->fullnum=q->fullnum-1;//队列减少一个存放数据的内存单元
       // usleep(rand()%10000); //进程被阻塞0us到10ms之间的一个随机数,CPU运行其他消费者进程
    } /* it is not reliable to use shared->written for process synchronization */
     
   if (shmdt(shmptr) == -1) {
        ERR_EXIT("shmread: shmdt()");
   }
 //control shmctl()进行共享内存段的删除
    sleep(1);
    exit(EXIT_SUCCESS);
}
c