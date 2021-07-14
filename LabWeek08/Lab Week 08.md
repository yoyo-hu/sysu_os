# Lab Week 08.

## 实验内容1：进程间通信—消息机制。

### 一. 编译运行课件 Lecture 09 例程代码： Algorithms 9-1 ~ 9-2.

### 相关知识点：

消息队列提供了一种从一个进程向另一个进程发送一个数据块的方法，

它的内部数据结构是msgid_ds结构。对于系统上创建的每个消息队列，内核均为其创建、存储和维护该结构的一个实例。该结构在Linux/msg.h中定义，如下所示。

```C
struct msgid_ds{
	struct ipc_perm msg_perm;
    /*它是ipc_perm结构的一个实例，ipc_perm结构是在Linux/ipc.h中定义的。用于存放消息队列的许可权限信息，其中包括访问许可信息，以及队列创建者的有关信息（如uid等）。*/
	time_t  msg_stime; /*发送到队列的最后一个消息的时间戳*/
    time__t  msg__rtime;    /*从队列中获取的最后一个消息的时间戮*/
    time_t  msg_ctime; /*对队列进行最后一次变动的时间戳*/
    unsigned long  _msg_cbytes;      /*在队列上所驻留的字节总数*/
    msgqnum_t  msg_qnum;    /*当前处丁队列中的消息数目*/
    msglen_t  msg_qbytes;   /*队列中能容纳的字节的最大数目*/
    pid_t  msg_lspid;   /*发送最后一个消息进程的PID */
    pid_t  msg_lrpid;   /*接收最后一个消息进程的PID */
   };
```

  上面成员中ipc_perm的介绍：

内核把IPC对象的许可权限信息存放在ipc_perm类型的结构中。例如某些消息队列的内部结构中，msg_perm成员就是ipc_perm类型的，它的定义是在文件 <linux/ipc.h>中，如下所示：

 

```c
struct ipc_perm{
    key_t key;           /*函数msgget ()使用的键值,用于区分消息队列*/
    uid_t uid;           /*用户的UID*/
    gid_t gid;           /*用户的GID*/
    uid_t cuid;          /*建立者的UID*/
    gid_t cgid;          /*建立者的GID*/
    unsigned short mode; /*权限,用户控制读写，例如0666,可以对消息进行读写操作。*/
    unsigned short seq;  /*序列号*/
};
```


  消息队列提供了以下的函数方便我们调用：

  1. **msgsnd()函数:**用来创建和访问一个消息队列。函数原型如下：

     ```c
     int msgget(key_t, key, int msgflg);
     /*函数参数说明*/
     //msgflg -> 一个权限标志，表示消息队列的访问权限。
     //它返回消息队列的标识符key，失败时返回-1.
     ```

  2. **msgsnd()函数:**用来把消息添加到消息队列中.函数原型如下：

     ```c
     int msgsnd(int msgid, const void *msg_ptr, size_t msg_sz, int msgflg);
     /*函数参数说明*/
     //msgid -> 由msgget函数返回的消息队列标识符
     
     //msg_ptr -> 一个指向准备发送消息的指针，指针msg_ptr所指向的消息结构一定要是以一个长整型成员变量开始的结构体，接收函数将用这个成员来确定消息的类型。
     //消息结构要定义：
     struct my_message {
         long int message_type;
         /* The data you wish to transfer */
     };
     //msg_sz 是msg_ptr指向的消息的长度，注意是消息的长度，而不是整个结构体的长度，也就是说msg_sz是不包括长整型消息类型成员变量的长度。
     
     //msgflg 用于控制当前消息队列满或队列消息到达系统范围的限制时将要发生的事情。
     
     //如果调用成功，消息数据的一份副本将被放到消息队列中，并返回0，失败时返回-1.
     ```

  3. **msgrcv()函数：**用来从消息队列获取消息，函数原型为：

     ```c
     int msgrcv(int msgid, void *msg_ptr, size_t msg_st, long int msgtype, int msgflg);
     /*函数参数说明*/
     //msgid, msg_ptr, msg_st -> 同 msgsnd()
     
     //msgtype -> 实现一种简单的接收优先级。当msgtype=0，就获取队列中的第一个消息；当msgtype>0,获取具有相同消息类型的第一个信息;当msgtype<0，获取类型等于或小于msgtype的绝对值的第一个消息。
     
     //msgflg -> 用于控制当队列中没有相应类型的消息时将发生的事情。
     
     //调用成功时，该函数返回放到接收缓存区中的字节数，消息被复制到由msg_ptr指向的用户分配的缓存区中，然后删除消息队列中的对应消息。失败时返回-1。
     ```

  4. **msgctl()函数:**用来控制消息队列，函数原型为：

     ```c
     nt msgctl(int msgid, int command, struct msgid_ds *buf);
     /*函数参数说明*/
     //msgid 标识符
     
     //command是将要采取的动作，它可以取3个值，
     /*
     IPC_STAT：把msgid_ds结构中的数据设置为消息队列的当前关联值，即用消息队列的当前关联值覆盖msgid_ds的值。
     IPC_SET：如果进程有足够的权限，就把消息列队的当前关联值设置为msgid_ds结构中给出的值
     IPC_RMID：删除消息队列
     */
     
     //buf是指向msgid_ds结构的指针，它指向消息队列模式和访问权限的结构。msgid_ds结构至少包括以下成员： 
     struct msgid_ds
     {
         uid_t shm_perm.uid;
         uid_t shm_perm.gid;
         mode_t shm_perm.mode;
     };
     //成功返回0.失败返回-1
     ```

     

### 源代码以及详细解释（注释中包含对代码执行过程的理解）

alg.9-0-magdata.h

```c
#define TEXT_SIZE 512
/* considering 
------ Messages Limits --------
max queues system wide = 32000
max size of message (bytes) = 8192
default max size of queue (bytes) = 16384
------------------------------------------
The size of message is set to be 512, the total number of messages is 16384/512 = 32
If we take the max size 8192, the number would be 16384/8192 = 2. It is not reasonable
*/

/* message structure */
struct msg_struct {//消息的结构
    long int msg_type;//消息的类型
    char mtext[TEXT_SIZE]; /* binary data */
    //消息数据
};

#define PERM S_IRUSR|S_IWUSR|IPC_CREAT

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

```

alg.9-0-msgsnd.txt

```
1 Luffy
1 Zoro
2 Nami
2 Usopo
1 Sanji
3 Chopper
4 Robin
4 Franky
5 Brook
6 Sunny
```

alg.9-1-msgsnd.c

```c
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
    long int msg_type;//表示消息的类型
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
    //建立消息队列
    msqid = msgget((key_t)key, 0666 | IPC_CREAT);//IPC_CREAT表示创建一个新的消息队列，0666表示给全部的读写权限
    if(msqid == -1) {
        //消息队列建立失败
        ERR_EXIT("msgget()");
    }
 	
    //使用可读写模式打开指定路径"./alg.9-0-msgsnd.txt"的文件
    fp = fopen("./alg.9-0-msgsnd.txt", "rb");
    if(!fp) {
        //打开文件失败
        ERR_EXIT("source data file: ./msgsnd.txt fopen()");
    }

    struct msqid_ds msqattr;//msgid_ds结构:消息队列的内部数据结构
    //把msgid_ds结构中的数据设置为消息队列的当前关联值
    ret = msgctl(msqid, IPC_STAT, &msqattr);
    //打印当前处于队列中的消息数目，以及可以插入的剩余消息数目
    printf("number of messages remainded = %ld, empty slots = %ld\n", msqattr.msg_qnum, 16384/TEXT_SIZE-msqattr.msg_qnum);
    
    printf("Blocking Sending ... \n");
    while (!feof(fp)) {//检测流上的文件结束符,当文件未被读取结束是，循环一直执行
        ret = fscanf(fp, "%ld %s", &msg_type, buffer);
        //从文件流fp中读取msg_type类型的消息到buffer中
        if(ret == EOF) {
            //读取到文件的最后，退出
            break;
        }
        //打印消息类型和消息内容
        printf("%ld %s\n", msg_type, buffer);
        //设置消息的类型字节数和数据              
        data.msg_type = msg_type;
        strcpy(data.mtext, buffer);
		//表示向标识符为msqid的消息队列发送大小为TEXT_SIZE的消息data
        ret = msgsnd(msqid, (void *)&data, TEXT_SIZE, 0); /* 0: blocking send, waiting when msg queue is full */
        
        if(ret == -1) {
            ERR_EXIT("msgsnd()");
        }
        count++;
    }
	//打印发送的消息数量
    printf("number of sent messages = %d\n", count);
	//关闭文件
    fclose(fp);
    
   	//使用ipcs -q打印出使用消息队列进行进程间通信的信息
    system("ipcs -q");
    exit(EXIT_SUCCESS);
}
```

alg.9-2-msgrcv.c

```c
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

	//msqid = msgget((key_t)key, 0666 | IPC_CREAT);
    //建立消息队列
    //当key所命名的消息队列不存在时创建一个消息队列，如果key所命名的消息队列存在时.0666表示给全部的读写权限
    //因为消息队列已经存在，在9-1程序中创立了，因此不需要重复创立
    msqid = msgget((key_t)key, 0666); /* do not create a new msg queue */
    if(msqid == -1) {
        //消息队列建立失败
        ERR_EXIT("msgget()");
    }
    //当命令行正确带有文件名时，将消息类型表记为0，否则命令的第三个词汇应该标记消息类型
    if(argc < 3)
        msgtype = 0;
    else {
        msgtype = atol(argv[2]);//把字符串转换成长整型数
        if (msgtype < 0)
            msgtype = 0;
    }    /* determin msgtype (class number) */
    printf("Selected message type = %ld\n", msgtype);

    while (1) {
        //从标识符为msqid的消息队列里按照参数msgtype的要求读取TEXT_SIZE大小的信息存放到data中，
        /*当msgtype=0 返回消息队列中第一条消息；
        当msgtype>0 返回消息队列中等于msgtyp类型的第一条消息；		当msgtype<0 返回msgtyp<=type 绝对值最小值的第一条消息。*/
        ret = msgrcv(msqid, (void *)&data, TEXT_SIZE, msgtype, IPC_NOWAIT); /* Non_blocking receive */
        //IPC_NOWAIT作为一个标志传送给该函数，而队列中没有任何消息，则该次调用将会向调用进程返回ENOMSG,没有这个参数进程就会阻塞
        if(ret == -1) { /* end of this msgtype */
            //读取失败，最后一次读取
            //输出读取的消息数目
            printf("number of received messages = %d\n", count);
            break;
        }
        //打印接收到的消息
        printf("%ld %s\n", data.msg_type, data.mtext);
        //读取的消息数目+1
        count++;
    }
    
    struct msqid_ds msqattr;//msgid_ds结构:消息队列的内部数据结构
    //把msgid_ds结构中的数据设置为消息队列的当前关联值
    ret = msgctl(msqid, IPC_STAT, &msqattr);
    //输出消息队列里面的消息个数
    printf("number of messages remainding = %ld\n", msqattr.msg_qnum); 
	//如果消息队列为空，判断是否要删除该信息队列
    if(msqattr.msg_qnum == 0) {
        printf("do you want to delete this msg queue?(y/n)");
        //输入流字母为y时，删除消息队列
        if(getchar() == 'y') {
            if(msgctl(msqid, IPC_RMID, 0) == -1)
                //删除队列失败
                perror("msgctl(IPC_RMID)");
        }
    }
   //使用ipcs -q打印出使用消息队列进行进程间通信的信息
    system("ipcs -q");
    exit(EXIT_SUCCESS);
}

```



### 执行结果分析：

1. 执行发送程序，可以观察到程序成功将文本中的10条消息按照文本的顺序发送到传送管道中。

   消息管道的key值为0x27019cda，标识符msqid为0，拥有者为用户moocos，权限perms为666表示可以读写，使用消息管道的比特数为5120，消息的数量为10个

   ![image-20210409101255717](http://hurq5.gitee.io/os-labwork/LabWeek08/pictures/image-20210409101255717.png)


2. 执行接收程序，当命令行没有第三个参数时，程序设置默认消息类型mytype为0，因此每次接收消息都接收消息队列的第一条消息，这样就可以无分别的接收所有消息，如图程序接收了消息队列中的所有10条消息，此时消息队列为空，因此会弹出是否需要删除消息队列（若消息队列不为空则不弹出该条判断），输入y删除消息队列，再打印所有的消息队列，可以看到消息队列列表为空。

   ![image-20210409103219897](http://hurq5.gitee.io/os-labwork/LabWeek08/pictures/image-20210409103219897.png)

3. 重新发送消息进行第二次测试：消息队列的标识符为32768

   ![image-20210409103839985](http://hurq5.gitee.io/os-labwork/LabWeek08/pictures/image-20210409103839985.png)

4. 执行接收程序，当命令行有第三个参数2，程序设置消息类型mytype为第三个参数2>0，因此每次接收消息接收消息队列的第一条类型为2的消息，这样就可以按顺序接收所有类型为2的消息，如图程序接收了消息队列中的所有类型为2的消息，消息队列中一共有两条，此时消息队列的消息数减少2，可以观察到标识符为32768的消息队列信息数为8。

   ![image-20210409104126442](http://hurq5.gitee.io/os-labwork/LabWeek08/pictures/image-20210409104126442.png)

5. 执行接收程序，当命令行有第三个参数1，程序设置消息类型mytype为第三个参数1>0，因此每次接收消息接收消息队列的第一条类型为1的消息，这样就可以按顺序接收所有类型为1的消息，如图程序接收了消息队列中的所有类型为1的消息，消息队列中一共有三条，此时消息队列的消息数减少3，可以观察到标识符为32768的消息队列信息数为5。

   ![image-20210409104713095](http://hurq5.gitee.io/os-labwork/LabWeek08/pictures/image-20210409104713095.png)

6. 执行接收程序，当命令行有第三个参数0，程序设置消息类型mytype为第三个参数0=0，因此每次接收消息都接收消息队列的第一条消息，这样就可以无分别的接收所有消息，如图程序接收了消息队列中的所有10条消息，此时消息队列为空，因此会弹出是否需要删除消息队列（若消息队列不为空则不弹出该条判断），输入y删除消息队列，再打印所有的消息队列，可以看到消息队列列表为空。

   ![image-20210409104801567](http://hurq5.gitee.io/os-labwork/LabWeek08/pictures/image-20210409104801567.png)

### 二. 修改代码,观察在 msgsnd 和 msgrcv 并发执行情况下消息 队列的变化情况。

### 修改内容：

1. 在头文件中，修改结构体，加入防止并发程序冲突的锁：

   当lock值为1的时候表示消息队列正在被访问，这个时候进程不能访问消息队列，当lock值为0的时候则表示消息队列可以被访问。

   ```c
   struct msg_struct {
       int lock;
   
       long int msg_type;
   
       char mtext[TEXT_SIZE]; /* binary data */
   
   };
   ```

2. 在发送程序的while循环的开始位置加入对进程是否可以访问消息队列的判断，即判断lock是否为1，如果为1则等待，如果不为1则标记为1并且给消息队列发送信息，并且在发动信息后加入printf（”Send the %dth message）显示这是在发送第几条消息，然后加入system("ipcs -q")，打印每次发送消息后消息管道的状态表：在while循环的最后更改lock值为0，表示消息队列可以被访问了，在设置lock为0之前，我们人为的让程序sleep3秒，这样可以使得程序不容易发生进程冲突，另一方面给接收程序sleep4秒，这样就可以有意的错开接收和发送的速度，更加符合真实的场境。

   发送程序的while循环如下所示：（有适当的添加注释增加程序的可读性）

   ```c
   while (!feof(fp)) {//检测流上的文件结束符,当文件未被读取结束是，循环一直执行
   	while(data.lock==1){//消息队列正在被接收进程访问，循环等待其访问结束
   	 sleep(1);
   	}
   	wait(0);
           ret = fscanf(fp, "%ld %s", &msg_type, buffer);//从文件流fp中读取msg_type类型的消息到buffer中
           if(ret == EOF) {
               //读取到文件的最后，退出
               break;
           }
       	//打印消息类型和消息内容
           printf("%ld %s\n", msg_type, buffer);
       	//设置消息的类型字节数和数据 
           data.msg_type = msg_type;
           strcpy(data.mtext, buffer);
       	//表示向标识符为msqid的消息队列发送大小为TEXT_SIZE的消息data
           ret = msgsnd(msqid, (void *)&data, TEXT_SIZE, 0); /* 0: blocking send, waiting when msg queue is full *///阻塞型发送，当消息队列满时，进程等待
           if(ret == -1) {
               //发送失败
               ERR_EXIT("msgsnd()");
           }
   		//count记录发送到第几条信息以及共发送了多少条信息
           count++;
       //输出发送到第几条信息
   	printf("Send the %dth message\n", count);
   	//打印消息队列状态表，方便参看消息实时增加
   	system("ipcs -q");
   	//休眠3s，尽量避免多进程冲突，同时方便测试
        sleep(3);
        //将逻辑值设为0，防止其它进程可进入共享内存执行
        data.lock = 0;
      
       }
   
   ```

3. 和发送程序的修改相同，接收程序程序也进行类似的修改，具体对while循环想修改如下：（有适当的添加注释增加程序的可读性）

   ```c
   while (1) {
       
   	while(data.lock==1){
   	 sleep(1);
   	}
   	wait(0);
       //从标识符为msqid的消息队列里按照参数msgtype的要求读取TEXT_SIZE大小的信息存放到data中，
           /*当msgtype=0 返回消息队列中第一条消息；
           当msgtype>0 返回消息队列中等于msgtyp类型的第一条消息；		
           当msgtype<0 返回msgtyp<=type 绝对值最小值的第一条消息。*/
       ret = msgrcv(msqid, (void *)&data, TEXT_SIZE, msgtype, IPC_NOWAIT); /* Non_blocking receive */ 
       //IPC_NOWAIT作为一个标志传送给该函数，而队列中没有任何消息，则该次调用将会向调用进程返回ENOMSG,没有这个参数进程就会阻塞
       if(ret == -1) { /* end of this msgtype */
           //读取失败，最后一次读取
           //输出读取的消息数目
            printf("number of received messages = %d\n", count);
            break;
       }
       //打印接收到的消息
   	printf("%ld %s\n", data.msg_type, data.mtext);
       //count记录接收到第几条信息以及共接收了多少条信息
       count++;
       //打印接收到第几条信息
       printf("reserve the %dth message\n", count);
       //打印消息状态表
   	system("ipcs -q");
   	//休眠4s，尽量避免多进程冲突，同时方便测试
       sleep(4);//设置不同的休眠数值还可以有效的区分开收发的速度
       //将逻辑值设为0，其它进程可进入共享内存执行
       data.lock = 0;
       }
   ```

4. 写一个程序使得发送程序和接收程序实现并发：注意因为程序调用发送和接收程序的时候是调用execv("./alg.9-1-msgsnd.o", argv1)和execv("./alg.9-2-msgrcv.o", argv1); 这两条语句，因此上面两个程序需要编译链接为可执行程序alg.9-1-msgsnd.o和alg.9-2-msgrcv.o。

   源代码以及注释如下：

   ```c
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
           //建立消息队列
       msqid = msgget((key_t)key, 0666 | IPC_CREAT);//IPC_CREAT表示创建一个新的消息队列，0666表示给全部的读写权限
       if(msqid == -1) {
           //消息队列建立失败
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
           execv("./alg.9-1-msgsnd.o", argv1); 
           //子进程则调用发送函数
       }
       else {
           childpid2 = vfork();
           if(childpid2 < 0) {
               ERR_EXIT("shmpthreadcon: 2nd vfork()");
           }
           else if (childpid2 == 0) {
               execv("./alg.9-2-msgrcv.o", argv1); 
               //子进程则调用接收函数
               /* call shmconsumer with filename */
           }
           else {
               wait(&childpid1);
               wait(&childpid2);
   			//等待两个进程都结束程序才退出
           }
       }
       exit(EXIT_SUCCESS);
   }
   ```

   

#### 执行结果以及分析：

* 编译执行代码

![image-20210410213522537](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210410213522537.png)

* 执行结果如下：

  ![image-20210410215503638](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210410215503638.png)

  ![image-20210410220045223](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210410220045223.png)

  ![image-20210410221704993](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210410221704993.png)

  同理并行发送和接收完10条消息，如下所示：消息队列的消息数有规律的增减着

  ![image-20210410221800080](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210410221800080.png)

  ![image-20210410221814443](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210410221814443.png)

  ![image-20210410221826332](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210410221826332.png)

  

  ![image-20210410222247198](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210410222247198.png)

  删除消息队列后，消息队列状态表为空：

  ![image-20210410222316837](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210410222316837.png)

##  实验内容2：

### 仿照 alg.8-4~8-6，编制基于 POSIX API 的进程间消息发送和消息接收例程。

#### 知识点：

1. 消息队列属性结构体

   ```c
   //消息队列属性结构体，
   struct mq_attr {
       long mq_flags;       //0 or O_NONBLOCK
       long mq_maxmsg;      //最大的消息数
       long mq_msgsize;     //每个消息最大的字节数
       long mq_curmsgs;     //队列中的当前消息数
   };
   ```

2. 消息队列的相关函数：

   a. mq_open: 创建或打开一个消息队列，函数原型如下：

   ```c
   mqd_t mq_open(const char *name, int oflag, /* mode_t mode, struct mq_attr *attr */); 
   //mqName:消息队列名称.格式如/xxx，如/mq_test.
   
   //oflags:表示打开的方式.可以是下面的各种组合
   /****************************
   *O_RDONLY.只读
   *O_WRONLY.只写
   *O_RDWR. 读写
   *O_CREAT. 如果不存在则创建
   *O_EXCL. 配合O_CREAT使用，只有不存在时才创建。存在的返回*失败
   *O_NONBLOCK. 不阻塞
   ****************************/
   
   //mode：是一个可选参数，在oflag中含有O_CREAT标志且消息队列不存在时，才需要提供该参数。表示默认访问权限。如0666
   
   //attr：也是一个可选参数，在oflag中含有O_CREAT标志且消息队列不存在时才需要。该参数用于给新队列设定某些属性，如果是空指针，那么就采用默认属性。
   
   //返回消息队列描述符
   ```

   b. mq_close:关闭消息队列,关闭后，消息队列并不从系统中删除,一个进程结束，会自动调用关闭打开着的消息队列。函数原型如下：

   ```c
   int mq_close(mqd_t mqdes)
   //mqdes:队列描述符
   //返回0表示关闭成狗，返回1表示关闭失败
   ```

   c. mq_unlink:删除消息队列, 函数原型如下：

   ```c
   int mq_unlink(const char *mqName)
   //返回0表示关闭成狗，返回1表示关闭失败
   ```

   d. mq_send:发送消息到消息队列, 函数原型如下：

   ```c
   int mq_send(mqd_t mqdes, const void *msg, size_t msglen, int prio)
   //mqdes:消息队列标识符
       
   //msg:消息内容
       
   //msglen:消息内容大小(单位为byte)。最大为mq_getattr()->mq_msgsize。
   
   //prio:优先级.小于MQ_PRIO_MAX。值越大优先级越高。
   
   //默认情况下mq_send和mq_receive是阻塞进行调用，可以通过mq_setattr来设置为O_NONBLOCK,当队列满时阻塞状态将会阻塞进程直到消息队列中有空的空间，非阻塞状态则直接返回ERROR，ERROR的种类如下
   /************************************
   errno如下:
   EAGAIN:队列非阻塞，并且已满
   EMSGSIZE:消息长度大于最大长度。
   EINTR:由信号中断返回。
   EPERM:没有权限
   *************************************/
   ```

   e. mq_receive:从消息队列中接收消息，函数原型如下：

   ```c
   ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio)
   //msg_ptr:接收到的消息放在的地方
   //msg_len:msg_ptr空间长度，消息大于该长度则接收出错，放回-1
   ```

   f. mq_getattr: 获取息队列关联的状态信息和属性,函数原型如下：

   ```c
   int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
   //mqstat:获取信息的buff.
   //获取成功返回0，否则返回-1
   ```

3. errno 是记录系统的最后一次错误代码。代码是一个int型的值，在errno.h中定义。查看错误代码errno是调试程序的一个重要方法。

#### 源代码以及详细的注释：

posix-api-mess-data.h 头文件：

```c
#define TEXT_SIZE 1024  /* = PAGE_SIZE, size of each message */
#define TEXT_NUM 20      /* maximal number of mesages */
struct shared_struct {
    int written; /* flag = 0: buffer writable; others: readable */
    char mtext[TEXT_SIZE]; /* buffer for message reading and writing */
};
//共享的结构体
#define PERM S_IRUSR|S_IWUSR|IPC_CREAT

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)
//退出代码
```

posix-api-mess.c 并发控制程序：

```c
/* gcc -lrt */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <error.h>
#include  <errno.h>
#include <mqueue.h>
#include <string.h>
#include "posix-api-mess-data.h"
int main(int argc, char *argv[]){
    struct shared_struct data_obj;
    struct mq_attr attr;
    int childpid1,childpid2;
    //消息队列属性设置
    attr.mq_maxmsg = TEXT_NUM;//初始化最大的消息数为TEXT_NUM
    attr.mq_msgsize = sizeof(data_obj);//初始化每个消息的最大字节数为shared_struct的字节数
    attr.mq_curmsgs=0;//初始化当前队列数为0
    // 创建一个消息队列
    mqd_t mq_id = mq_open("/mq_test", O_CREAT | O_RDWR, 0666, &attr);//以可读写的状态打开打开当前文件下mq_test文件，如果没有的话程序自己创建
    //0666给定读写发访问限权
    if(mq_id == -1){
        //消息队列创建失败，打印失败的原因
        //errno 是记录系统的最后一次错误代码，一个错误代码对应一个错误原因，可以在errno-base.h或者errno.h的头文件里面查看该表格
        printf("mq_open error %d\n",errno);
        exit(1);
    }
    //打印创建一个消息通道
    printf("-----------------------------------------\n");
    printf("Create a message channel:\n");
    //打印其相关参数
    printf("mq_msgsize:%ld\n",attr.mq_msgsize);//打印最大消息数
    printf("mq_maxmsg:%ld\n",attr.mq_maxmsg);//打印一个消息的最大字节数
    printf("mq_curmsgs:%ld\n",attr.mq_curmsgs);//打印当前队列数
    printf("-----------------------------------------\n");
    char *argv1[] = {" ", argv[1], 0};
    childpid1 = vfork();//创立子进程
    if(childpid1 < 0) {
        ERR_EXIT("shmpthreadcon: 1st vfork()");
    } 
    else if(childpid1 == 0) {
        execv("./posix-api-mess-send.o", argv1); 
        //子进程则调用发送消息者函数
    }
    else {
        childpid2 = vfork();
        if(childpid2 < 0) {
            ERR_EXIT("shmpthreadcon: 2nd vfork()");
        }
        else if (childpid2 == 0) {
            execv("./posix-api-mess-receive.o", argv1); 
            //子进程则调用接收消息函数
        }
        else {
            wait(&childpid1);
            wait(&childpid2);
            //等待子进程终结
            //删除消息队列
            if(mq_unlink("/mq_test") == -1){
                //当消息队列删除失败时，打印消息错误码
                printf("mq_unlink error %d \n",errno);
                exit(1);
            }
            //成功删除消息队列，打印删除成功
            printf("delete the message channel\n");
        }
    }
    //执行成功程序退出
    exit(EXIT_SUCCESS);
}	
```

posix-api-mess-send.c

```c
/* gcc -lrt */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <error.h>
#include  <errno.h>
#include <mqueue.h>
#include <string.h>
#include "posix-api-mess-data.h"
int main(int argc, char *argv[])
{
    struct shared_struct data_obj;
    //待发送的消息
    const char *message_0 = "Hello World!";
    //struct mq_attr attr;
    //创建一个消息队列
    mqd_t mq_id = mq_open("/mq_test", O_RDWR | O_CREAT, 0666,NULL);  
    //以可读写的状态打开打开当前文件下mq_test文件，如果没有的话程序自己创建，其中O_RDWR代表的就是读写模式，O_CREAT代表的就是不存在消息队列时候，程序自己创建。
    //0666是给全面的读写权限
    if(mq_id == -1) {
        //打开消息队列失败，输出消息队列打开失败的错误编码，从而方便我们分析原因
        printf("mq_open error %d\n",errno);
        exit(1);
    } 
    //将要发送的字符串存储在消息结构中，格式化消息便于发送数据
    sprintf((char*)data_obj.mtext,"%s",message_0);
    data_obj.written=1;//表示可读
    //发送大小为sizeof（data_obj)的消息data_obj到标识符为mq_id的消息队列中
    int ret=mq_send(mq_id,(char*)&data_obj,sizeof(data_obj),0);
    if(ret==0){//发送成功
        printf("send message:%s\n",message_0);
    }
    else{
        //发送错误，显示是什么类型的错误
        int errno_is=errno;
        printf("mq_sent error %d\n",errno_is);
    }
    // 获取消息队列相关属性
    printf("-----------------------------------------\n");
    printf("Aftering sending 'Hello World!' to the message channel:\n");
    struct mq_attr attr;//attr:获取信息的buff.
    if(mq_getattr(mq_id, &attr) == -1){
        //获取信息失败，打印失败的错误码，方便我们分析其错误原因
        printf("mq_getattr error %d\n",errno);
        exit(1);
    }
    printf("mq_msgsize:%ld\n",attr.mq_msgsize);//打印消息管道的最大消息数
    printf("mq_maxmsg:%ld\n",attr.mq_maxmsg);//打印消息的最大字节数
    printf("mq_curmsgs:%ld\n",attr.mq_curmsgs);//打印消息管道的当前的消息数
    printf("-----------------------------------------\n");
    //关闭消息队列
    if(mq_close(mq_id)==-1){
        //消息队列关闭失败，打印关闭失败的错误码，从而方便我们分析原因
        printf("mq_close error %d\n",errno);
        exit(1);
    }
    //程序执行成功，成功退出
    return EXIT_SUCCESS;
}
```

posix-api-mess-receive.c

```c
/* gcc -lrt */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <error.h>
#include  <errno.h>
#include <mqueue.h>
#include <string.h>
#include "posix-api-mess-data.h"
int main(int argc, char *argv[])
{
    struct shared_struct data_obj;
    //打开消息队列
    mqd_t mq_id = mq_open("/mq_test", O_RDONLY, 0, NULL);
    //O_RDONLY：只读限权，程序以只读的方式打开当前文件夹下的mq_test文件，因为在并发调用控制的程序中已经创立了该消息管道，因此此处不需要再创立消息管道，因为打开的是同一个共享文件，因此该程序与发送程序共用一个消息管道，mq_id的值相同。
    if(mq_id == -1){
        //打开消息队列失败，输出消息队列打开失败的错误编码，从而方便我们分析原因
        printf("mq_open error %d\n",errno);
        exit(1);
    }
    //接收消息队列中的消息
    ssize_t ret = mq_receive(mq_id, (char *)&data_obj, sizeof(data_obj), NULL);
    if(ret>=0&&data_obj.written==1){
        //接收成功，打印消息
        printf("receive message: %s\n", data_obj.mtext);
    }
    else {
        //接收消息失败，答应失败发生的错误码，从而方便我们分析出现的错误类型
         int errno_is = errno;
         printf("mq_receive error %d\n",errno_is);
    }
    printf("-----------------------------------------\n");
    // 在向消息管道中获取“Hello world”消息后，获取消息队列相关属性
    printf("After receiving 'Hello World!' from the message channel:\n");
    struct mq_attr attr;
    if(mq_getattr(mq_id, &attr) == -1){
         //获取信息失败，打印失败的错误码，方便我们分析其错误原因
        printf("mq_getattr error %d\n",errno);
        exit(1);
    }
    printf("mq_msgsize:%ld\n",attr.mq_msgsize);//打印消息管道的消息数
    printf("mq_maxmsg:%ld\n",attr.mq_maxmsg);//打印消息的最大字节数
    printf("mq_curmsgs:%ld\n",attr.mq_curmsgs);//打印消息管道的当前消息数
    printf("-----------------------------------------\n");
    //关闭消息队列
    if(mq_close(mq_id)==-1){
        //关闭队列失败，答应关闭队列的错误类型
        printf("mq_close error %d \n",errno);
        exit(1);
    }
    return EXIT_SUCCESS;
}

```



#### 运行结果图以及分析：

使用如下的命令行编译程序：

![image-20210411141713201](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210411141713201.png)

程序执行结果图：

![image-20210411141855351](C:%5CUsers%5Cyou%5CDesktop%5CLab%20Week%2008.assets%5Cimage-20210411141855351.png)

分析：可以观察到，

1. 创建消息管道后，消息管道的最大消息数使我们设置的1028大小，消息的最大字节数也是我们设置的20大小，当前的消息管道中的消息数为0；
2. 发送程序发送Hello World！的消息到消息管道中；
3. 在程序发送消息后我们打印消息管道的状态表，可以观察到，消息管道的固有性质（最大消息数以及消息的最大字节数）都没有发生变化，当前的消息管道中的消息数+1，变成了1.
4. 从消息管道中接收程序接收Hello World！的消息；
5. 在程序接收消息后我们打印消息管道的状态表，可以观察到，消息管道的固有性质（最大消息数以及消息的最大字节数）都没有发生变化，当前消息管道的消息数-1，变成了0；
6. 进程控制的程序wait发送程序和接收程序都执行完成后，程序删除掉消息管道，并且删除成功。