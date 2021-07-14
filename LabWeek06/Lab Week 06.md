# Lab Week 06.

## 实验内容：

进程间通信—共享内存。
(1) 验证：编译运行课件 Lecture 08 例程代码：

* Linux 系统调用示例 reader-writer 问题：Algorithms 8-1 ~ 8-3.
*  POSIX API 应用示例 producer-consumer 问题：Algorithms 8-4 ~ 8-5.

(2) 设计：Alg.8-1 ~ 8-3 示例只解决单字符串的读写。修改程序将共享空间组织成一个结构类型（比如学号、姓名）的循环队列进行 FIFO 操作，采用共享内存变量控制队列数据的同步（参考数据结构课程有关内容）。

## 实验报告

实验内容的原理性和实现细节解释，包括每个系统调用的作用过程和结果。

## 实验内容：

### 一. Linux 系统调用示例 reader-writer 问题：Algorithms 8-1 ~ 8-3

* 知识点：

  1. open() 用于打开已经存在的文件或者创建一个新文件

     原型

     ```c
     int open(const char *pathname,int flag);
     int open(const char *pathname,int flag,mode_t mode);
     /********************************************
     注：
     1. pathname 不能超过1024个字节，否则会被截断。
     2. flag 常用分类: 顾名思义。
         O_RDONLY 0 只读
         O_WRONLY 1 只写
         O_RDWR 2 读写
         O_APPEND 追加
         O_CREAT ： 文件不存在则创建，与mode一起使用，用来指定文件权限。
         O_EXCL ： 查看文件是否存在。同O_CREATE 一起使用时，如果文件已存在返回错误。
         O_TRUNC ：将文件长度截断为0。通常对需要清空的文件进行归零操作
         O_NONBLOCK :非阻塞打开文件。
     3. mode 分类
         S_IRWXU 00700 用户有读写执行权限
         S_IRUSE 00400
         S_IWUSE 00200
         S_IXUSE 001001
     *******************************************/
     ```

  2. create() 用于创建一个新文件。用法与open类似

     原型：

     ```c
     int creat(const char *pathname ,mode_t mode);
     //成功返回 0 ，失败返回-1；
     ```

  3. 共享内存就是允许两个不相关的进程访问同一个逻辑内存，我们提供了相关的函数使得不同的进程同步地访问共享内存，它们声明在头文件 sys/shm.h 中。

     **1、shmget()函数**

     该函数用来创建共享内存，它的原型为：

     ```c
     int shmget(key_t key, size_t size, int shmflg);
     /****************************************
     1. key（非0整数）: 它有效地为共享内存段命名，shmget()函数成功时返回一个与key相关的共享内存标识符（非负整数），用于后续的共享内存函数。调用失败返回-1.
     2. size: 以字节为单位指定需要共享的内存容量
     3. shmflg: 权限标志，它的作用与open函数的mode参数一样，共享内存的权限标志与文件的读写权限一样.
     ******************************************/
     ```

     **2、shmat()函数   -- at：attach**

     shmat()函数的作用就是用来启动对该共享内存的访问，并把共享内存连接到当前进程的地址空间。它的原型如下：

     ```C
     void *shmat(int shm_id, const void *shm_addr, int shmflg);
     /*******************************************
     第一个参数，shm_id是由shmget()函数返回的共享内存标识。
     第二个参数，shm_addr指定共享内存连接到当前进程中的地址位置，通常为空，表示让系统来选择共享内存的地址。
     第三个参数，shm_flg是一组标志位，通常为0。
     调用成功时返回一个指向共享内存第一个字节的指针，如果调用失败返回-1.
     *******************************************/
     ```

     **3、shmdt()函数   -- dt：detach**

     该函数用于将共享内存从当前进程中分离。注意，将共享内存分离并不是删除它，只是使该共享内存对当前进程不再可用。它的原型如下：

     ```c
     int shmdt(const void *shmaddr);
     /**************************************
     参数shmaddr是shmat()函数返回的地址指针，调用成功时返回0，失败时返回-1
     *****************************************/
     ```

     **4、shmctl()函数   -- ctl：control**

     与信号量的semctl()函数一样，用来控制共享内存，它的原型如下：

     ```c
     int shmctl(int shm_id, int command, struct shmid_ds *buf);
     /******************************************
        第一个参数，shm_id是shmget()函数返回的共享内存标识符。
        第二个参数，command是要采取的操作，它可以取下面的三个值 ：
     - IPC_STAT：把shmid_ds结构中的数据设置为共享内存的当前关联值，即用共享内存的当前关联值覆盖shmid_ds的值。
     - IPC_SET：如果进程有足够的权限，就把共享内存的当前关联值设置为shmid_ds结构中给出的值
     - IPC_RMID：删除共享内存段
     第三个参数，buf是一个结构指针，它指向共享内存模式和访问权限的结构。
     *******************************************/
     ```

     

* 源代码以及**详细注释：**程序思路包含在注释中

  8-0：

  ```c
  #define TEXT_SIZE 4*1024  /* = PAGE_SIZE, size of each message */
  #define TEXT_NUM 1      /* maximal number of mesages */
  //TEXT_NUM改成n
      /* total size can not exceed current shmmax,
         or an 'invalid argument' error occurs when shmget */
  
  /* a demo structure, modified as needed */
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

  8-1

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <sys/shm.h>
  #include <fcntl.h>
  
  #include "alg.8-0-shmdata.h"
  
  int main(int argc, char *argv[])
  {
      struct stat fileattr;
      key_t key; /* of type int */
      int shmid; /* shared memory ID *////共享内存标识符,创建共享内存 
      void *shmptr;
      struct shared_struct *shared; /* structured shm *///用户定义指向shm
      
      pid_t childpid1, childpid2;
      char pathname[80], key_str[10], cmd_str[80];
      int shmsize, ret;
  
      shmsize = TEXT_NUM*sizeof(struct shared_struct);//共享区域大小
      printf("max record number = %d, shm size = %d\n", TEXT_NUM, shmsize);
  
      if(argc <2) {
          printf("Usage: ./a.out pathname\n");
          return EXIT_FAILURE;
      }
      strcpy(pathname, argv[1]);
      //如果没有共享文件就创建
      if(stat(pathname, &fileattr) == -1) {
          //stat()用来将参数file_name 所指的文件状态, 复制到参数buf 所指的结构中。
          ret = creat(pathname, O_RDWR);
          if (ret == -1) {
              ERR_EXIT("creat()");
          }
          printf("shared file object created\n");
      }
   
      key = ftok(pathname, 0x27); /* 0x27 a project ID 0x0001 - 0xffff, 8 least bits used */
      //把一个已存在的路径名和一个整数标识符转换成IPC键值
      //出错：-1，错误原因存于error中
      if(key == -1) {
          ERR_EXIT("shmcon: ftok()");
      }
      printf("key generated: IPC key = %x\n", key); /* can set any nonzero key without ftok()*/
      //寻找与生产者的共享内存,成功返回共享内存的ID，出错返回-1 
      shmid = shmget((key_t)key, shmsize, 0666|PERM);//0666是给全面的读写权限
      //shmid:共享内存标识符
      if(shmid == -1) {
          ERR_EXIT("shmcon: shmget()");//创建失败
      }
      printf("shmcon: shmid = %d\n", shmid);
  	////将共享内存连接到当前进程的地址空间（挂接操作）
      //成功返回共享存储段的指针，出错返回-1
      shmptr = shmat(shmid, 0, 0); /* returns the virtual base address mapping to the shared memory, *shmaddr=0 decided by kernel */
  //shmat()函数的作用就是用来启动对该共享内存的访问，并把共享内存连接到当前进程的地址空间
      if(shmptr == (void *)-1) {
          ERR_EXIT("shmcon: shmat()");
      }
      printf("shmcon: shared Memory attached at %p\n", shmptr);
      //进行强制类型转换，防止编译器发出warming
      //将shmptr强制转换为struct shared_struct *类型的指针并赋给shared之后的目的是为了将shared看作为一个共享对象在共享内存区进行操作了。
      shared = (struct shared_struct *)shmptr;
      shared->written = 0;
  	//设置written使共享内存段可写
      sprintf(cmd_str, "ipcs -m | grep '%d'\n", shmid);
      //ipcs -m，查看共享内存
      //grep 指令用于查找内容包含指定的范本样式的文件，在这里显示所有包含共享内存标识符的行。
      printf("\n------ Shared Memory Segments ------\n");
      system(cmd_str);//调用系统命令查看IPC对象：显示所有包含共享内存标识符的行。
     //此处分别显示共享内存地址，共享内存标识符，进程拥有者，权限，字节连接数和状态
      if(shmdt(shmptr) == -1) {
          //把共享内存从当前进程中分离，使该共享内存对当前进程不再可用
          //调用失败返回-1
          ERR_EXIT("shmcon: shmdt()");
      }
  
      printf("\n------ Shared Memory Segments ------\n");
      system(cmd_str);//调用系统命令查看IPC对象：显示所有包含共享内存标识符的行。
  
      sprintf(key_str, "%x", key);
      char *argv1[] = {" ", key_str, 0};
      //生成子进程
      childpid1 = vfork();
      if(childpid1 < 0) {//错误
          ERR_EXIT("shmcon: 1st vfork()");
      } 
      else if(childpid1 == 0) {//进入子进程
          //执行外部的可进行代码
          execv("./alg.8-2-shmread.o", argv1); /* call shm_read with IPC key */
          //读进程
      }
      else {
          //创建子进程
          childpid2 = vfork();
          if(childpid2 < 0) {
              ERR_EXIT("shmcon: 2nd vfork()");
          }
          else if (childpid2 == 0) {//进入子进程
          //执行指定路径的alg.8-3-shmwrite.o文件（在执行前已经编译存在
              execv("./alg.8-3-shmwrite.o", argv1); /* call shmwrite with IPC key */
              //写进程
          }
          else {
              wait(&childpid1);
              wait(&childpid2);
                   /* shmid can be removed by any process knewn the IPC key */
              //删除共享内存段
              if (shmctl(shmid, IPC_RMID, 0) == -1) {
                  ERR_EXIT("shmcon: shmctl(IPC_RMID)");
              }
              else {
                  printf("shmcon: shmid = %d removed \n", shmid);
                  printf("\n------ Shared Memory Segments ------\n");
                  system(cmd_str);//同上
                  printf("nothing found ...\n"); 
              }
          }
    }
      exit(EXIT_SUCCESS);
}
  
  
  ```
  
  8-2-shmread
  
  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <string.h>
  #include <sys/shm.h>
  
  #include "alg.8-0-shmdata.h"
  int main(int argc, char *argv[])
  {
      void *shmptr = NULL;//分配的共享内存的原始首地址  
      struct shared_struct *shared;//用户指向shm
      int shmid;/* shared memory ID *///进程私有
      //共享内存标识符,创建共享内存  
      key_t key;
   
      sscanf(argv[1], "%x", &key);
      printf("%*sshmread: IPC key = %x\n", 30, " ", key);
      //寻找与生产者的共享内存,成功返回共享内存的ID，出错返回-1 
      shmid = shmget((key_t)key, TEXT_NUM*sizeof(struct shared_struct), 0666|PERM);
      //0666是给全面的读写权限
      //shmid:共享内存标识符
      if (shmid == -1) {
          ERR_EXIT("shread: shmget()");
      }
  //将共享内存连接到当前进程的地址空间（挂接操作）
      //成功返回共享存储段的指针，出错返回-1
      shmptr = shmat(shmid, 0, 0); /* returns the virtual base address mapping to the shared memory, *shmaddr=0 decided by kernel */
  //shmat()函数的作用就是用来启动对该共享内存的访问，并把共享内存连接到当前进程的地址空间
      if(shmptr == (void *)-1) {
          ERR_EXIT("shread: shmat()");
      }
      printf("%*sshmread: shmid = %d\n", 30, " ", shmid);    
      printf("%*sshmread: shared memory attached at %p\n", 30, " ", shmptr);
      printf("%*sshmread process ready ...\n", 30, " ");
      
      shared = (struct shared_struct *)shmptr;
      //持续的读，等待读
      while (1) {
          while (shared->written == 0) {
              sleep(1); /* message not ready, waiting ... */
          }
          //当标识符表示可写时，输出写入的内容
          printf("%*sYou wrote: %s\n", 30, " ", shared->mtext);
          shared->written = 0;//设置写入位为不可写
          //当写入的内容为end时。停止读取
          if (strncmp(shared->mtext, "end", 3) == 0) {
              break;
          }
      } /* it is not reliable to use shared->written for process synchronization */
       //把共享内存从当前进程中分离
     if (shmdt(shmptr) == -1) {
        ERR_EXIT("shmread: shmdt()");
     }
 //control shmctl()进行共享内存段的删除
      sleep(1);
      exit(EXIT_SUCCESS);
  }
  
  ```
  
  ​		8-3-shmwrite
  
  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <string.h>
  #include <sys/shm.h>
  
  #include "alg.8-0-shmdata.h"
   
  int main(int argc, char *argv[])
  {
      void *shmptr = NULL;//分配的共享内存的原始首地址
      struct shared_struct *shared = NULL;
      int shmid;
      key_t key;
  
      char buffer[BUFSIZ + 1]; /* 8192bytes, saved from stdin *///用于保存输入的文本
      
      sscanf(argv[1], "%x", &key);
  
      printf("shmwrite: IPC key = %x\n", key);
  //创建共享内存,成功返回共享内存的ID，出错返回-1
      shmid = shmget((key_t)key, TEXT_NUM*sizeof(struct shared_struct), 0666|PERM);
      if (shmid == -1) {
          ERR_EXIT("shmwite: shmget()");
      }
      //将共享内存连接到当前进程的地址空间（挂接操作）
      //成功返回共享存储段的指针，出错返回-1
      shmptr = shmat(shmid, 0, 0);
      if(shmptr == (void *)-1) {
          ERR_EXIT("shmwrite: shmat()");
      }
      printf("shmwrite: shmid = %d\n", shmid);
      printf("shmwrite: shared memory attached at %p\n", shmptr);
      printf("shmwrite precess ready ...\n");
      
      shared = (struct shared_struct *)shmptr;
      //持续的写，等待写
      while (1) {
          while (shared->written == 1) {
              sleep(1); /* message not read yet, waiting ... */ 
          }
   
          printf("Enter some text: ");
          fgets(buffer, BUFSIZ, stdin);//从目标文件流stdin中读取 BUFSIZ-1 个字符，放入以 buffer 起始地址的内存空间中。
          strncpy(shared->mtext, buffer, TEXT_SIZE);//将buffer区域的数据拷贝到mtext中
          printf("shared buffer: %s\n",shared->mtext);//打印写入的数据
          shared->written = 1;  /* message prepared */
          //修改写标识符为1，表示内存中有数值且不可写，这是为了实现进程间的同步互斥
   
          if(strncmp(buffer, "end", 3) == 0) {
              break;
          }
          //当写入的数据为end时，跳出循环，程序结束
    }
         /* detach the shared memory */
      if(shmdt(shmptr) == -1) {
          ERR_EXIT("shmwrite: shmdt()");
      }
  
  //    sleep(1);
      exit(EXIT_SUCCESS);
  }
  
  ```
  
  运行结果图：
  
  ![image-20210331082007483](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331082007483.png)
  
  ![image-20210331082116341](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331082116341.png)
  
  具体分析：
  
  ![image-20210331082652487](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331082652487.png)
  
  
  
  IPC键值为2701c9558,在父进程和子进程中的IPC键值相同，两者共用同一个共享内存。
  
  ![image-20210331082905436](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331082905436.png)
  
  ![image-20210331082955207](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331082955207.png)
  
  三个进程映射到共享内存的地址都不同，说明不同进程附加到同一块共享内存的共享内存地址不一定相同。
  
  ![image-20210331090710379](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331090710379.png)
  
  可以观察到在调用shmdt()前后共享内存的状态量发生了改变，调用前程序通过调用shmat函数将共享内存连接到当前进程的地址空间，因此进程可以使用共享内存，而在调用shmdt()函数后，该共享内存从当前进程中分离，但是没有删除，因此共享内存存在，但是当前进程无法使用该共享内存。
  
  ![image-20210331091514954](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331091514954.png)
  
  在调用shmctl()函数后，共享内存段被删除，因此找不到该数据段，无法显示相关的信息

### 二. POSIX API 应用示例 producer-consumer 问题：Algorithms 8-4 ~ 8-5.

* 知识点：

  1. **shm_open**

     shm_open用于创建一个新的Posix共享内存对象或打开一个已存在的Posix共享内存对象。

     ```c
     //成功返回非负描述符，失败返回-1
     int shm_open(const char *name, int oflag, mode_t mode);
     /*
     - oflag参数不能设置O_WRONLY标志
     - 和mq_open、sem_open不同，shm_open的mode参数总是必须指定，当指定了O_CREAT标志时，mode为用户权限位，否则将mode设为0
     -  shm_open的返回值是一个描述符，它随后用作mmap的第五个参数fd。
      */
     ```

  2. **shm_unlink函数**
     shm_unlink用于从系统中删除一个Posix共享内存对象。

     ```c
     //成功返回0，失败返回-1
     int shm_unlink(const char *name);
     ```

  3. **ftruncate函数**

     处理mmap的时候，普通文件或Posix共享内存对象的大小都可以通过调用ftruncate设置。

     ```c
     //成功返回0，失败返回-1
     int ftruncate(int fd, off_t length):
     /*************************************************************
     - 对于普通文件，若文件长度大于length，额外的数据会被丢弃；若文件长度小于length，则扩展文件大小到length
     - 对于Posix共享内存对象，ftruncate把该对象的大小设置成length字节
       我们调用ftruncate来指定新创建的Posix共享内存对象大小，或者修改已存在的Posix共享内存对象大小。
     ***********************************************************/
     ```

  4.  mmap函数

     mmap函数把一个文件或一个Posix共享内存对象映射到调用进程的地址空间，使用该函数有三个目的：

  - 使用普通文件以提供内存映射IO
  - 使用特殊文件以提供匿名内存映射
  - 使用Posix共享内存对象以提供Posix共享内存区

  ```c
  //成功返回映射内存的起始地址，失败返回MAP_FAILED
  void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
  ```

  ​		**mmap参数解析：**

  - addr指定映射内存的起始地址，通常设为NULL，让内核自己决定起始地址

  - len是被映射到调用进程地址空间中的字节数，它从被映射文件fd开头起第offset个字节处开始算，offset通常设为0，下图展示了这个映射关系

  - prot指定对映射内存区的保护，通常设为PROT_READ | PROT_WRITE

  - flags必须在MAP_SHARED和MAP_PRIVATE这两个标志中选择指定一个，进程间共享内存需要使用MAP_SHARED

  - 可移植的代码应把addr设为NULL，并且flags不指定MAP_FIXED

  - | prot       | 说明         | flags       | 说明               |
    | ---------- | ------------ | ----------- | ------------------ |
    | PROT_READ  | 数据可读     | MAP_SHARED  | 变动是共享的       |
    | PROT_WRITE | 数据可写     | MAP_PRIVATE | 变动是私有的       |
    | PROT_EXEC  | 数据可执行   | MAP_FIXED   | 准确地解释addr参数 |
    | PROT_NONE  | 数据不可访问 |             |                    |

    mmap成功返回后，可以关闭fd，这对已建立的映射关系没有影响。
    注意，不是所有文件都能进行内存映射，例如终端和套接字就不可以。

* 源代码以及非常详细的注释

  Algorithm 8-4: shmpthreadcon.c  

  ```c
  /* gcc -lrt */
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <fcntl.h>
  #include <sys/mman.h>
  
  #include "alg.8-0-shmdata.h"
  
  int main(int argc, char *argv[])
  {
      char pathname[80], cmd_str[80];
      struct stat fileattr;
      int fd, shmsize, ret;
      pid_t childpid1, childpid2;
  
      if(argc < 2) {
          printf("Usage: ./a.out filename\n");
          //打印```Usage: ./a.out pathname```提示我们补全路径名，以便之后根据文件信息使用ftok()函数建立共享内存ID值
          return EXIT_FAILURE;
      }
  	//创建一个新的Posix共享内存对象
      fd = shm_open(argv[1], O_CREAT|O_RDWR, 0666);
      //O_CREAT如果指定文件(argv[1]文件）不存在，则创建这个文件
      //O_RDWR读写模式
      //0666是给全面的读写权限
      //成功放回非负描述符
          /* /dev/shm/filename as the shared object, creating if not exist */
      if(fd == -1) {//创建失败
          ERR_EXIT("con: shm_open()");
      } 
      system("ls -l /dev/shm/");   
   	//其中，/dev/shm就是共享内存，它使用内存虚拟出一个文件路径，可以视为文件进行访问。
      /* set shared size to 1.8GB; near the upper bound of 90% phisical memory size of 2G
         shmsize = 1.8*1024*1024*1024; */
  
      shmsize = TEXT_NUM*sizeof(struct shared_struct);
      ret = ftruncate(fd, shmsize);
      //调用ftruncate来指定新创建的Posix共享内存对象fd大小，或者修改已存在的Posix共享内存对象大小。
      if(ret == -1) {//调用失败返回-1
          ERR_EXIT("con: ftruncate()");
      }
      
      char *argv1[] = {" ", argv[1], 0};
      childpid1 = vfork();//创立子进程
      if(childpid1 < 0) {
          ERR_EXIT("shmpthreadcon: 1st vfork()");
      } 
      else if(childpid1 == 0) {
          execv("./alg.8-5-shmproducer.o", argv1); /* call shmproducer with filename */
          //子进程则调用生产者函数
      }
      else {
          childpid2 = vfork();
          if(childpid2 < 0) {
              ERR_EXIT("shmpthreadcon: 2nd vfork()");
          }
          else if (childpid2 == 0) {
              execv("./alg.8-6-shmconsumer.o", argv1); 
              //子进程则调用消费者函数
              /* call shmconsumer with filename */
          }
          else {
              wait(&childpid1);
              wait(&childpid2);
              //等待子进程终结
              ret = shm_unlink(argv[1]); /* shared object can be removed by any process knew the filename */
              //用于从系统中删除一个Posix共享内存对象
              if(ret == -1) {
                  ERR_EXIT("con: shm_unlink()");
              }
              system("ls -l /dev/shm/");   
              //列出共享内存内容
          }
      }
      exit(EXIT_SUCCESS);
  }
  	
  
  ```

  8-5-shmproducer.c生产者进程

  ```c
  /* gcc -lrt */
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/mman.h>
  
  #include "alg.8-0-shmdata.h"
  
  int main(int argc, char *argv[])
  {
      int fd, shmsize, ret;
      void *shmptr;
      const char *message_0 = "Hello World!";
      //创建一个新的Posix共享内存对象
      fd = shm_open(argv[1], O_RDWR, 0666); /* 
      //O_RDWR读写模式
      //0666是给全面的读写权限/dev/shm/filename as the shared object */
      if(fd == -1) {
          ERR_EXIT("producer: shm_open()");
      } 
      
      /* set shared size to 1.8GB; near the upper bound of 90% phisical memory size of 2G
         shmsize = 1.8*1024*1024*1024; */
  
      shmsize = TEXT_NUM*sizeof(struct shared_struct);
      shmptr = (char *)mmap(0, shmsize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
      //把一个文件或一个Posix共享内存对象映射到调用进程的地址空间,返回映射内存的起始地址
      //PROT_READ	数据可读
      //PROT_WRITE 数据可写
      //MAP_SHARED:  建立共享，用于进程间通信，如果没有这个标志，则别的进程即使能打开文件，也看不到数据。
      if(shmptr == (void *)-1) {
          //映射失败
          ERR_EXIT("producer: mmap()");
      }
  	//把message_0格式化输入到shmptr中
      sprintf(shmptr,"%s",message_0);
      //打印格式化信息
      printf("produced message: %s\n", (char *)shmptr);
  
      return EXIT_SUCCESS;
  }
  
  ```

  8-6-shmconsumer.c 消费者进程

  ```c
  /* gcc -lrt */
  #include <stdio.h>
  #include <stdlib.h>
  #include <fcntl.h>
  #include <sys/mman.h>
  
  #include "alg.8-0-shmdata.h"
  
  int main(int argc, char *argv[])
  {
      int fd, shmsize;
      void *shmptr;
      
      fd = shm_open(argv[1], O_RDONLY, 0444);
      //0444是只给进程读的权限
      if(fd == -1) {
          ERR_EXIT("consumer: shm_open()");
      } 
  
        /* set shared size to 1.8GB; near the upper bound of 90% phisical memory size of 2G
           shmsize = 1.8*1024*1024*1024; */
  
      shmsize = TEXT_NUM*sizeof(struct shared_struct);
      shmptr = (char *)mmap(0, shmsize, PROT_READ, MAP_SHARED, fd, 0);
      //PROT_WRITE 数据可写
      //MAP_SHARED:  建立共享，用于进程间通信，如果没有这个标志，则别的进程即使能打开文件，也看不到数据。
      if(shmptr == (void *)-1) {
          ERR_EXIT("consumer: mmap()");
      }
      
      printf("consumed message: %s\n", (char *)shmptr);
      return EXIT_SUCCESS;
  }
  
  
  ```

  

* 运行结果：

  ![image-20210401014647344](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210401014647344.png)

分析：程序系统调用命令"ls -l /dev/shm/"，使用"ls -l"命令查看"/dev/shm/"目录下的文件信息，可以看到，第一行的"total"为0，代表该目录下所有文件所占空间的总和为0，

接下来是该目录下唯一一个文件即共享对象myshm一个7个字段的列表"-rw-rw-r-- 1 moocos moocos 0 4月  1 01:46 myshm"，其中"-rw-rw-r--"表示该文件是一个普通文件，用户和用户组的其他成员对该文件有读写的权限，其他用户只有读的权限，"1"代表这个文件只有一个文件名，只有一个指向该链接的硬链接，"moocos moocos"表示文件拥有者和文件拥有者所在组，```0```代表文件所占用的空间，"4月  1 01:46"表示文件最近访问或修改时间，"myshm"表示文件名。

### 三. 修改程序将共享空间组织成一个结构类型（比如学号、姓名）的循环队列进行 FIFO 操作，采用共享内存变量控制队列数据的同步（参考数据结构课程有关内容）

* 思路：设计共享空间为循环队列结构类型，开辟循环队列的空间大小为20，这样就可以实现多个字符串（比如学号、姓名）的传输了，设置readlock，writelock来确保单个读写程序在执行，这样就能有效的控制队列数据的同步。

* 源代码以及详细的注释：

  头文件:shmdata.h

  ```c
  #define TEXT_SIZE 4*1024  /* = PAGE_SIZE, size of each message */
  #define TEXT_NUM 20      /* maximal number of mesages */
  //TEXT_NUM改成n
      /* total size can not exceed current shmmax,
         or an 'invalid argument' error occurs when shmget */
  
  /* a demo structure, modified as needed */
  // struct shared_struct {
  //     int written; /* flag = 0: buffer writable; others: readable */
  //     char mtext[TEXT_SIZE]; /* buffer for message reading and writing */
  // };
  // //共享的结构体
  
  typedef struct shared_struct {//循环队列
      char buf[TEXT_SIZE * TEXT_NUM];//字符串数组
      int front;//队头指针，读指针
      int rear;//队尾指针，写指针
      int readlock;//值为1时读的临界区内存可访问，为0时不可访问
      int writelock;//值为1时写的临界区内存可访问，为0时不可访问
      int fullnum;//队列中写入字符串的内存单元个数
      int emptynum;//队列中为空的内存单元个数
      int firstbuild;//第一次创建队列的标志位，创建完成为1
  } Queue;
  int  enqueue(Queue *q, const char *data, size_t len);
  
  int dequeue(Queue *q, char *data);
  
  void init_queue(Queue *q) {
      q->front = 0;
      q->rear = 0;
      q->readlock=0;
      q->writelock=1;
      q->fullnum=0;
      q->emptynum=1;
      q->firstbuild=1;
      memset(q->buf, 0, TEXT_SIZE * TEXT_NUM);
  }
  
  int  enqueue(Queue *q, const char *data, size_t len) {
      if (len > TEXT_SIZE || len <= 0) {
          puts("ERROR: the length of data is not supported!");
          return 1;
      } else {
              strncpy(q->buf + q->rear * TEXT_SIZE, data, len);
              q->rear = (q->rear + 1) % TEXT_NUM;
              return 0;
      	   }
  }
  
  int dequeue(Queue *q, char *data) {
          strncpy(data, q->buf + q->front * TEXT_SIZE, TEXT_SIZE);
          q->front = (q->front + 1) % TEXT_NUM;  
          return 1;
  }
  #define PERM S_IRUSR|S_IWUSR|IPC_CREAT
  
  #define ERR_EXIT(m) \
      do { \
          perror(m); \
          exit(EXIT_FAILURE); \
      } while(0)
  //退出代码
  
  
  ```

  主文件：shmcon.c

  ​	

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <sys/shm.h>
  #include <fcntl.h>
  
  #include "alg.8-0-shmdata.h"
  
  int main(int argc, char *argv[])
  {
      struct stat fileattr;
      key_t key; /* of type int */
      int shmid; /* shared memory ID *///进程私有
      void *shmptr;
      Queue *q; /* structured shm *///用户定义
      pid_t childpid1, childpid2;
      char pathname[80], key_str[10], cmd_str[80];
      int shmsize, ret;
  
      shmsize = TEXT_NUM*sizeof(struct shared_struct);//共享区域大小
      printf("max record number = %d, shm size = %d\n", TEXT_NUM, shmsize);
  
      if(argc <2) {
          printf("Usage: ./a.out pathname\n");
          return EXIT_FAILURE;
      }
      strcpy(pathname, argv[1]);
      //如果没有共享文件就创建
      if(stat(pathname, &fileattr) == -1) {
          //stat()用来将参数file_name 所指的文件状态, 复制到参数buf 所指的结构中。
          ret = creat(pathname, O_RDWR);
          if (ret == -1) {
              ERR_EXIT("creat()");
          }
          printf("shared file object created\n");
      }
   
      key = ftok(pathname, 0x27); /* 0x27 a project ID 0x0001 - 0xffff, 8 least bits used */
      if(key == -1) {
          ERR_EXIT("shmcon: ftok()");
      }
      printf("key generated: IPC key = %x\n", key); /* can set any nonzero key without ftok()*/
      //创建共享内存
      shmid = shmget((key_t)key, shmsize, 0666|PERM);//0666是给全面的读写权限
      //shmid:共享内存标识符
      if(shmid == -1) {
          ERR_EXIT("shmcon: shmget()");
      }
      printf("shmcon: shmid = %d\n", shmid);
  
      shmptr = shmat(shmid, 0, 0); /* returns the virtual base address mapping to the shared memory, *shmaddr=0 decided by kernel */
  
      if(shmptr == (void *)-1) {
          ERR_EXIT("shmcon: shmat()");
      }
      printf("shmcon: shared Memory attached at %p\n", shmptr);
      //进行强制类型转换，防止编译器发出warming
     q  = (Queue*)shmptr;
     q->writelock = 1;
  
      sprintf(cmd_str, "ipcs -m | grep '%d'\n", shmid); 
      printf("\n------ Shared Memory Segments ------\n");
      system(cmd_str);
  	
      if(shmdt(shmptr) == -1) {
          ERR_EXIT("shmcon: shmdt()");
      }
  
      printf("\n------ Shared Memory Segments ------\n");
      system(cmd_str);
  
      sprintf(key_str, "%x", key);
      char *argv1[] = {" ", key_str, 0};
      //生成子进程
      childpid1 = vfork();
      if(childpid1 < 0) {//错误
          ERR_EXIT("shmcon: 1st vfork()");
      } 
      else if(childpid1 == 0) {//进入子进程
          //执行外部的可进行代码
          execv("./alg.8-2-shmread.o", argv1); /* call shm_read with IPC key */
      }
      else {
          //创建子进程
          childpid2 = vfork();
          if(childpid2 < 0) {
              ERR_EXIT("shmcon: 2nd vfork()");
          }
          else if (childpid2 == 0) {//进入子进程
          //执行外部的可进行代码
              execv("./alg.8-3-shmwrite.o", argv1); /* call shmwrite with IPC key */
          }
          else {
              wait(&childpid1);
              wait(&childpid2);
                   /* shmid can be removed by any process knewn the IPC key */
              //删除共享内存段
              if (shmctl(shmid, IPC_RMID, 0) == -1) {
                  ERR_EXIT("shmcon: shmctl(IPC_RMID)");
              }
              else {
                  printf("shmcon: shmid = %d removed \n", shmid);
                  printf("\n------ Shared Memory Segments ------\n");
                  system(cmd_str);
                  printf("nothing found ...\n"); 
              }
          }
      }
      exit(EXIT_SUCCESS);
  }
  
  
  ```

  ​	读程序shmread.c：

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <string.h>
  #include <sys/shm.h>
  
  #include "alg.8-0-shmdata.h"
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
  ```

  ​	写程序shmwrite.c

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <string.h>
  #include <sys/shm.h>
  
  #include "alg.8-0-shmdata.h"
   
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
  
  ```

  

* 运行结果以及分析：

  ![image-20210331210757241](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331210757241.png)

  编译运行代码:

  ![image-20210331210838364](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331210838364.png)

  ​				
  ![image-20210331210854980](http://hurq5.gitee.io/os-labwork/LabWeek06/pictures/image-20210331210854980.png)

  可以观察到程序可以传输多条字符串。