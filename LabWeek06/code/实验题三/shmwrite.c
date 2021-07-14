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
    struct shared_struct *shared = NULL;
    int shmid;
    key_t key;

    char buffer[BUFSIZ + 1]; /* 8192bytes, saved from stdin */
    
    sscanf(argv[1], "%x", &key);

    printf("shmwrite: IPC key = %x\n", key);

    shmid = shmget((key_t)key, TEXT_NUM*sizeof(struct shared_struct), 0666|PERM);
    if (shmid == -1) {
        ERR_EXIT("shmwite: shmget()");
    }

    shmptr = shmat(shmid, 0, 0);
    if(shmptr == (void *)-1) {
        ERR_EXIT("shmwrite: shmat()");
    }
    printf("shmwrite: shmid = %d\n", shmid);
    printf("shmwrite: shared memory attached at %p\n", shmptr);
    printf("shmwrite precess ready ...\n");
    //将内存地址赋值给队列  
    Queue *q= (Queue*)shmptr;   
    //初始化队列
    if(q->firstbuild!=1)
    	{
    		init_queue(q);
    		printf("队列初始化\n");
    	}
    while (1) {
        //写的临界区不可用或者队列已满 
        while (q->writelock == 0||q->fullnum==TEXT_NUM) {
            if(q->fullnum==TEXT_NUM)
            	printf("The queue is full\n");
            usleep(rand()%10000);//因队列满进程被阻塞0us到10ms之间的一个随机数，等待消费者消费      
            printf("waiting~\n"); 
        }
        q->emptynum=q->emptynum-1;//申请一个空的单元             
	    q->writelock=0;//占用写的临界区内存空间,这是为了实现同步互斥
        printf("Enter some text: ");
        fgets(buffer, BUFSIZ, stdin);//获取数据到缓冲区
        enqueue(q,buffer,TEXT_SIZE);  //向写临界区（队尾）中写入数据
        q->writelock = 1;//释放写的临界区内存空间         
        q->fullnum=q->fullnum+1; //队尾增加一个存放数据的单元
        //printf("shared buffer: %s\n",shared->mtext);
        //如果存放数据的单元既是队头又是队尾
        //则在写操作完毕后释放读的临界区内存资源
        if(q->fullnum==1)
           q->readlock=1;   
        usleep(rand()%10000); //执行写操作一次后，进程被阻塞0us到10ms之间的一个随机数，CPU运行其他写进程
        //输入了end，退出循环（程序）
        if(strncmp(buffer, "end", 3) == 0) {
            break;
        }
    }
       /* detach the shared memory */
    if(shmdt(shmptr) == -1) {
        ERR_EXIT("shmwrite: shmdt()");
    }

//    sleep(1);
    exit(EXIT_SUCCESS);
}
