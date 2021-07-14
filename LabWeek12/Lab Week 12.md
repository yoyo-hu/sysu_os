# Lab Week 12.

## 1. 编译运行课件 Lecture14 例程代码： Algorithms 14-1 ~ 14-7.

### 1.1 Algorithms 14-1

#### 1.1.1 相关知识点：

1. 线程局部存储（TLS），是一种变量的存储方法，这个变量在它所在的线程内是全局可访问的，但是不能被其他线程访问到，这样就保持了数据的线程独立性。

2.  __NR_gettid和SyS_gettid的意义相同，linux下的系统调用syscall(SYS_gettid)或者syscall(__NR_gettid)获得一个真实的线程id唯一标识tid。

   ```
   #define gettid() syscall(__NR_gettid)
   ```

3. 可以使用语言级别的调用来定义TLS空间中的变量，样例如下：

   ```c
   __thread int i;
   extern __thread struct state s;
   static __thread char *p
   ```

#### 1.1.2 程序的主要实现细节：

程序在主线程中创建两个子线程，主线程和子线程一起调用以下的程序块，对tlsvar变量进行5次加一操作，加一的时间具有一定的随机性。

```c
    for (int i = 0; i < 5; ++i) {
        randomcount = rand() % 100000;
        for (int k = 0; k < randomcount; k++) ;
        printf("%ld, tlsvar = %d\n", gettid(), tlsvar);
        tlsvar++; /* main- thread has its local tlsvar */
    }
```

#### 1.1.3 程序运行结果及分析：

![image-20210510164205860](http://hurq5.gitee.io/os-pictures/image-20210510164205860.png)

分析：从输出可以看出，主线程，和两个子线程tid1、tid2都使用了tls变量tlsvar且该变量的变化是线程间互不影响的，线程各自都拥有各自的tlsvar，可以得出tls变量的存储实现了线程基本的数据独立。

### 1.2 Algorithms 14-2

#### 1.2.1 相关知识点：

1. POSIX的pthread.h提供了一组API来实现此功能,包括以下的函数

   a. pthread_key_create()函数：

   * 函数作用：用来创建线程私有数据，私有数据采用的是一键对多值的技术

   * 函数原型以及参数含义：

     ```
     int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
     //key:从 TSD 池中分配一项，将其地址值赋给key供以后访问使用
     //destructor:销毁函数,为NULL则系统调用默认的销毁函数进行相关的数据注销
     ```

   * 头文件：\#include <pthread.h>

   b. pthread_setspecific()函数:

   * 函数作用：该函数将value的值与key相关联

   * 函数原型：

     ```
     int pthread_setspecific(pthread_key_t key, const void *value);
     ```

   * 头文件:\#include <pthread.h>

   * 返回值：成功返回0.错误返回值包括：ENOMEM（虚拟内存不足），EINVAL:key（无效）

   c. pthread_getspecific()函数:

   * 函数作用：确认是否已经于对应的key相关联的变量

   * 函数原型：

     ```
     void *pthread_getspecific(pthread_key_t key);
     ```

   * 返回值：返回指针，如果没有与线程相关的数据与键关联，则返回NULL

   d. pthread_key_delete()函数:

   * 函数作用：删除键，其所占用的内存会被释放

   * 函数原型：

     ```
     int pthread_key_delete(pthread_key_t key)
     ```

2. system("lsof +d ./log"): 列出目录./log的所有打开的实例
3. system("cat ./log/thread-1.log ./log/thread-5.log")：查看. ./log目录下thread-1.log和thread-5.log文件的内容

#### 1.2.2 程序的主要实现细节：

1. 程序使用``pthread_key_create(&log_key, &close_log_file)``：用来创建线程私有数据，并将其地址值赋给log_key，使用close_log_file来销毁该数据。

2. 程序调用``pthread_create(&tids[i], NULL, &thread_worker, NULL)`创建5个子线程同时运行以下的thread_worker函数，并调用`pthread_join(tids[i], NULL)`阻塞等待全部线程执行完毕，并做好资源的回收

   ```c
   void write_log(const char *msg)
   {	
       FILE *fp_log;
       fp_log = (FILE *)pthread_getspecific(log_key); /* fp_log is shared in the same thread */
       fprintf(fp_log, "writing msg: %s\n", msg);
       printf("log_key = %d, tid = %ld, address of fp_log %p\n", log_key, gettid(), fp_log);
   }
   static void *thread_worker(void *args)
   {
       static int thcnt = 0;
       char fname[64], msg[64];
       FILE *fp_log; /* a local variable */
   	
       sprintf(fname, "log/thread-%d.log", ++thcnt);  /* directory ./log must exist */
       fp_log = fopen(fname, "w");
       if(!fp_log) {
           printf("%s\n", fname);
           perror("fopen()");
           return NULL;
       }
   
       pthread_setspecific(log_key, fp_log); /* fp_log is associated with log_key */
    
       sprintf(msg, "Here is %s\n", fname);
       write_log(msg);
   }
   ```

   在thread_worker函数中，

   * fname是用来存储文件路径名的，该文件为指定线程号的log日志
   * ```fopen(fname, "w")```表示在```"./log"```文件目录下创建该文件
   * ```pthread_setspecific(log_key, fp_log)```表示关联置当前线程中的```log_key```与```fp_log```
   * 调用write_log函数记录该线程运行的信息，函数传入参数msg（"Here is %s\n", fname）

   在write_log函数中：

   * ```fp_log = (FILE *)pthread_getspecific(log_key)```表示log_key的内容，并转换为```FILE *```类型赋给变量```fp_log```
   * 打印log_key，线程号，以及fp_log（` "writing msg: %s\n", msg`）

3. 程序调用`pthread_key_delete(log_key)`删除键log_key，以及释放相关的空间

4. 程序调用`"lsof +d ./log"`列出目录./log的所有打开的实例

5. 程序调用`"cat ./log/thread-1.log ./log/thread-5.log"`查看./log/thread-1.log ./log目录下thread-5.log文件的内容

#### 1.2.3 程序运行结果分析：

![image-20210510194105114](http://hurq5.gitee.io/os-pictures/image-20210510194105114.png)

分析：不同的线程使用的是同一个键但是，但是使用的是不同的数值。

![image-20210510195913853](http://hurq5.gitee.io/os-pictures/image-20210510195913853.png)

分析：线程运行结束后，目录./log的所有打开的实例为空，没有实例处于打开状态。

![image-20210510195955293](http://hurq5.gitee.io/os-pictures/image-20210510195955293.png)

分析：查看. ./log目录下thread-1.log和thread-5.log文件的内容，发现存在该文件，即thcnt的值成功从1增加到了5。

### 1.3 Algorithms 14-3

#### 1.3.1 程序的主要实现细节：

1. 主函数使用``pthread_key_create(&tls_key, NULL)``：用来创建线程私有数据，并将其地址值赋给tls_key。

2. 使用 `pthread_create(&ptid1, NULL, &thread_func1, NULL);`和`pthread_create(&ptid2, NULL, &thread_func2, NULL);`创建两个线程，分别运行函数thread_func1和函数thread_func2；

3. 调用pthread_join函数阻塞等待两个线程执行完毕，回收资源后；

4. 调用`pthread_key_delete(tls_key)`函数删除键tls_key，以及释放相关的空间。

5. 在线程一调用的函数thread_func1代码如下：

   ```c
   static void *thread_func1(void *args)
   {
       struct msg_struct1 ptr[5]; /* local variable in thread stacke */
       printf("thread_func1: tid = %ld   ptr = %p\n", gettid(), ptr);
   	//绑定这个线程中的tls_key与ptr
       pthread_setspecific(tls_key, ptr); /* binding ptr to the tls_key */
   	//设置这个线程中的ptr[0]的学生学号stuno为18000001和学生姓名stuname为Alex,ptr[4]的学生学号stuno为18000005和学生姓名stuname为Michael
       sprintf(ptr[0].stuno, "18000001");
       sprintf(ptr[0].stuname, "Alex");
       sprintf(ptr[4].stuno, "18000005");
       sprintf(ptr[4].stuname, "Michael");
       print_msg1();
   
       pthread_exit(0);
   }
   ```

   thread_func1程序

   * 打印了该线程的线程号tid和申请的本地变量ptr的地址，将该本地变量和线程中的tls_key进行绑定，使得该变量在这个线程中被全局拥有
   * 设置这个线程中的ptr[0]的学生学号stuno为18000001和学生姓名stuname为Alex,ptr[4]的学生学号stuno为18000005和学生姓名stuname为Michael，执行print_msg1()函数,如下所示：

   ```c
   void print_msg1(void)
   {	
       int randomcount;
   
       struct msg_struct1 *ptr = (struct msg_struct1 *)pthread_getspecific(tls_key);
       printf("print_msg1:   tid = %ld   ptr = %p\n", gettid(), ptr); 
   	    /* sharing storage with thread_func1 */ 
   
       for (int i = 1; i < 6; i++) {
           randomcount = rand() % 10000;
           for (int k =0; k < randomcount; k++) ;
           printf("tid = %ld  i = %2d   %s  %*.*s\n", gettid(), i, ptr->stuno, 8, 8, ptr->stuname);
           ptr++;
       }
       
       return;
   }
   ```

   print_msg1()函数：

   * 调用```pthread_getspecific()```获取与线程私有变量```tls_key```绑定的并转化为 struct msg_struct1*类型变量赋值给ptr。
   * 打印线程号和```ptr```数组的首地址
   * 在随机时间（分别等待随机时间) 分别打印5个线程的信息：线程号，线程编号，其私有变量ptr的学生学号和学生姓名。

   ```c
   static void *thread_func2(void *args)
   {
       struct msg_struct2 *ptr;
       ptr = (struct msg_struct2 *)malloc(5*sizeof(struct msg_struct2)); /* storage in process heap */
       printf("thread_func2: tid = %ld   ptr = %p\n", gettid(), ptr); 
   
       pthread_setspecific(tls_key, ptr);
   
       ptr->stuno = 19000001;
       sprintf(ptr->stuname, "Bob");
       sprintf(ptr->nationality, "United Kingdom");
       (ptr+2)->stuno = 19000003;
       sprintf((ptr+2)->stuname, "John");
       sprintf((ptr+2)->nationality, "United States");
       print_msg2();
   
       free(ptr);
       ptr = NULL;
   
       pthread_exit(0);
   }
   ```

   thread_func2程序：

   * 打印了该线程的线程号tid和申请的本地变量ptr的地址，将该本地变量ptr和线程中的tls_key进行绑定，使得该变量在这个线程中被全局拥有。
   * 设置这个线程中的ptr[0]的学生学号`stuno`为`19000001`，学生姓名`stuname`为`Bob`和学生的国籍`nationality`为`United Kingdom,`ptr[2]的学生学号`stuno`为`19000003`，学生姓名`stuname`为`John`和学生的国籍`nationality`为`United States`，执行`print_msg2()`函数,如下所示：

   ```c
   void print_msg2(void)
   {	
       int randomcount;
       struct msg_struct2* ptr = (struct msg_struct2 *)pthread_getspecific(tls_key);
       printf("print_msg2:   tid = %ld   ptr = %p\n", gettid(), ptr);
   
       for (int i = 1; i < 6; i++) {
           randomcount = rand() % 10000;
           for (int k =0; k < randomcount; k++) ;
           printf("                                          tid = %ld  i = %2d   %d  %*.*s   %s\n", gettid(), i, ptr->stuno, 8, 8, ptr->stuname, ptr->nationality);
           ptr++;
       }
       
       return;
   }
   ```

   print_msg2()函数：

   * 调用```pthread_getspecific()```获取与线程私有变量```tls_key```绑定的变量，并转化为 struct msg_struct2*类型变量赋值给ptr。
   * 打印线程号tid和```ptr```数组的首地址
   * 在随机时间（分别等待随机时间) 分别打印5个线程的信息：线程号，线程编号，其私有变量ptr的学生学号，学生姓名和学生国籍。

#### 1.3.2 程序运行结果及分析：

![image-20210511083149662](http://hurq5.gitee.io/os-pictures/image-20210511083149662.png)

分析：观察以上执行结果可以得出，在不同线程中绑定tls_key后的变量ptr位于不同的内存空间中，且这两个不同线程中的私有变量的结构体结构不同，彼此为独立变量，该线程本地变量的数值和线程本身设定的结果一致，说明线程私有变量ptr可以由该线程设定和修改。

### 1.4 Algorithms 14-4

#### 1.4.1 程序的主要实现细节：

相比较与程序14-3，程序14-4的改变如下：

1. 线程一和线程二的tls变量使用了相同结构的结构体：

   ```c
   struct msg_struct {
       char pos[80];
       char stuno[9];
       char stuname[20];
   };
   ```
   
   其中pos表示打印信息的位置，stuno表示学生的学号，stuname表示学生的名字
   
2. 在线程执行的函数thread_func1和thread_func2中添加私有结构体变量的初始化：

   ```c
   for (int i =0; i<5; i++) {
       sprintf(ptr[i].pos, " ");
           sprintf(ptr[i].stuno, "        ");
           sprintf(ptr[i].stuname, "                   ");
   }
   ```

3. 在打印信息的函数中，直接通过输出ptr->pos变量，使得结构体信息打印在指定位置。

   ```c
   printf("%stid = %ld  i = %2d   %s  %*.*s\n", ptr->pos, gettid(), i, ptr->stuno, 8, 8, ptr->stuname);
   ```

#### 1.4.2 程序运行结果及分析：

![image-20210511090859279](http://hurq5.gitee.io/os-pictures/image-20210511090859279.png)

分析：观察以上执行结果可以得出，在不同线程中绑定tls_key后的变量ptr位于不同的内存空间中，彼此为独立变量，该线程本地变量的数值和线程本身设定的结果一致，说明线程私有变量ptr可以由该线程设定和修改。

### 1.5 Algorithms 14-5

#### 1.5.1 程序的主要实现细节：

1. 主函数使用``pthread_key_create(&tls_key, NULL)``：用来创建线程私有数据，并将其地址值赋给`tls_key`。

2. 使用 `pthread_create(&ptid1, NULL, &thread_func, NULL)`创建一个线程，分别运行函数`thread_func`；

3. 调用pthread_join函数阻塞等待线程执行完毕，回收资源后；

4. 调用`pthread_key_delete(tls_key)`函数删除键tls_key，以及释放相关的空间。

5. 在线程调用的函数thread_func代码如下：

   ```c
   static void *thread_func(void *args)
   {
       struct msg_struct *ptr;
   
       thread_data1();
       ptr = (struct msg_struct *)pthread_getspecific(tls_key); /* get ptr from thread_data1() */
       perror("pthread_getspecific()");
       printf("ptr from thread_data1() in thread_func(): %p\n", ptr);
       for (int i = 1; i < 6; i++) {
           printf("tid = %ld  i = %2d   %s  %*.*s\n", gettid(), i, (ptr+i-1)->stuno, 8, 8, (ptr+i-1)->stuname);
       }
   
       thread_data2();
       ptr = (struct msg_struct *)pthread_getspecific(tls_key); /* get ptr from thread_data2() */
       perror("pthread_getspecific()");
       printf("ptr from thread_data2() in thread_func(): %p\n", ptr);
       for (int i = 1; i < 6; i++) {
           printf("tid = %ld  i = %2d   %s  %*.*s\n", gettid(), i, (ptr+i-1)->stuno, 8, 8, (ptr+i-1)->stuname);
       }
   
       free(ptr);
       ptr = NULL;
   
       pthread_exit(0);
   }
   ```

   在thread_func函数中：

   1. 线程调用thread_data1函数，函数在栈区定义了结构体数组ptr，并绑定到tls_key上。

      在thread_data1函数中

      ```c
      void thread_data1(void)
      {
          struct msg_struct ptr[5]; /* in thread stack */
          pthread_setspecific(tls_key, ptr); /* binding the tls_key to address of ptr */
          printf("ptr in thread_data1(): %p\n", ptr);
      
          for (int i = 0; i < 5; i++) {
              sprintf(ptr[i].stuno, "        ");
              sprintf(ptr[i].stuname, "                   ");
          }
          sprintf(ptr[0].stuno, "19000001");
          sprintf(ptr[0].stuname, "Bob");
          sprintf(ptr[2].stuno, "19000003");
          sprintf(ptr[2].stuname, "John");
      
          return;
          /* thread stack space is deallocated when thread_data1() returns and data lost */ 
      }
      ```

      函数还对该私有变量进行了以下的初始化：先初始化变量为指定空格数，再初始化ptr[0]的学生学号`stuno`为"19000001"和学生姓名`stuname`为"Bob"，初始化ptr[2]的学生学号`stuno`为"19000003"和学生姓名`stuname`为 "John".

   2. thread_data1函数执行结束后，在thread_func中调用` ptr = (struct msg_struct *)pthread_getspecific(tls_key); /* get ptr from thread_data1() */`获取和tls_key绑定的变量，并转换为struct msg_struct *类型赋值为ptr。

   3. 打印线程的线程号tid，步骤二中获得变量的学生信息表（包括学生编号，学生学号，学生姓名）

   4. 调用thread_data2函数，函数动态申请了结构体数组ptr，并将其绑定到tls_key上。

      ```c
      void thread_data2(void)
      {
          struct msg_struct *ptr;
          ptr = (struct msg_struct *)malloc(5*sizeof(struct msg_struct));  /* in process heap */
          pthread_setspecific(tls_key, ptr); /* binding the tls_key to address of ptr */
          printf("ptr in thread_data2(): %p\n", ptr);
      
          for (int i = 0; i < 5; i++) {
              sprintf(ptr[i].stuno, "        ");
              sprintf(ptr[i].stuname, "                   ");
          }
          sprintf(ptr->stuno, "19000001");
          sprintf(ptr->stuname, "Bob");
          sprintf((ptr+2)->stuno, "19000003");
          sprintf((ptr+2)->stuname, "John");
      
          return;
          /* the heap space is kept effective if ptr is not freed */
          /* if free(ptr) before return, the space is reallocated and data lost */
          /* need to free the space in thread_func or there is a memory leak */
      }
      ```

      函数还对该私有变量进行了以下的初始化：先初始化变量为指定空格数，再初始化ptr[0]的学生学号`stuno`为"19000001"和学生姓名`stuname`为"Bob"，初始化ptr[2]的学生学号`stuno`为"19000003"和学生姓名`stuname`为 "John".

   5. thread_data2函数执行结束后，在thread_func中调用` ptr = (struct msg_struct *)pthread_getspecific(tls_key); /* get ptr from thread_data1() */`获取和tls_key绑定的变量，并转换为struct msg_struct *类型赋值为ptr。

   6. 打印线程的线程号tid，步骤五中获得变量的学生信息表（包括学生编号，学生学号，学生姓名）

   #### 1.5.2 程序运行结果及分析：

   ![image-20210511100534262](http://hurq5.gitee.io/os-pictures/image-20210511100534262.png)

   分析：可以观察到，thread_data1函数执行结束后返回到原函数输出，打印在其中定义的私有变量的数据，发现数据丢失，而thread_data2函数执行结束后返回到原函数输出，打印在其中定义的私有变量的数据，发现数据完全，未发生丢失；

   这是因为

   1. 在thread_data1中定义的tls变量存储在函数的栈中，当函数执行完毕时，栈的内存会被回收，因此该变量的数据将不复存在。
   2. 而对于thread_data2中定义的tls变量是动态申请的变量，存储在堆中，释放该空间中的内存，需要手动调用free函数，因此thread_data2函数执行结束后，该内存的数据仍然存在，可以在线程函数中被完整的打印出来；

   其次，我们可以观察到当和tls_key 绑定的数据发生丢失时，tls_key 任然可以继续使用，依旧可以和新的变量进行绑定。线程正常执行。

### 1.6 Algorithms 14-6

#### 1.6.1 相关知识点：

* close函数：

  1. 函数作用：创建子进程

  2. 函数原型以及参数的解释

     ```c
      #define _GNU_SOURCE
      #include <sched.h>
     
     int clone(int (*fn)(void *), void *stack, int flags, void *arg, ...
         /* pid_t *parent_tid, void *tls, pid_t *child_tid */ );
     ```

     * 创建子进程时，子进程会执行函数``fn``,参数`arg`传入``fn``中作为函数`fn`的参数。当`fn(arg)`函数返回后，子进程就会退出。`fn`返回的整数为子进程的返回状态。可以通过调用`exit(2)`或接收终止信号来结束进程。

     * `stack`参数指定了子进程使用的栈的位置。由于子进程和调用进程可能会共享内存，因此不能在调用进程的栈中运行子进程。调用进程必须为子进程的栈配置内存空间，并向`clone()`传入一个执行该空间的指针。

     * flags就是标志需要从父进程继承的资源，可以取到以下的值：

       | **标志**      | **含义**                                                     |
       | ------------- | ------------------------------------------------------------ |
       | CLONE_PARENT  | 创建的子进程的父进程和调用者的父进程相同，子进程和调用者为兄弟关系 |
       | CLONE_FS      | 子进程与父进程共享相同的文件系统                             |
       | CLONE_FILES   | 子进程与父进程共享相同的文件描述符                           |
       | CLONE_NEWNS   | 在新的namespace启动子进程，namespace描述了进程的文件hierarchy |
       | CLONE_SIGHAND | 子进程与父进程共享相同的信号处理表                           |
       | CLONE_PTRACE  | 若父进程被trace，子进程也被trace                             |
       | CLONE_VFORK   | 父进程被挂起，直至子进程释放虚拟内存资源                     |
       | CLONE_VM      | 子进程与父进程运行于相同的内存空间                           |
       | CLONE_PID     | 子进程在创建时PID与父进程一致                                |
       | CLONE_THREAD  | 子进程与父进程共享相同的线程群                               |

* 在clone中设置SIGCHLD目的：设置子进程在终止后会向父进程发送SIGCHLD信号，通知父进程回收该子进程的资源，信号的默认会被忽略，需要设置捕捉处理来实现子进程的回收。

#### 1.6.2 程序的主要实现细节：

1. 主函数先为即将创建的两个子进程申请系统堆栈：

   ```c
       //为子进程申请系统堆栈
       char *stack1 = malloc(STACK_SIZE*sizeof(char)); /* allocating from heap, safer than stack1[STACK_SIZE] */
       char *stack2 = malloc(STACK_SIZE*sizeof(char));
   ```

2. 设置clone的flag位：

   ```c
         /* set CLONE flags */
       if((argc > 1) && (!strcmp(argv[1], "vm"))) {
           flags |= CLONE_VM;//子进程与父进程运行于相同的内存空间
       }
       if((argc > 2) && (!strcmp(argv[2], "vfork"))) {
           flags |= CLONE_VFORK;//父进程被挂起，直至子进程释放虚拟内存资源
       }
   ```

   当`argv[1]`存在并等于vm时，子进程与父进程运行于相同的内存空间，当`argv[2]`存在并等于`vfork`时，父进程被挂起，直至子进程释放虚拟内存资源。

3. 主函数打印自己的进程号`pid`并打印提示自己要开始使用clone创建子进程

4. 主函数调用`chdtid1 = clone(child_func1, stack1 + STACK_SIZE, flags | SIGCHLD, buf); /* what happened if without SIGCHLD */`来创建子进程，子进程执行函数child_func1,并传入参数buf，该新进程的系统堆栈的栈顶地址为`stack1 + STACK_SIZE`，`flags`就是标志需要从父进程继承的资源,设置`SIGCHLD`是为了使得子进程在结束时候可以通知父进程去回收它的资源，与后面`waitpid`函数形成照应。

   第一个子进程执行的child_func1函数如下：

   ```c
   static int child_func1(void *arg)
   {
       char *chdbuf = (char*)arg; /* type casting */
       printf("child_func1 read buf: %s\n", chdbuf);
       sleep(1);
       sprintf(chdbuf, "I am child_func1, my tid = %ld, pid = %d", gettid(), getpid());
       printf("child_func1 set buf: %s\n", chdbuf);
       sleep(1);
       printf("child_func1 sleeping and then exists ...\n");
       sleep(1);
   
       return 0;
   }
   ```

   在child_func1函数中：

   1. 将读入的参数转换成字符串类型赋值给chdbuf，并打印该从父进程中接收进来从参数的内容；
   2. 休眠1秒后，向chdbuf写入含有该进程号和线程号信息的句子，并打印其设置的句子
   3. 休眠1秒后，打印子进程正在休眠并再休眠1s

5. 同理，主程序调用`chdtid2 = clone(child_func2, stack2 + STACK_SIZE, flags | SIGCHLD, buf);`语句来创建第二个子进程，子进程执行函数child_func2,并传入参数buf，该新进程的系统堆栈的栈顶地址为`stac21 + STACK_SIZE`，`flags`和`SIGCHLD`的解释步骤4。

   第二个子进程执行的child_func2函数如下：

   ```c
   static int child_func2(void *arg)
   {
       char *chdbuf = (char*)arg; /* type casting */
       printf("child_func2 read buf: %s\n", chdbuf);
       sleep(1);
       sprintf(chdbuf, "I am child_func2, my tid = %ld, pid = %d", gettid(), getpid());
       printf("child_func2 set buf: %s\n", chdbuf);
       sleep(1);
       printf("child_func2 sleeping and then exists ...\n");
       sleep(1);
   
       return 0;
   }
   ```

   在child_func2函数中：

   1. 将读入的参数转换成字符串类型赋值给chdbuf，并打印该从父进程中接收进来从参数的内容；
   2. 休眠1秒后，向chdbuf写入含有该进程号和线程号信息的句子，并打印其设置的句子
   3. 休眠1秒后，打印子进程正在休眠并再休眠1s

6. 主函数调用`waitpid(-1, &status, 0) == -1`等待子进程结束

7. 主函数读出变量buf中的值，并且调用ps指令显示当前进程的状态，回收栈资源。

#### 1.6.3 程序运行结果及分析：

1. 执行./a.out不对flag进行设置，执行结果如下：

   ![image-20210511134845552](http://hurq5.gitee.io/os-pictures/image-20210511134845552.png)

   分析：当子进程修改buf的内容时后，主进程的buf的内容并不会发生改变，可见每个进程使用自己独立的内存空间

   ![image-20210511142518318](http://hurq5.gitee.io/os-pictures/image-20210511142518318.png)

   分析：主进程和子进程异步执行。

2. 执行./a.out vm使程序执行`flags |= CLONE_VM`语句，设置子进程与父进程运行于相同的内存空间，执行结果如下：

   ![image-20210511143103055](http://hurq5.gitee.io/os-pictures/image-20210511143103055.png)

   分析：当第一个子进程最后一次修改buf的内容时后，主进程的buf的内容为第一个子进程最后修改buf的内容，可见主进程和子进程共享内存空间。

   ![image-20210511143130160](http://hurq5.gitee.io/os-pictures/image-20210511143130160.png)

   分析：主进程和子进程异步执行。

3. 执行./a.out vm vfork使程序执行`flags |= CLONE_VM`语句，设置子进程与父进程运行于相同的内存空间，执行`flags |= CLONE_VFORK`语句，设置父进程被挂起，直至子进程释放虚拟内存资源，执行结果如下：

   ![image-20210511143356365](http://hurq5.gitee.io/os-pictures/image-20210511143356365.png)

   分析：当父进程使用clone创建子进程并执行子进程时，父进程被挂起，直至子进程执行结束，释放虚拟内存资源后，父进程才恢复执行。

### 1.7 Algorithms 14-7

#### 1.7.1 程序的主要实现细节：

1. 主函数先为即将创建的子进程申请系统堆栈：

2. 主函数调用`chdtid = clone(test, stack + STACK_SIZE, flags | SIGCHLD, buf);`来创建子进程，子进程执行函数test, 并传入参数buf，该新进程的系统堆栈的栈顶地址为`stack + STACK_SIZE`，`flags`就是标志需要从父进程继承的资源,此处设置为0，设置`SIGCHLD`是为了使得子进程在结束时候可以通知父进程去回收它的资源，与后面`waitpid`函数形成照应。

   子进程执行的test函数如下：

   ```c
   static int test(void *arg)
   { 
       static int i = 0;
       char buffer[1024]; 
       if(i == 0) {
           printf("test: my ptd = %d, tid = %ld, ppid = %d\n", getpid(), gettid(), getppid());
           printf("\niteration = %8d", i); 
       }
       printf("\b\b\b\b\b\b\b\b%8d", i); 
       i++; 
       test(arg); /* recursive calling */
   } 
   ```

   在test中，函数定义1024个字节的buffer空间并递归调用自己。递归调用的函数申请的空间压入系统为进程申请的栈中。

3. 程序调用`ret = waitpid(-1, &status, 0);`等待子进程执行结束并回收相关的资源；

4. 主进程休眠2秒后，打印自己的进程号和子进程返回的结果。

#### 1.7.2 程序运行结果及分析：

![](http://hurq5.gitee.io/os-pictures/image-20210511144732128.png)

分析：

栈空间大小=(524288-10000)\*4096 =2,106,523,648

设置buffer变量的空间为1936125\*1024 =1,982,592,000

系统调用函数花费的总空间为(524288-10000)\*4096-1936125\*1024 =123,931,648

每次系统调用使用掉的空间为123,931,648/1936125=64.01014810510685（约等于64个字节）

## 2. 比较 pthread 和 clone() 线程实现机制的异同

### 2.1 不同点

* Pthread（）线程实现是基于用户级线程来实现，对核心是不可见的，由线程库调度；

  而clone（）是基于轻量级进程（LWP）来实现的，对核心是可见的

* pthread（）线程实现是通过线程库实现的，无需内核参与下创建、释放和管理，pthread（）线程实现的线程不需要内核模式就可以实现线程的切换,；

  而通过clone（）创建出来的LWP消耗内核栈空间，并且系统调度时需要在内核线程和用户线程之间切换。

* pthread_create()创建出来的线程阻塞，整个进程也随之阻塞。进程调度不能由内核调度，而是需要自己实现调度算法，好处是可以选择多种算法；

  而clone（）由核心所调度，创建出来的LWP在系统调用中出现了阻塞，是不会影响整个进程的执行的

### 2.2 相同点

* 对于clone（）函数，没有设置flags设置，则类似于使用fork（）进行进程创建；当函数调用的flags设置了诸多标志后，则类似于pthread_create()。

* pthread_create()相当于clone（）的调用加上标志的设置，并开辟了寄存器空间，栈空间，以及私有存储空间。

## 3. 对clone() 的 flags 采用不同的配置，设计测试程序讨论其结果 (配置包括 COLNE_PARENT, CLONE_VM, CLONE_VFORK, CLONE_FILES, CLONE_SIGHAND, CLONE_NEWIPC, CLONE_THREAD)

### 3.1 测试配置CLONE_VM：alg.14-8-clone-vm.c

#### 3.1.1相关知识点：

`clone() `的` flags`设置`CLONE_VM`参数表示clone出来的子进程和父进程之间共用同样的地址空间，设定`CLONE_VM` ，则调用进程和子进程会运行在同一个内存空间中，否则子进程拷贝调用进程的内存空间，并在其中运行。

#### 3.1.2程序的主要实现细节：

1. 主函数先为即将创建的子进程申请系统堆栈：

   ```c
       //为子进程申请系统堆栈
       char *stack = malloc(STACK_SIZE*sizeof(char)); /* allocating from heap, safer than stack[STACK_SIZE] */
       if(!stack) {
           perror("malloc()");
           exit(1);
       }
   ```

2. 设置clone的`flag`位：

   在程序运行的开始，程序提示输入`./a.out `或者` ./a.out vm`

   ```c
   	printf("./a.out or ./a.out vm\n");
   ```

   按照一下代码，当执行命令输入`./a.out vm`时，满足以下if语句的条件，因此`flags`配置有`CLONE_VM`属性，子进程与父进程运行于相同的内存空间。相反当执行命令输入`./a.out`时，不满足以下if语句的条件，因此flags未配置有`CLONE_VM`属性。

   ```c
       unsigned long flags = 0;
         /* set CLONE flags */
       if((argc > 1) && (!strcmp(argv[1], "vm"))) {
           flags |= CLONE_VM;//子进程与父进程运行于相同的内存空间
       }
   ```

3. 主函数在内存空间中定义buffer字符串，将字符串设置为`"I am parent, my pid = %d", getpid()`,打印自己的进程号`pid`并打印提示自己要开始使用clone创建子进程

   ```c
       sprintf(buf,"I am parent, my pid = %d", getpid());
       printf("parent set buf: %s\n", buf);
       sleep(1);
       printf("parent clone ...\n");
   ```

4. 主函数调用`chdtid1 = clone(child_func, stack + STACK_SIZE, flags | SIGCHLD, buf); /* what happened if without SIGCHLD */`来创建子进程，子进程执行函数child_func,并传入参数buf，该新进程的系统堆栈的栈顶地址为`stack + STACK_SIZE`，`flags`就是标志需要从父进程继承的资源,设置`SIGCHLD`是为了使得子进程在结束时候可以通知父进程去回收它的资源，与后面`waitpid`函数形成照应。

   子进程执行的child_func函数如下：

   ```c
   static int child_func(void *arg)
   {
       char *chdbuf = (char*)arg; /* type casting */
       printf("child_func read buf: %s\n", chdbuf);
       sprintf(chdbuf, "I am child_func, my tid = %ld, pid = %d", gettid(), getpid());
       printf("child_func set buf: %s\n", chdbuf);
       sleep(1);
       return 0;
   }
   ```

   在`child_func`函数中：

   1. 将读入的参数转换成字符串类型赋值给`chdbuf`，并打印该从父进程中接收进来从参数的内容；
   2. 休眠1秒后，向`chdbuf`写入含有该进程号和线程号信息的句子，并打印其设置的句子
   3. 休眠1秒

5. 主函数打印`"parent waiting ... \n"`，调用`waitpid(-1, &status, 0) == -1`等待子进程结束

6. 主函数读出变量buf中的值，并且回收栈资源。

#### 3.1.3 运行结果及分析：

* 执行语句`./a.out`，`clone()` 的 `flags `不配置`CLONE_VM`,执行结果如下：

  ![image-20210512224116774](http://hurq5.gitee.io/os-pictures/image-20210512224116774.png)

  分析：在主线程设置好buf的内容后，调用子进程，子进程修改buf的内容为新的内容，但是在子进程执行结束后，主进程的buf的内容并不会发生改变，可见父进程和子进程没有共享内存空间，每个进程使用自己独立的内存空间

* 执行语句`./a.out vm`，`clone()` 的 `flags `配置`CLONE_VM`,执行结果如下：

  ![image-20210513001508784](http://hurq5.gitee.io/os-pictures/image-20210513001508784.png)

  分析：在主线程设置好buf的内容后，调用子进程，子进程修改buf的内容为新的内容，但是在子进程执行结束后，主进程的buf的内容变为子进程修改的内容，可见父进程和子进程共享内存空间。

#### 3.1.4 完整代码：

见 Lab Week12 Program.1

### 3.2 测试配置CLONE_VFORK：alg.14-9-clone-vfork.c

#### 3.2.1相关知识点：

 对clone() 的 flags 配置CLONE_VFORK则执行clone出来的子进程的过程中，父进程会被挂起暂时不执行，知道子进程执行结束，父进程才恢复执行。如果没有设置CLONE_VFORK，那么父进程和子进程会异步执行，在子进程执行的过程中，父进程不挂起任然执行。

#### 3.2.2程序细节实现说明：

1. 主函数先为即将创建的子进程申请系统堆栈：

   ```c
       //为子进程申请系统堆栈
       char *stack = malloc(STACK_SIZE*sizeof(char)); /* allocating from heap, safer than stack[STACK_SIZE] */
       if(!stack) {
           perror("malloc()");
           exit(1);
       }
   ```

2. 设置clone的`flag`位：

   在程序运行的开始，程序提示输入`./a.out `或者` ./a.out vfork`

   ```c
   	printf("./a.out or ./a.out vfork\n");
   ```

   按照一下代码，当执行命令输入`./a.out vfork`时，满足以下if语句的条件，因此`flags`配置有`CLONE_VFORK`属性，子进程执行过程中父进程挂起。相反当执行命令输入`./a.out`时，不满足以下if语句的条件，因此flags未配置有`CLONE_VFORK`属性。

   ```c
       unsigned long flags = 0;
       /* set CLONE flags */
       if((argc > 1) && (!strcmp(argv[1], "vfork"))) {
           flags |= CLONE_VFORK;
       }
   ```

3. 主函数打印自己的进程号`pid`并打印提示自己要开始使用clone创建子进程

   ```c
      printf("I am parent, my pid = %d\n", getpid());
       printf("parent clone ...\n");
   ```

4. 主函数调用`chdtid = clone(child_func, stack + STACK_SIZE, flags | SIGCHLD, NULL);`来创建子进程，子进程执行函数child_func，该新进程的系统堆栈的栈顶地址为`stack + STACK_SIZE`，`flags`就是标志需要从父进程继承的资源,设置`SIGCHLD`是为了使得子进程在结束时候可以通知父进程去回收它的资源，与后面`waitpid`函数形成照应。

   子进程执行的child_func函数如下：

   ```c
   static int child_func()
   {
       printf( "I am child_func, my tid = %ld, pid = %d\n", gettid(), getpid());
       printf("child_func sleeping and then exists ...\n");
       sleep(1);
       return 0;
   }
   ```

   在`child_func`函数中：

   1. 打印进程调用函数的进程号pid和线程号tid；
   2. 调用waitpid(-1, &status, 0) == -1等待子进程结束

5. 主函数打印`"parent waiting ... \n"`，调用`waitpid(-1, &status, 0) == -1`等待子进程结束

6. 主程序打印`"parent waiting ... \n"`表示等待回收栈资源，并在子进程结束后回收栈资源。

#### 3.2.3 运行结果及分析：

* 执行语句`./a.out`，`clone()` 的 `flags `不配置`CLONE_VFORK`,执行结果如下：

  ![image-20210513003546240](http://hurq5.gitee.io/os-pictures/image-20210513003546240.png)

  分析：在子进程还在执行的过程中，主进程已经打印`"parent waiting ... \n"`表示等待回收栈资源，说明子进程的执行并不影响主进程的继续执行，即主进程没有挂起，主进程和子进程异步执行。

* 执行语句`./a.out vfork`，`clone()` 的 `flags `配置`CLONE_VFORK`,执行结果如下：

  ![image-20210513003831599](http://hurq5.gitee.io/os-pictures/image-20210513003831599.png)

  分析：在子进程执行结束后，主进程才打印`"parent waiting ... \n"`表示等待回收栈资源，说明子进程的执行的过程中主进程挂起，主进程等到子进程执行结束才恢复执行。

#### 3.2.4 完整代码：

见 Lab Week12 Program.2

### 3.3 测试配置CLONE_FILES：alg.14-10-clone-files.c

#### 3.3.1相关知识点：

对`clone()` 的` flags `配置`CLONE_FILES`则调用进程和子进程共享相同的文件描述符（file descriptor）表，调用进程或子进程创建的文件描述符同样对对方有效。

#### 3.3.2程序细节实现说明：

1. 主函数先为即将创建的子进程申请系统堆栈：

   ```c
       //为子进程申请系统堆栈
       char *stack = malloc(STACK_SIZE*sizeof(char)); /* allocating from heap, safer than stack[STACK_SIZE] */
       if(!stack) {
           perror("malloc()");
           exit(1);
       }
   ```

2. 设置clone的`flag`位：

   在程序运行的开始，程序提示输入`./a.out `或者` ./a.out files`

   ```c
   	printf("./a.out or ./a.out files\n");
   ```

   按照一下代码，当执行命令输入`./a.out files`时，满足以下if语句的条件，因此`flags`配置有`CLONE_FILES`属性，子进程执行过程中父进程挂起。相反当执行命令输入`./a.out`时，不满足以下if语句的条件，因此flags未配置有`CLONE_FILES`属性。

   ```c
       unsigned long flags = 0;
       /* set CLONE flags */
       if((argc > 1) && (!strcmp(argv[1], "files"))) {
           flags |= CLONE_FILES;
       }
   ```

3. 主函数打印`"parent clone ...\n"`，提示自己要开始使用clone创建子进程

   ```c
       printf("parent clone ...\n");
   ```

4. 主函数调用`chdtid = clone(child_func, stack + STACK_SIZE, flags | SIGCHLD, NULL);`来创建子进程，子进程执行函数child_func，该新进程的系统堆栈的栈顶地址为`stack + STACK_SIZE`，`flags`就是标志需要从父进程继承的资源,设置`SIGCHLD`是为了使得子进程在结束时候可以通知父进程去回收它的资源，与后面`waitpid`函数形成照应。

   子进程执行的child_func函数如下：

   ```c
   static int child_func()
   {
   	sleep(1);
       printf("In child_func:pid=%d\n", getpid());
   	printf("The files opened by child_process are as follows:\n");
   	char buffer[50];
   	sprintf(buffer,"lsof -p %d",getpid());
   	system(buffer);
   	sleep(1);
       return 0;
   }
   ```

   在`child_func`函数中：

   1. 休眠1s，等待调用程序打开或者创建f.txt文件；

   1. 打印进程调用函数的进程号pid；
   2. 系统调用`lsof -p +进程号`，打印当前进程中打开的文件表；
   3. 休眠1s

5. 主程序打开或者创建f.txt文件（存在则打开，不存在则创建并打开），打印信息，提示在主程序中打开了文件以及表明文件的标识符fd值。

   ```c
   	int fd = open("f.txt", O_WRONLY);
   	if (fd == -1) {
   		creat("f.txt",0666);
   		fd = open("f.txt",O_WRONLY);
   	}
       printf("In parent, open f.txt:fd = %d\n", fd);
   ```

6. 主程序打印`"parent waiting ... \n"`表示等待回收栈资源，并在子进程结束后回收栈资源，并关闭文件。

#### 3.3.3 运行结果及分析：

* 执行语句`./a.out`，`clone()` 的 `flags `不配置`CLONE_FILES`,执行结果如下：

  ![image-20210513005040052](http://hurq5.gitee.io/os-pictures/image-20210513005040052.png)

  分析：在调用进程中打开文件f.txt后，在子进程打印出来的，当前进程中打开的文件表中没有观察到f.txt，说明子进程下f.txt没有被打开，即调用进程和子进程不共享相同的文件描述符表。

* 执行语句`./a.out`，`clone()` 的 `flags `不配置`CLONE_FILES`,执行结果如下：

  ![image-20210513005801100](http://hurq5.gitee.io/os-pictures/image-20210513005801100.png)

  分析：在调用进程中打开文件f.txt后，观察到在子进程打印出来的，当前进程中打开的文件表中存在f.txt，说明在调用进程打开文件f.txt后，子进程下f.txt被打开了，即调用进程和子进程共享相同的文件描述符表。

#### 3.3.4 完整代码：

见 Lab Week12 Program.3

### 3.4 测试配置COLNE_PARENT：alg.14-11-clone-parent.c

#### 3.4.1相关知识点：

对`clone()` 的` flags `配置`COLNE_PARENT`则调用进程和子进程的父进程相同。

#### 3.4.2程序细节实现说明：

1. 同上面的程序，主函数先为即将创建的子进程申请系统堆栈

2. 同上面的程序，设置clone的`flag`位，当执行命令输入`./a.out parent`时，`flags`配置`COLNE_PARENT`属性, 相反当执行命令输入`./a.out`时，不满足以下if语句的条件，因此flags未配置有`COLNE_PARENT`属性。

3. 调用进程打印自己的父进程号

   ```
   printf("I am parent, my parent PID = %ld\n", (long)getppid());
   ```

4. 主函数打印`"parent clone ...\n"`，提示自己要开始使用clone创建子进程

5. 同上面的程序，主函数无传参创建子进程，

   子进程执行的child_func函数如下：

   ```
   static int child_func()
   {
       printf("I am child_func, my parent PID = %ld\n", (long)getppid());
       sleep(1);
       return 0;
   }
   ```

   进程调用函数打印自己的父程序号；

6. 主程序打印`"parent waiting ... \n"`表示等待回收栈资源，并在子进程结束后回收栈资源。

#### 3.4.3 运行结果及分析：

* 执行语句`./a.out`，`clone()` 的 `flags `不配置`COLNE_PARENT`,执行结果如下：

  ![image-20210513011908312](http://hurq5.gitee.io/os-pictures/image-20210513011908312.png)

  分析：调用进程和子进程的父进程不同。

* 执行语句`./a.out parent`，`clone()` 的 `flags `配置`COLNE_PARENT`,执行结果如下：

  <img src="http://hurq5.gitee.io/os-pictures/image-20210513012020117.png" alt="image-20210513012020117" style="zoom:150%;" />

  分析：调用进程和子进程的父进程相同。

#### 3.4.4 完整代码：

见 Lab Week12 Program.4

### 3.5 测试配置CLONE_SIGHAND：alg.14-12-clone-sighand.c

#### 3.5.1相关知识点：

对`clone()` 的` flags `配置`CLONE_SIGHAND`则子进程与父进程共享相同的信号句柄表.

此时，如果调用进程或子进程修改了某个信号的行为，那么这个修改也会影响到另一个进程

相反没有设置`CLONE_SIGHAND`，则子进程只是拷贝调用进程执行clone期间的一份信号句柄，调用进程和子进程对信号调用函数的修改不影响到彼此。

从linux 2.6.0开始，当指定`CLONE_SIGHAND`后，必须也指定`CLONE_VM` 。

#### 3.5.2程序细节实现说明：

1. 主函数打印自己的进程号，表明此时运行在主函数中；

2. 同上面的程序，设置clone的`flag`位，当执行命令输入`./a.out sighand`时，`flags`配置`CLONE_SIGHAND`属性, 相反当执行命令输入`./a.out`时，不满足以下if语句的条件，因此flags未配置有`CLONE_SIGHAND`属性。

3. 主进程设置SIGINT信号的信号调用函数为int_parent_handler，并打印相关信息表示已经设置。

   ```c
   	printf("Set SIGINT to trigger the execution of int_parent_handler()\n");
   	signal( SIGINT, int_parent_handler);
   ```

   int_parent_handler函数的内容如下：

   ```c
   void int_parent_handler(){
        printf("int_parent_handler:Get a SIGINT signal!\n"); 
   }
   ```

   程序打印信息，表面该`SIGINT`信号被`int_parent_handler`函数所处理。

4. 主程序提示用户通过键盘输入`'ctrl+c'`来产生SIGINT信号触发其信号处理函数，或者通过键盘输入`'ctrl+\'`来退出程序，程序调用`pause()`阻塞等待用户的指定输入，测试此时`SIGINT`信号的信号处理函数是什么。

   ```c
   	printf("input 'ctrl+c' to call handler\n");
   	printf("input 'ctrl+\\' to quit\n");
   	pause(); 
   ```

5. 主函数打印`"parent clone ...\n"`，提示自己要开始使用clone创建子进程

6. 同上面的程序，主函数无传参创建子进程，

   子进程执行的`child_func`函数如下：

   ```c
   static int child_func() {
   	printf( "I am child, my tid = %ld, pid = %d\n", gettid(), getpid());
   	printf("Set SIGINT to trigger the execution of int_child_handler()\n");
   	signal( SIGINT, int_child_handler);
   	return 0;
   }
   ```

   在 `child_func`()函数中：

   1. 函数打印打印当前执行子进程的进程号和线程号；

   2. 子程设置`SIGINT`信号的信号调用函数为`int_child_handler`，并打印相关信息表示已经设置。

   3. `int_child_handler`如下：

      ```c
      void int_child_handler(){
           printf("int_child_handler:Get a SIGINT signal!\n"); 
      }
      ```

      程序打印信息，表面该`SIGINT`信号被`int_child_handler`函数所处理。

7. 主程序打印`"parent waiting ... \n"`表示等待回收栈资源，并在子进程结束，主程序提示用户通过键盘输入'ctrl+c'来产生SIGINT信号触发其信号处理函数，或者通过键盘输入`'ctrl+\'`来退出程序，程序调用`pause()`阻塞等待用户的指定输入，测试此时`SIGINT`信号的信号处理函数是什么。

#### 3.5.3 运行结果及分析：

* 执行语句`./a.out`，`clone()` 的 `flags `不配置`CLONE_SIGHAND`,执行结果如下：

  ![image-20210513012321472](http://hurq5.gitee.io/os-pictures/image-20210513012321472.png)

  分析：在主进程设置`SIGINT`信号的信号处理函数为`int_parent_handler`后，测试`SIGINT`信号的信号处理函数，该处理函数为`int_parent_handle`

  在子进程执行完毕，成功将`SIGINT`信号的信号处理函数设置为`int_child_handler`后，再次在调用进程中测试`SIGINT`信号的信号处理函数，该处理函数仍为`int_parent_handle`。

  说明此时调用进程和子进程之间未共享相同的信号句柄表.

* 执行语句`./a.out sighand` ，`clone()` 的 `flags `配置`CLONE_SIGHAND`,执行结果如下：

  ![image-20210513012357280](http://hurq5.gitee.io/os-pictures/image-20210513012357280.png)
  
  分析：在主进程设置`SIGINT`信号的信号处理函数为`int_parent_handler`后，测试`SIGINT`信号的信号处理函数，该处理函数为`int_parent_handle`
  
  在子进程执行完毕，成功将`SIGINT`信号的信号处理函数设置为`int_child_handler`后，再次在调用进程中测试`SIGINT`信号的信号处理函数，该处理函数为`int_child_handler`，是子进程设置的结果。
  
  说明此时调用进程和子进程之间未共享相同的信号句柄表.

#### 3.5.4 完整代码：

见 Lab Week12 Program.5

### 3.6测试配置 CLONE_THREAD： alg.14-13-clone-thread.c

#### 3.6.1相关知识点：

对`clone()` 的` flags `配置`CLONE_THREAD`则子进程与父进程共享相同的线程群.子线程会放到与调用进程相同的线程组中。

所谓线程组是Linux 2.4中添加的一项功能，用于支持一组POSIX线程共享一个PID。因此我们只需要通过调用getpid函数得到进程或线程的pid值就可以知道他们是否位于同一个线程组

组中的线程可以通过调用gettid获得其唯一线程标识符TID进行区分。

#### 3.6.2程序细节实现说明：

1. 同上面的程序，主函数先为即将创建的子进程申请系统堆栈

2. 同上面的程序，设置clone的`flag`位，当执行命令输入`./a.out thread`时，`flags`配置`CLONE_THREAD`属性, 相反当执行命令输入`./a.out`时，不满足以下if语句的条件，因此flags未配置有`CLONE_THREAD`属性。

3. 调用进程打印自己的父进程号，进程号和线程号

   ```
   printf("I am parent, my parent PID = %ld,PID = %d,tid=%ld\n",(long)getppid(),getpid(),gettid());
   ```

4. 主函数打印`"parent clone ...\n"`，提示自己要开始使用clone创建子进程

5. 同上面的程序，主函数无传参创建子进程，

   子进程执行的child_func函数如下：

   ```
   static int child_func()
   {
   	printf("I am child_func, my parent PID = %ld,PID = %d,tid=%ld\n",(long)getppid(),getpid(),gettid());
       sleep(1);
       return 0;
   }
   ```

   进程调用函数打印自己的父进程号，进程号和线程号；

6. 主程序打印`"parent waiting ... \n"`表示等待回收栈资源，并在子进程结束后回收栈资源。

#### 3.6.3 运行结果及分析：

* 执行语句`./a.out`，`clone()` 的 `flags `不配置`CLONE_THREAD`,执行结果如下：

  ![image-20210513012607328](http://hurq5.gitee.io/os-pictures/image-20210513012607328.png)

  分析：可以观察到调用进程和子进程的pid值不同，因此不位于相同的线程组。

* 执行语句`./a.out thread`，`clone()` 的 `flags `配置`CLONE_THREAD`,执行结果如下：

  ![image-20210513012651768](http://hurq5.gitee.io/os-pictures/image-20210513012651768.png)
  
  分析：可以观察到调用进程和子进程的pid值相同，因此位于相同的线程组。

#### 3.6.4 完整代码：

见 Lab Week12 Program.6

### 3.7测试配置 CLONE_NEWIPC： alg.14-14-clone-newipc.c

#### 3.7.1相关知识点：

* 对`clone()` 的` flags `配置`CLONE_NEWIPC`则clone出来的子进程会在新的IPC命名空间中创建，相反没有设置该标志，则子进程会和调用进程使用相同的IPC命名空间。

* 使用`ipcs`可以查看该命名空间下的ipc信息。使用`ipcs -q`打印出使用消息队列进行进程间通信的信息。

#### 3.7.2程序细节实现说明：

1. 同上面的程序，主函数先为即将创建的子进程申请系统堆栈

2. 同上面的程序，设置clone的`flag`位，当执行命令输入`./a.out newipc`时，`flags`配置`CLONE_NEWIPC`属性, 相反当执行命令输入`./a.out`时，不满足以下if语句的条件，因此flags未配置有`CLONE_NEWIPC`属性。

3. 调用进程打印自己的父进程号，进程号和线程号

   ```
   printf("I am parent, my parent PID = %ld,PID = %d,tid=%ld\n",(long)getppid(),getpid(),gettid());
   ```

4. 主函数打印`"parent clone ...\n"`，提示自己要开始使用clone创建子进程

5. 同上面的程序，主函数无传参创建子进程，

   子进程执行的child_func函数如下：

   ```c
   static int child_func(void *arg)
   {
       sleep(1);
       printf("I am child_func, my tid = %ld, pid = %d\n", gettid(), getpid());
       //使用ipcs -q打印出使用消息队列进行进程间通信的信息
       system("ipcs -q");
       sleep(1);
       return 0;
   }
   ```

   在child_func函数中

   1. 休眠1秒，给异步执行的调用进程时间创建一个消息队列
   2. 打印该子进程的线程号和进程号
   3. 使用ipcs -q打印出使用消息队列进行进程间通信的信息，观察是否有调用进程创建出来的消息队列

6. 主程序在子进程执行的过程中创建一个消息队列在ipc空间中，并向消息队列中发送一条信息 （内容为"hello world!"），使用`ipcs -q`打印出使用消息队列进行进程间通信的信息。

   ```c
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
   ```

7. 主程序打印`"parent waiting ... \n"`表示等待回收栈资源，并在子进程结束后回收栈资源，调用`msgctl(msqid, IPC_RMID, 0)`删除队列。

#### 3.7.3 运行结果及分析：

* 执行语句`sudo ./a.out`，`clone()` 的 `flags `不配置`CLONE_NEWIPC`,执行结果如下：

  ![image-20210513012739576](http://hurq5.gitee.io/os-pictures/image-20210513012739576.png)

  分析：在子进程执行过程中，调用进程创建的队列出现在子进程的IPC命名空间中，说明两者共用相同的IPC命名空间，clone出来的子进程没有在新的IPC命名空间中创建。

* 执行语句`sudo ./a.out newipc`，`clone()` 的 `flags `配置`CLONE_NEWIPC`,执行结果如下：

  ![image-20210513012821812](http://hurq5.gitee.io/os-pictures/image-20210513012821812.png)
  
  分析：在子进程执行过程中，调用进程创建的队列没有出现在子进程的IPC命名空间中，说明两者使用不同的IPC命名空间，clone出来的子进程在新的IPC命名空间中创建。

#### 3.7.4 完整代码：

见 Lab Week12 Program.7