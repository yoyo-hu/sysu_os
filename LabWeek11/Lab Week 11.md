# Lab Week 11.
## 实验要求：

编译运行课件 Lecture13 例程代码：Algorithms 13-1 ~ 13-8.

## 使用到的知识点：

相关知识点：

* pthread_create()函数：

  1. 函数功能：创建线程

  2. 函数原型以及个变量的解释

     ```c
     #include <pthread.h>
     int pthread_create(
     	pthread_t *restrict tidp,   //新创建的线程ID指向的内存单元。
     	const pthread_attr_t *restrict attr,  //线程属性默认为NULL，默认线程为非分离属性
         void *(*start_rtn)(void *), //新创建的线程从start_rtn函数的地址开始运行
         void *restrict arg //上述函数的参数，将参数放入结构中并将地址作为arg传入，若没有参数则为NULL
     );
     ```

* pthread_join()函数：

  

  1. 函数功能：

     阻塞调用线程，直到指定的线程终止，当`pthread_join()`返回之后，程序可回收该线程的资源。

     ```
     int pthread_join(
     	pthread_t tid, //指定等待的线程
         void **status  //存储等待线程的返回值，可以为NULL
     );
     ```

* 在编译的时候需要使用“ -lpthread”手动链接-线程库，这是因为pthread非linux系统的默认库

* gettimeofday()函数：

  1. 函数功能：获取当前的时间

  2. 函数原型：

     ```c
     int gettimeofday (struct timeval * tv, struct timezone * t)
     /*
     timeval结构体的定义：
     struct timeval{
         long tv_sec;  //秒
         long tv_usec;  //微秒
     };
     */
     ```

  2. 头文件：\#include <sys/time.h>   #include <unistd.h>

* strerror()函数：

     1. 函数功能：从内部数组中搜索错误号参数

     2. 函数原型：

        ```
        char strerror(int errnum)
        //errnum为要搜索的错误号
        ```

        

     3. 返回值：返回一个指向错误消息字符串的指针

* atoi 函数：是把字符串转换成整型数，并将该整型数返回

* pthread_attr_setstack()：设定线程栈的地址和大小

* 使用\#pragma omp parallel的程序在编译的过程中需要加上-fopenmp

* \#pragma omp parallel：通过定义代码块创建多线程，其格式如下：

      #pragma omp parallel  
      {
        执行的线程程序   
       }

* struct sigaction结构体

     ```c
     struct sigaction
     {
         void (*sa_handler) (int);//代表新的信号处理函数
         sigset_t sa_mask;//用来设置在处理该信号时暂时将sa_mask 指定的信号搁置
         int sa_flags;//设置信号处理的其他相关操作
         void (*sa_restorer) (void);
     }
     ```

* int sigemptyset(sigset_t *set)：初始化set所指向的信号集,使其中所有信号的对应的bit清零

*  int sigaddset(sigset_t *set,int signo)：在参数指定的信号集中添加指定的有效信号.

## 实验内容：

### alg.13-1-pthread-create-1.c

* 代码实现的功能：使用线程实现小于n（输入的数）的正整数的求和

* 源代码以及详细的注释：（包括实验内容的原理性和实现细节解释  ）

  ```c
  /* gcc -lpthread | -pthread */
  #include <stdio.h>
  #include <stdlib.h>
  #include <pthread.h>
  int sum; /* shared by thread(s) */
  static void *runner(void *); /* thread function */
  
  int main(int argc, char *argv[])
  {
      int *retptr; /* pointer variable in main-thread stack */
      int ret;
   	//输入的命令行参数需要是2个以上，其中第二个参数作为输入的数n
      if(argc < 2) {
          printf("usage: ./a.out <positive integer value>\n");
          return -1;
      }
  ty
      
      pthread_t ptid; /* thread identifier */
      pthread_attr_t attr; /* thread attributes structure */
      pthread_attr_init(&attr); /* set the default attributes */
        /* create the thread - runner with argv[1] */
      //创立线程，线程运行的是runner函数，函数传入的参数为argv[1]，创建的线程标识符为ptid，设置线程属性为默认线程，默认线程为非分离属性
      ret = pthread_create(&ptid, &attr, &runner, argv[1]);
      //线程创建成功返回0
      if(ret != 0) {
          //线程创建失败，打印失败原因，并退出程序
          perror("pthread_create()");
          return 1;
      }
  	//阻塞调用线程，直到标识符为ptid的线程终止
      ret = pthread_join(ptid, NULL);
      //调用成功返回0
      if(ret != 0) {
          //调用失败，打印失败原因并退出程序
          /* retptr points to the address in process heap, allocated in runner() */
          perror("pthread_join()");
          return 1;
      }
      //打印线程运算的结果
      printf("sum = %d\n", sum); 
      free(retptr);
      retptr = NULL;
      return 0;
  }
  
    /* The thread will begin control in this function */
  //实现小于n（输入的数）的正整数相加
  static void *runner(void *param)
  {
      int i, upper;
  	//把参数字符串param转换成整型数
      upper = atoi(param);
      sum = 0;
      for ( i = 1; i <= upper; i++)
          sum += i;
  
      pthread_exit(0);
  }
  
  ```

* 运行结果以及说明：

  ![image-20210505201123701](http://hurq5.gitee.io/os-pictures/image-20210505201123701.png)

  分析：可以观察到求和线程把小于等于输入数的正整数求和得到sum的结果（如第一个例子sum=1+2+3+4+5）

### alg.13-1-pthread-create-1-1.c

* 代码实现的功能：使用线程实现小于n（输入的数）的正整数的求和，以及返回值赋值为16.

* 相比较与alg.13-1-pthread-create-1.c的程序alg.13-1-pthread-create-1-1.c的程序，使用retptr存放返回值，程序返回数值为16的值作为返回值，如下：

  在主函数中：

  ```c
  	//阻塞调用线程，直到标识符为ptid的线程终止，线程执行返回值存储在参数retptr中，该值即是函数赋值的16
      ret = pthread_join(ptid, (void **)&retptr);
      //调用成功返回0
      if(ret != 0) {
          //调用失败，打印失败原因并退出程序
          /* retptr points to the address in process heap, allocated in runner() */
          perror("pthread_join()");
          return 1;
      }
      //打印返回的值
      printf("return val = %d\n", *retptr);
  ```

  在线程执行函数中：

  在线程空间中申请的内存空间，该空间位于堆段中，线程共享进程的堆段，因此可以作为结果值返回到主线程中

  ```c
  /* The thread will begin control in this function */
  //实现小于n（输入的数）的正整数相加
  static void *runner(void *param)
  {
      int i, upper;
  	//把参数字符串param转换成整型数
      upper = atoi(param);
      sum = 0;
      for ( i = 1; i <= upper; i++)
          sum += i;
  	
      int *retptr = (int *)malloc(sizeof(int)); /* allocated in process space */
      *retptr = 16; /* assigning */
      pthread_exit((void *)retptr);
  }
  ```

  

* 运行结果以及说明：

  ![image-20210505154042686](http://hurq5.gitee.io/os-pictures/image-20210505154042686.png)

  分析：可以观察到求和线程把小于等于输入数的正整数求和得到sum的结果（如第一个例子sum=1+2+3+4+5），返回值为赋值结果16.

### alg.13-1-pthread-create-1-2.c

* 代码实现的功能：使用线程实现小于n（输入的数）的正整数的求和，将求和的结果作为返回值返回

* 相比较与alg.13-1-pthread-create-1-1.c的程序alg.13-1-pthread-create-1-2.c修改的线程执行程序的返回值将求和的结果作为返回值返回：如下：

  sum变量之所以可以作为返回值返回到主线程中，这是因为sum变量为全局变量，是所有线程所共有的

  ```c
  static void *runner(void *param)
  {
      int i, upper;
  
      upper = atoi(param);
      sum = 0;
      for (i = 1; i <= upper; i++)
          sum += i;
  
      pthread_exit((void *)&sum); /* return the address in process .bss segment */
      /* also: return (void *)&sum; */
  }
  ```

* 运行结果以及说明：

  ![image-20210505154813755](http://hurq5.gitee.io/os-pictures/image-20210505154813755.png)

  分析：可以观察到求和线程把小于等于输入数的正整数求和得到sum的结果（如第一个例子sum=1+2+3+4+5），而返回值为运算的结果sum

### alg.13-1-pthread-create-1-3.c

* 代码实现的功能：使用线程实现小于n（输入的数）(被赋值为10）的正整数的求和，将求和的结果作为返回值返回

* 相比较与alg.13-1-pthread-create-1-1.c的程序和alg.13-1-pthread-create-1-2.c的程序，alg.13-1-pthread-create-1-3.c，给n赋值为10，使得运算的结果不受输入值的干扰，恒定为55，线程执行程序将求和的结果作为返回值返回：如下：

  ```c
    /* The thread will begin control in this function */
  static void *runner(void *param)
  {
      int *sum = (int *)param;
      int upper = 10;
      int i;
  
      *sum = 0;
      for (i = 1; i <= upper; i++)
          *sum += i;
  
      pthread_exit((void *)sum); /* return the address in process stack segment */
      /* also: return (void *)sum; */
  }
  ```

* 运行结果以及说明：

  ![image-20210505155236117](http://hurq5.gitee.io/os-pictures/image-20210505155236117.png)

  分析：无论输入的值为什么，sum的结果恒定为55（即1+2+3+4+5+6+7+8+9+10），而返回值为运算的结果sum。

### alg.13-1-pthread-create-2.c

* 代码实现的功能：使用线程实现小于n（输入的数）的正整数的求和，将字符串信息"Hello, world!"作为返回值返回

* 相比较与alg.13-1-pthread-create-1-2.c的程序，alg.13-1-pthread-create-2.c，将接收线程程序返回值的retptr定义为字符串类型，线程执行程序将字符串信息"Hello, world!"作为返回值返回：如下：

  在main中：

  ```c 
  char *retptr;
  ```

  在线程调用函数中：

  在线程空间中申请的内存空间，该空间位于堆段中，线程共享进程的堆段，因此可以作为结果值返回到主线程中

  ```c
    /* The thread will begin control in this function */
  static void *runner(void *param)
  {
      int i = 1;
      int upper = atoi(param);
  
      sum = 0;
      for (; i <= upper; i++)
          sum += i;
  
      char msg[] = "Hello, world!";
      char *retptr = (char *)malloc((strlen(msg)+1)*sizeof(char)); /* allocated in process space */
      strcpy(retptr, msg);
  
      pthread_exit((void *)retptr);
  }
  ```

* 运行结果以及说明：

  ![image-20210505160015810](http://hurq5.gitee.io/os-pictures/image-20210505160015810.png)

  分析：可以观察到求和线程把小于等于输入数的正整数求和得到sum的结果（如第一个例子sum=1+2+3+4+5），而返回值为字符串信息"Hello, world!"

### alg.13-1-pthread-create-3.c

* 代码实现的功能：建立指定个或者默认个线程，每个线程都返回将字符串信息"This is thread-%d, ptid = %lu"即返回线程的编号和标识符的信息

* 代码的主要部分以及较为详细的注释：

  线程执行的函数ftn：

  线程程序返回该字符串"This is thread-%d, ptid = %lu"，即返回线程的编号和标识符的信息

  ```c
  static void *ftn(void *arg)
  {
      int *numptr = (int *)arg;
      int num = *numptr;
  
      char *retval = (char *)malloc(80*sizeof(char));  /* allocated in the process heap */
    
      sprintf(retval, "This is thread-%d, ptid = %lu", num, pthread_self( ));
      printf("%s\n", retval);
  	//线程程序返回该字符串
      pthread_exit((void *)retval); /* or return (void *)retval; */
  }
  ```

  主函数main中的主要代码：

  函数输入的第二个参数为创立线程的个数，如果没有第二个参数，那么线程创立的数量默认为5

  ```c
      int max_num = 5;
  	//程序提示输入第二个参数作为要建立线程的数量
  	printf("Usage: ./a.out total_thread_num\n");
      if(argc > 1) {
          //将命令函的第二个参数的字符串类型转换成整数类型赋值给max_num
          max_num = atoi(argv[1]);
      }
  	//打印主函数的标识符和线程标识符
  	printf("main(): pid = %d, ptid = %lu.\n", getpid( ), pthread_self( ));
  ```

  创建max_num个线程：

  ```c
      //创建max_num个线程，线程标识符存储在ptid数组中，线程属性为默认属性：非分离属性，线程都运行函数ftn，传入的参数为i
      for (i = 0; i < max_num; i++) {
          ret = pthread_create(&ptid[i], NULL, ftn, (void *)&i);
          if(ret != 0) {
              fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
              exit(1);
          }
      }
  ```

  主线程阻塞等待线程执行结束

  ```c
  	//主线程阻塞等待所创建的max_num个线程执行结束，程序的返回值存储在字符串类型的retptr变量中，打印每个线程返回的字符串
  	for (i = 0; i < max_num; i++) {
          char *retptr;  /* retptr pointing to address allocated by ftn() */
          ret = pthread_join(ptid[i], (void **)&retptr);
          if(ret!=0) {
              fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
              exit(1);
          }
          printf("thread-%d: retval = %s\n", i, retptr);
          free(retptr);
          retptr = NULL;  /* preventing ghost pointer */
      }
  ```

* 运行结果以及说明：

  ![image-20210505165702274](http://hurq5.gitee.io/os-pictures/image-20210505165702274.png)

  分析:	可以观察到主线程（进程）有自己特定的进程标识符和线程标识符；未输入第二个参数时，程序默认建立5个线程，当输入第二个参数为3的时候，程序默认建立3个线程，但是可以观察到，线程传入的参数（作为线程的编号）都是0，这是因为创建进程太快，在新的线程在获取传递参数时，线程获取的变量值已经还未被主线程修改。

### alg.13-1-pthread-create-3-1.c

* 代码实现的功能：建立指定个或者默认个线程，每个线程都返回将字符串信息"This is thread-%d, ptid = %lu"即返回线程的编号和标识符的信息

* 相比较与alg.13-1-pthread-create-3.c的程序，alg.13-1-pthread-create-3.c程序在创建线程的for循环后面加了sleep(1），使得每个进程创建之间有一定的时间间隔，避免了主线程传入的变量值来不及被改变

* 运行结果以及分析：

  ![image-20210505175241235](http://hurq5.gitee.io/os-pictures/image-20210505175241235.png)

  ![image-20210505175247900](http://hurq5.gitee.io/os-pictures/image-20210505175247900.png)

  分析:	可以观察到主线程（进程）有自己特定的进程标识符和线程标识符；未输入第二个参数时，程序默认建立5个线程，当输入第二个参数为10的时候，程序默认建立10个线程，因为每个进程创建之间有一定的时间间隔，避免了主线程传入的变量值来不及被改变，因此线程的编号（传进线程的参数）是准确的（符合预期的）。

  

### alg.13-1-pthread-create-4.c

* 代码实现的功能：建立指定个或者默认个线程，每个线程都返回将字符串信息"This is thread-%d, ptid = %lu"即返回线程的编号和标识符的信息

* 相比较与alg.13-1-pthread-create-3.c的程序，alg.13-1-pthread-create-4.c程序增加了以下代码，即申请了一块内存来存入需要传递的参数，并且用内存中保存的值代替参数，如下：

  ```
      int thread_num[max_num];
      for (i = 0; i < max_num; i++) {
          thread_num[i] = i;
      }
  ```

  ```
  ret = pthread_create(&ptid[i], NULL, ftn, (void *)&thread_num[i]);
  ```

  

  这是为了避免直接在传递的参数中传递发生改变的量，否则会导致结果不可测。当再创造一个单线程，可能会再线程未获取传递参数时，线程获取的变量值已经被主线程进行了修改。

* 运行结果以及分析：

  ![image-20210505200346134](http://hurq5.gitee.io/os-pictures/image-20210505200346134.png)

  分析:	可以观察到主线程（进程）有自己特定的进程标识符和线程标识符；未输入第二个参数时，程序默认建立5个线程，当输入第二个参数为3的时候，程序默认建立3个线程，可以看到申请了一块内存来存入需要传递的参数，并且用内存中保存的值代替参数避免了主线程传入的变量值来不及被改变的问题，线程的编号（传进线程的参数）是准确的（符合预期的）。

### alg.13-2-pthread-shm.c

* 代码实现的功能：在主线程中定义并初始化一个结构体变量，包含三条信息，在生成的线程中修改该结构体变量，该结构体变量的成员信息被成功修改，这是因为所有的线程共用进程的栈段。

* 源代码的主要部分解释以及注释：

  程序定义了结构体msg_stru

  ```c
  struct msg_stru {
      char msg1[MSG_SIZE], msg2[MSG_SIZE], msg3[MSG_SIZE];
  }; 
  ```

  在main函数中（主线程中），定义了结构体变量，变量存储在主主线程的栈段中，因为该进程的所有线程共享栈段，因此在其他线程中也可以修改该变量的信息，在主线程中初始化msg.msg1为 "message 1 by parent"，初始化msg.msg2为 "message 2 by parent"，初始化msg.msg3为 "message 3 by parent"，并输出该结构体信息。

  ```
      struct msg_stru msg; /* storage in main-thread stack */
      sprintf(msg.msg1, "message 1 by parent");
      sprintf(msg.msg2, "message 2 by parent");
      sprintf(msg.msg3, "message 3 by parent");
      printf("\nparent say:\n%s\n%s\n%s\n", msg.msg1, msg.msg2, msg.msg3);
  	
  ```

  创建并阻塞等待两个线程的执行：

  ```c
      if(pthread_create(&tid1, &attr, &runner1, (void *)&msg) != 0) {
          perror("pthread_create()");
          return 1;
      }
      if(pthread_create(&tid2, &attr, &runner2, (void *)&msg) != 0) {
          perror("pthread_create()");
          return 1;
      }
      if(pthread_join(tid1, NULL) != 0) {
          perror("pthread_join()");
          return 1;
      }
      if(pthread_join(tid2, NULL) != 0) {
          perror("pthread_join()");
          return 1;
      }
  ```

  其中线程一执行runner1程序，将传入的msg变量的msg1成员修改为"message 1 changed by child1"

  ```c
  static void *runner1(void *param)
  {
      struct msg_stru *ptr = (struct msg_stru *)param;
      sprintf(ptr->msg1, "message 1 changed by child1");
      pthread_exit(0);
  }
  ```

  其中线程二执行runner2程序，将传入的msg变量的msg2成员修改为"message 2 changed by child2"

  ```
  static void *runner2(void *param)
  {
      struct msg_stru *ptr = (struct msg_stru *)param;
      sprintf(ptr->msg2, "message 2 changed by child2");
      pthread_exit(0);
  }
  
  ```

  在等待两个线程执行结束后，打印结构体变量中的信息，观察是否被成功改变。

* 运行结果以及分析：

  ![image-20210505204803744](http://hurq5.gitee.io/os-pictures/image-20210505204803744.png)

  分析：可以观察到主线程成功将结构体变量的成员msg.msg1初始化为 "message 1 by parent"，msg.msg2初始化为 "message 2 by parent"，msg.msg3初始化为 "message 3 by parent"，

  线程执行结束后，我们可以观察到线程一成功将主线程中定义的结构体变量的msg1成员修改为"message 1 changed by child1"，线程二成功将主线程中定义的结构体变量的msg2成员修改为"message 2 changed by child2"

  其他线程之所以可以修改主线程中的变量是因为所有的线程共用进程的栈段。

### alg.13-3-pthread-stack.c

* 代码实现的功能：在主线程中定义并初始化一个结构体变量，包含三条信息，在生成的线程中修改该结构体变量，该结构体变量的成员信息被成功修改，这是因为所有的线程共用进程的栈段。

* 源代码的主要部分解释以及注释：

  ```
  #define STACK_SIZE (524288-10000)*4096 /* 2^19 = 524288, STACK_SIZE is nearly 2G */
  ```

  在main函数中，主线程在stackptr处的位置申请了STACK_SIZE大小的空间，pthread_attr_setstack将线程的栈空间地址修改为该空间的地址，栈空间的大小修改为该空间的大小，即将该空间作为栈空间来使用。

  ```c
   char *stackptr = malloc(STACK_SIZE);
   pthread_attr_setstack(&tattr, stackptr, STACK_SIZE); 
   ret = pthread_create(&ptid, &tattr, &test, NULL); 
  ```

  在线程程序test中，递归调用程序，当编号为0到5和大于等于1965030时候程序输出八格大小的递归编号，当编号为6到1965029打印回退8格再打印八格大小的递归编号。每个递归程序在线程栈中定义了1024个字节的地址空间。

  ```c
  static void *test(void *arg)
  { 
      static int i =0;
      char buffer[1024]; /* 1KiB saved to the thread stack */
  
      if(i > 5 && i < 1965030)
          printf("\b\b\b\b\b\b\b\b%8d", i); 
      else
          printf("\niteration = %8d", i);
      i++; 
      test(arg); /* recursive calling until segmentation fault */ 
  }
  ```

* 运行结果以及分析：

  ![image-20210505211847863](http://hurq5.gitee.io/os-pictures/image-20210505211847863.png)

  分析：

  可以观察到程序按照test的打印规律输出，并且递归调用到编号为1965033的递归程序时程序出现段错误，这是因为线程栈已经满了，不能够在上面定义变量了，相关的栈空间计算如下：514288 * 4096-1965032 * 1024 = 94330880      94330880/1965032 = 48（字节），其中对于系统
  每次迭代的开销为48（字节）。

### alg.13-4-pthread-demo.c

* 代码实现的功能：使用#pragma omp parallel建立进程

* 源代码的主要代码解释以及注释：

  程序建立内核数（我的笔记本电脑的内核数是1）个线程运行第一个线程块，阻塞运行完运行该线程块中的所有线程后，建立2个线程运行第二个线程块，阻塞运行完运行该线程块中的所有线程后，建立4个线程运行第三个线程块，阻塞运行完运行该线程块中的所有线程后，建立6个线程运行第四个线程块，阻塞运行完运行该线程块中的所有线程后。

  ```c
      #pragma omp parallel
      {
          printf("parallel region 1. pid = %d, tid = %ld\n", getpid(), gettid());
      }
  
      #pragma omp parallel num_threads(2)
      {
          printf("parallel region 2 with num_thread(2). pid = %d, tid = %ld\n", getpid(), gettid());
      }
  
      #pragma omp parallel num_threads(4)
      {
          printf("parallel region 3 with num_thread(4). pid = %d, tid = %ld\n", getpid(), gettid());
      }
  
      #pragma omp parallel num_threads(6)
      {
          printf("parallel region 4 with num_thread(6). pid = %d, tid = %ld\n", getpid(), gettid());
      }
  ```

* 运行结果以及分析:

  ![image-20210505220507894](http://hurq5.gitee.io/os-pictures/image-20210505220507894.png)

  分析：程序在同一个进程中依次创建了1个线程运行线程程序一，2个线程运行线程程序二，4个线程运行线程程序三，6个线程运行线程程序四，这种运行是按顺序的。

### alg.13-5-openmp-matrixadd.c

* 代码实现的功能：使用#pragma omp parallel建立进程

* 源代码的主要代码解释以及注释：

  计算程序运行时长的程序：

  ```c
      gettimeofday(&t, 0);
      //获得当前的时间，即程序开始运行前的时间
      start_us = (long)(t.tv_sec * 1000 * 1000) + t.tv_usec;
      //开始的时间用微秒表示
  	/*此时为运行的程序*/
      gettimeofday(&t, 0);
      //获得当前时间，即程序结束运行后的时间
      end_us = (long)(t.tv_sec * 1000 * 1000) + t.tv_usec;
      //结束时间用微秒表示
      printf("Overhead time usec = %ld, omp thread = *\n", end_us-start_us);	
  	//用end_us-start_us计算程序运行的时长，其单位为微秒
  ```

  程序定义了全局变量如下：

  ```c
  int a[MAX_N][MAX_N], b[MAX_N][MAX_N];
  //两个用于相加的矩阵
  int ans[MAX_N][MAX_N];
  //矩阵相加的结果存放的地方
  int od = 10;
  //矩阵的大小，默认为10，可以被输入命令行的第二个参数设置
  ```

  main（）函数初始化a矩阵全部元素大小为20，b矩阵全部元素大小为30，依次用一个线程，两个线程和四个线程运行两个矩阵相加的程序matrixadd，并分别用上面所提到的计算程序运行时长的程序来计算和比较用一个线程，两个线程和四个线程运行两个矩阵相加所用的时长

* 运行结果以及分析：

  ![image-20210506004548680](http://hurq5.gitee.io/os-pictures/image-20210506004548680.png)

  分析：当没有输入矩阵阶数（即命令行没有第二个参数）的时候，矩阵的阶数默认为10，当输入按照预期（即命令行有第二个参数），矩阵的阶数对应的被设置为输入的数

  可以观察到和预期相反，多线程的计算时长反而比单线程的执行时长慢，由上一个程序可以知道我的笔记本电脑为单核的笔记本电脑，使用如下命令行也可以知道该电脑为单CPU单核的电脑

  ![image-20210506004916288](http://hurq5.gitee.io/os-pictures/image-20210506004916288.png)

  之所以会使用多个线程比使用单个线程计算时间长，是因为当线程数量比核的数量多时，线程被频繁切换，所花费掉的多余时间更加长，反而会降低计算效率，当线程更多时，这种切换就会更加的频繁，因此程序的执行时长将更长，程序计算的效率将更低。

### alg.13-6-pthread-demo1.c

* 代码实现的功能：在多线程程序中主函数中调用fork（）,观察他们之间的协调性。

* 源代码的主要代码解释以及注释：

  在main（）函数中，创建线程后，又使用fork（）建立子进程（又可以说用fork创建线程，因为在linux下进程和线程是没有区分的）

  ```c
      pthread_create(&ptid, NULL, &thread_worker, NULL);
  
      pid_t pid = fork(); /* child duplicates parent's main thread only, without thread_worker */
  ```

  在子进程和父进程中都使用“system("ps -l -T");”的系统调用查看进程线程状态表

* 运行结果及分析：

  ![image-20210506012348134](http://hurq5.gitee.io/os-pictures/image-20210506012348134.png)

  分析：可以观察到pid为2685的父进程同时创立了两个pid相同，都等于3004的进程或者线程，可以推断到这两个pid相同的线程，是分别由pthread_create和fork函数创建的，这种两个线程标识符相同的情况是不允许存在的。

  其次，我们通过观察到父进程存在孙子线程可以推断到，fork产生的子进程，同时复制了父进程的线程。

  综合上面的观察我们可以得出：

  子进程和线程的标识符相同，是因为子进程是通过该一个线程创建的，程序会将该线程复制到子进程中，但是这种存在两个线程标识符相同的情况是不被允许的，其次不能同时创建出于父进程一样多线程的子进程。

  其次，除了被子进程复制的线程外，如果还存在其他线程，这些其他线程都会在在子进程中停止并消失，并且程序不会为其调用清理函数以及针析构函数等。

### alg.13-7-pthread-demo2.c

* 代码实现的功能：在多线程程序中线程函数中调用fork（）,观察他们之间的协调性。

* 源代码的主要代码解释以及注释：

  在main（）函数中创建线程并使用“system("ps -l -T");”的系统调用查看进程线程状态表，该线程运行以下程序，线程运行的函数使用fork（）创建子进程，并且在该子进程中使用“system("ps -l -T");”的系统调用查看进程线程状态表。

  ```c
  static void *thread_worker(void *args)
  {
      pid_t pid = fork(); /* the forked child takes thread_worker as main thread. This may cause unexpect behaviours in synchronization or signal handling */
      if(pid < 0 ) {
          return (void *)EXIT_FAILURE;
      }
      if(pid == 0) { /* child pro */
          i = 1;
          printf("in thread_worker's forked child\n");
          system("ps -l -T | grep a.out");
      }
      sleep(2);
      while (1) {
          printf("%d\n", i); 
          /* will print '0' by thread_worker of parent main(); '1' by forked child pro */
          sleep(2);
      }   
      pthread_exit(0);
  }
  ```

* 运行结果以及分析：

  ![image-20210506015506311](http://hurq5.gitee.io/os-pictures/image-20210506015506311.png)

  分析:上述运行的结果和alg.13-6运行的结果相同，可以观察到pid为3125的父进程同时创立了两个pid相同，都等于3213的进程或者线程，可以推断到线程创立的子进程也等同于父进程创立了子进程，因为线程属于进程。

  其次，我们通过观察到父进程存在孙子线程可以推断到，fork产生的子进程，同时复制了父进程的线程。

  综合上面的观察我们可以得出：

  子进程和线程的标识符相同，是因为子进程是通过线程创建的，程序会将该线程复制到子进程中。

  

### alg.13-8-sigaction-demo.c

* 代码实现的功能：程序一开始先对与检查或修改与指定信号相关联的处理动作相关的结构体进行一些初始化操作，程序等待用户按下Ctrl+c，用户在终端按下Ctrl+c时，程序捕捉并进入处理程序，在处理程序执行的过程中，按下Ctrl+\所产生的信号量是暂时被屏蔽，sleep10s后，处理程序完成屏蔽消失，程序继续等待捕捉Ctrl+c产生的信号量，此时若输入为Ctrl+\，程序出现core dumped错误结束。

* 源代码及其详细注释：

  ```c
  #include <stdio.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <signal.h>
   
  void my_handler(int signo)  /* user signal handler */
  {
      printf("\nhere is my_handler");
      printf("\nsignal catched: signo = %d", signo);
      printf("\nCtrl+\\ is masked");
      printf("\nsleeping for 10 seconds ... \n");
      sleep(10); //线程在执行的过程中Ctrl+\是被屏蔽的
      printf("my_handler finished\n");
      printf("after returned to the main(), Ctrl+\\ is unmasked\n");
      return;
  }
  
  int main(void)
  {
      int ret;
      //一个与检查或修改与指定信号相关联的处理动作相关的结构体
      struct sigaction newact;
   	//设置处理信号函数为用户自定义的my_handler()函数。
      newact.sa_handler = my_handler; /* set the user handler */
      //初始化newact信号集sa_mask成员,使其中所有信号的对应的bit清零，表示该信号集不包含任何有效信号
      sigemptyset(&newact.sa_mask); /* clear the mask *
      //在该信号集中添加信号编号为3的SIGQUIT信号
      //用户在终端按下ctrl-\时，终端驱动程序会发送信号SIGQUIT信号给前台进程
      sigaddset(&newact.sa_mask, SIGQUIT); /* sa_mask, set signo=3 (SIGQUIT:Ctrl+\) */
      newact.sa_flags = 0; /* default */
  
      printf("now start catching Ctrl+c\n");
      //signum参数指出要捕获的信号类型
      //newact指定新的信号处理方式
      //sigaction()会依SIGINT指定的信号编号指定新的信号处理方式newact
      ret = sigaction(SIGINT, &newact, NULL); /* register signo=2 (SIGINT:Ctrl+C) */
      if(ret == -1) {
          //sigaction执行失败，退出程序
          perror("sigaction()");
          exit(1);
      }
   	//表示线程一直执行
      while (1);
   
      return 0;
  }
  
  ```

* 执行结果以及分析：

  ![image-20210506023116608](http://hurq5.gitee.io/os-pictures/image-20210506023116608.png)

分析：程序的执行和上述代码实现的功能相同，当终端按下Ctrl+c程序捕捉并进入处理程序，在这期间按下Ctrl+\所产生的信号量是暂时被屏蔽，直到处理程序运行结束，该信号量才能成功被接收并起作用，当Ctrl+\起作用时程序出现core dumped错误结束。



