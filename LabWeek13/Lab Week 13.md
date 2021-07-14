#  Lab Week 13.

## 一. 实验要求：

设计实现一个线程池 (`Thread Pool`)

* 使用 `Pthread API`
* `FIFO`
* 先不考虑互斥问题
* 编译、运行、测试用例

## 二. 实验内容

### 1. 相关知识点：

* (1) 线程池的基本介绍：

  是一种软件设计模式，用于实现计算机程序中的执行并发。线程池维护多个线程，线程从线程池中的任务队列中按顺序取出任务进行执行。通过线程池可提高任务执行的性能并避免由于频繁创建和销毁短期任务的线程而导致的执行延迟。同时线程池的实现使将工作排入队列，按顺序加入并发过程，比手动管理线程完成起来更加轻松。

* (2) 简单线程池的基本结构：

  根据以上线程池的基本介绍，我们可以知道一个简单的线程池需要有一个任务队列的结构体，在本程序中，任务队列的数据结构设置为链表结构，线程池存放指向任务队列头结点的指针，存放线程ID的数组，便于使用 `Pthread API`，线程池的最大线程数，任务队列的当前任务数，已经标志线程池是否被摧毁（其中的线程是否能够被继续使用）的标记位ifShutdown。

* (3) 线程池的工作流程：

  * 在进程启动时创建多个线程并将其放置，进入线程池，线程等待工作；
  * 服务器收到请求后，将唤醒此线程池，并将请求传递给服务；
  * 线程完成其服务后，将返回到池中，并等待更多的工作。 如果该池不包含可用线程，服务器等待，直到一个空闲。

### 2. 程序关键代码以及思路：

* 线程池的结构体以及个变量的解释如下：

  ```c
  /*线程池的结构体*/
  typedef struct{
  
      threadTask *taskQueue; //线程池的任务队列*/
      int ifShutdown;      //是否摧毁线程池,为0表示不摧毁，为1表示不摧毁
      pthread_t *threadId; //线程的ID数组
      int maxThreadNum;    //线程池允许的最大线程数*/
      int curQueueNum;     //任务队列的当前任务数目*/
  
  } threadPool;
  ```

  其中任务队列的数据结构用链表的形式表示，任务队列的结点为threadTask结构体，该结构体以及个变量的解释如下：

  ```c
  /* 任务的结构体*/
  typedef struct task{
      void *(*calledFunc)(void *arg); //任务调用的函数
      void *arg;                      //传入调用函数的参数
      struct task *next;              //指向队列的下一个任务
  } threadTask;
  ```

  其中设置任务调用的测试函数calledFunc如下，用来打印哪个线程执行哪个任务：

  ```c
  /*任务调用的函数*/
  static void *calledFunc(void *arg){
      printf("Thread whose id is %ld is working on task %d.\n", (long int)pthread_self(), *(int *)arg);
      sleep(1);
  	return(NULL);
  }
  ```

* 线程池的相关操作函数如下：

  （为了便于在不同的函数之间传参，我们设置一个静态全局变量的线程池指针，指向我们要测试的线程池）

  ```c
  /*线程池实例*/
  static threadPool *pool = NULL;
  ```

  1. 初始化线程池：过程包括申请一个线程池的空间以及存放指定参数（最大线程数）大小的存放线程ID的数组，设置当前的任务队列头结点为空，当前任务队列的任务数量为0，设置线程池的最大线程数为指定的最大线程数，设置线程是否摧毁或者是否正在被摧毁的参数为0，调用 `pthread_create(&(pool->threadId[i]), NULL, poolFunc, NULL);`函数来创建`maxThreadNum`个线程放在线程池中，线程执行工作函数，在工作函数中等待接收任务队列中的任务执行。

     ```c
     /*初始化线程池*/
     void initPool(int maxThreadNum){
         //申请一个线程池并设置其参数
         pool = (threadPool *)malloc(sizeof(threadPool));
         pool->taskQueue = NULL;
         pool->curQueueNum = 0;             //任务队列为空
         pool->maxThreadNum = maxThreadNum; //设置最大的线程数为输入函数的参数
         pool->ifShutdown = 0;              //不摧毁线程池
         pool->threadId = (pthread_t *)malloc(maxThreadNum * sizeof(pthread_t));
     
         //创建线程
         for (int i = 0; i < maxThreadNum; i++){
             //线程执行poolFunc工作函数
             pthread_create(&(pool->threadId[i]), NULL, poolFunc, NULL);
         }
     }
     ```

     其中线程池中的线程执行的工作函数如下：

     线程执行工作函数遇到的情况有三如下：

     1. 当任务队列中没有等待的任务且线程池未被摧毁时，线程陷入循环等待；
     2. 当线程池被销毁或正在被摧毁时，线程不能接收任务队列中的任务去执行，直接退出工作函数；
     3. 当线程池未被摧毁且任务队列中有待执行的任务时，线程取出等待的任务队列中的头一个任务交给线程去执行，并且将任务队列长度减去1，释放任务结构体的空间。

     ```c 
     /*工作线程函数*/
     void *poolFunc(void *arg){
     	printf("-------------------------------------------------------\n");
         printf("Thread whose id is %ld calls poolFunc.\n", (long int)pthread_self());
     
         while(1){
             //当任务队列为空时，线程等待
             while (pool->curQueueNum <= 0 && !pool->ifShutdown){
                 printf("Thread whose id is %ld is waiting.\n", (long int)pthread_self());
             }
     
             //当线程池被销毁或正在被摧毁时，退出工作程序
             if (pool->ifShutdown){
                 printf("Thread whose id is %ld exits.\n", (long int)pthread_self());
                 pthread_exit(NULL);
             }
     
             //空闲线程开始执行任务
             printf("Thread whose id is %ld is starting to work.\n", (long int)pthread_self());
     
             //取出等待的任务队列中的头一个任务交给线程去执行，队列长度减去1
             pool->curQueueNum -= 1;
             threadTask *task = pool->taskQueue;
             pool->taskQueue = task->next;
     
             //执行任务
             (*(task->calledFunc))(task->arg);
     
             //任务执行完毕，销毁任务
             free(task);
         }
     	printf("-------------------------------------------------------\n");
         return (NULL);
     }
     ```

  2. 将任务加入线程池的任务队列：过程包括将传入的任务指定函数以及参数封装到一个任务队列节点的结构体中，便于插入到链表数据结构的任务队列，使用尾插法将创建的新任务节点插入到任务队列中，设置当前队列中的任务数增加1.

     ```c
     /*任务加入队列*/
     int addTaskToPool(void *(*calledFunc)(void *arg), void *arg){
     
         //创建一个新任务，并设置其参数
         threadTask *aTask = (threadTask *)malloc(sizeof(threadTask));
         aTask->calledFunc = calledFunc;
         aTask->arg = arg;
         aTask->next = NULL;
     
         //将创建后的任务放在任务队列中
         threadTask *tempTask = pool->taskQueue;
         if (tempTask != NULL){
             //队列有头结点
             while (tempTask->next != NULL)
                 tempTask = tempTask->next;
             tempTask->next = aTask;
         }
     
         else{
             pool->taskQueue = aTask;
         }
     
         pool->curQueueNum ++; //当前任务队列数加一
         return 0;
     }
     ```

  3. 销毁线程池：过程包括

     a. 为了避免多个线程同时摧毁线程池导致内存出现错误，这只线程池的一个互斥变量`ifShutdown`，当有程序在开始吹会线程池的时候，设置该互斥变量`ifShutdown`为1，表示该线程池正在被摧毁，该变量在线程池摧毁结束后并步恢复为0，因此也可以用来表示线程池已经被摧毁了。同时该变量的设置也是为了通知所有还未开始成功接收任务的线程退出工作函数。

     b. 函数调用`pthread_join(pool->threadId[i], NULL);`阻塞等待正在执行的线程工作结束

     c. 释放线程池空间的成员变量，包括任务队列，存放线程id的数组

     d. 调用`free(pool);`释放线程池本身

     ```c
     /*销毁线程池*/
     int destroyPool(){
     	printf("-------------------------------------------------------\n");
         printf("Begin destroying pool.\n");
         //线程池已摧毁或者在摧毁，返回，防止多个函数同时摧毁线程池
         if (pool->ifShutdown)
             return -1; 
         pool->ifShutdown = 1;
     
         //阻塞等待所有的线程退出并回收资源
         for (int i = 0; i < pool->maxThreadNum; i++)
             pthread_join(pool->threadId[i], NULL);
     
         //释放线程池空间，包括任务队列，id数组
         threadTask *head = NULL;
         while (pool->taskQueue != NULL){
             head = pool->taskQueue;
             pool->taskQueue = pool->taskQueue->next;
             free(head);
         }
         free(pool->threadId);
         //释放线程池本身
         free(pool);
         printf("Finish destroying pool.\n");
     	printf("-------------------------------------------------------\n");
         return 0;
     }
     ```

### 3. 执行结果分析：

* 使用./a.out 调用编译后的程序，因为命令行没有第二个第三个参数，因此程序使用默认的线程数4和默认的任务数5.执行结果如下：

  ![image-20210517105717135](https://hurq5.gitee.io/os-pictures/image-20210517105717135.png)

  ![image-20210517105821835](https://hurq5.gitee.io/os-pictures/image-20210517105821835.png)

  ![image-20210517105830507](https://hurq5.gitee.io/os-pictures/image-20210517105830507.png)

  分析：如图所示，任务1，2，3，4，5成功被添加到线程池中的任务队列中，并被随机的不忙碌的线程所执行并删除，每个任务只被执行一次，在这个程序中我们设置的任务执行函数为一秒，线程池存活的时间大概为`SLEEP_TIME`秒，因此每个线程在存活时间中可以运行的任务数`THREAD_WORK`大概等于`SLEEP_TIME`,因此在线程池存活时间内，4个线程可以执行THREAD_WORK*4个任务数，在程序中为20个任务，而当前只有5个任务，因此有长的一段时间线程处于等待的状态。

  ```c
  #define SLEEP_TIME 5
  #define THREAD_WORK SLEEP_TIME
  ```

* 按照上面的分析我们将线程数设置为1，任务数设置为5，即执行命令函`./a.out 1 5`，观察实验的结果,如下所示：

  ![image-20210517110453226](https://hurq5.gitee.io/os-pictures/image-20210517110453226.png)

  分析：线程在存活的时间内刚好可以执行完所有的任务，而没有空闲时间处于等待，此时线程池的执行效率最高。同理将线程数设置为2，任务数设置为10，如下，也可以得到线程刚好完成任务，么有等待的结果，此时线程池的效率也是很高的。

  ![image-20210517121044572](https://hurq5.gitee.io/os-pictures/image-20210517121044572.png)

  ![image-20210517121051344](https://hurq5.gitee.io/os-pictures/image-20210517121051344.png)

  综上可以观察到线程数量，任务数量以及线程存活时间有着密切的关系，因此在给定任务数以及最小执行时间的情况下，我们可以通过调整线程池的线程数来提高线程池执行的效率，在给定任务数和线程数的情况下，我们也可以估计出最小的任务全部完成的时间，调整线程池的存活时间，提高空间和时间效率。

  在该题目中，我们就可以设置一下代码（因为每个任务调用函数的执行时间大概为1s）来让程序执行的时间尽可能达到最小，线程池的效率达到最大：

  ```c
  if(taskNum%threadNum==0)
  	sleep(taskNum/threadNum);
  else 
  	sleep(taskNum/threadNum+1); 
  ```

  