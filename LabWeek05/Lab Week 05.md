# Lab Week 05. 

## 实验内容：

进程的创建和终止。编译运行课件 Lecture 06 例程代码：Algorithms 6-1 ~ 6-6. 

## 实验报告 

 实验内容的原理性和实现细节解释，包括每个系统调用的作用 过程和结果。

## 提交内容

在 matrix 系统提交通过编译的源代码和 MD 格式的实验报告。 

## 提交时间

2021.03.25 上午 实验课时间

## Algorithm 6-1: fork-demo.c (forking a separate process)

* 知识点：
  1. 概念：fork（）函数通过系统调用创建一个与原来进程几乎完全相同的进程，也就是两个进程可以做完全相同的事，但如果初始参数或者传入的变量不同，两个进程也可以做不同的事。
  2. 在fork函数执行完毕后，如果创建新进程成功，则出现两个进程，一个是子进程，一个是父进程。在子进程中，fork函数返回0，在父进程中，fork返回新创建子进程的进程ID。我们可以通过fork返回的值来判断当前进程是子进程还是父进程。
  3. fork出错可能有两种原因：
       1）当前的进程数已经达到了系统规定的上限，这时errno的值被设置为EAGAIN。
       2）系统内存不足，这时errno的值被设置为ENOMEM。
  4. 每个进程都有一个独特（互不相同）的进程标识符（process ID），可以通过getpid（）函数获得

* 源代码（详细注释）：

  ```c
  int main(void)
  {
  int count = 1;
  pid_t childpid;
  childpid = fork(); /* child duplicates parent’s address space *///只是复制父进程的数据段，代码段
  if (childpid < 0) {//fork()返回0表示出错
  perror("fork error: ");
  return EXIT_FAILURE;
  }
  else /* fork() returns 2 values: 0 for child pro and childpid for parent pro */
  if (childpid == 0) { /* This is child pro */
  count++;
  printf(“Child pro pid = %d, count = %d (addr = %p)\n", getpid(), count,
  &count);
  }
  else { /* This is parent pro */
  printf("parent pro pid = %d, child pid = %d, count = %d (addr = %p)\n",
  getpid(), childpid, count, &count);
  sleep(5);//可以有效避开并发，休息5秒
  wait(0); /* waiting for all children terminated */
      //对应于fork(),0表示等待所有子进程，如果是特定pid就等待特定子进程
  }
  printf("Testing point by %d\n", getpid()); /* child executed this statement and
  became defunct before parent wait()
  因为父进程sleep（5）*/
  return EXIT_SUCCESS;
  }
  ```

  

* ![image-20210318101008746](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318101008746.png)

  分析：父亲进程和子进程共用同样的虚拟地址因为fork（）使得子进程直接复制父进程的整个栈结构，还有我们可以观察到父进程和子进程的pid不同，这是因为每个进程都有一个独特（互不相同）的进程标识符（process ID）

  ![image-20210318101625975](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318101625975.png)

  分析：父进程和子进程的count数值不同，因为它们有不同的映射表，被映射到不同的物理地址空间中。

  ![image-20210318101948492](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318101948492.png)

  分析：子进程和父进程都执行了最后的测试指令，子进程先结束，因为父进程sleep了5秒。

  

## Algorithm 6-2: vfork-demo.c (vforking a separate process).

* 知识点：vfork()与fork()的区别：

  1.  fork ()是子进程拷贝父进程的数据段、代码段，而vfork ()是子进程与父进程**共享数据段**
  2.  fork ()父子进程的执行次序是不确定，而vfork() 确保子进程先运行，在调用exec 或exit 之前与父进程数据是共享的,在它调用exec或exit 之后父进程才可能被调度运行。
  3.  vfork()保证子进程先运行是因为在调用这两个函数之前子进程依赖于父进程的进一步动作，则会导致死锁。
  4.  vfork()要和exec()搭配使用。(execv)


* 源代码（详细注释）:

  ```c
  int main(void)
  {
  int count = 1;
  pid_t childpid;
  childpid = vfork(); /* child shares parent’s address space *///共享资源，两个进程不能并发
  if (childpid < 0) {//返回值小于0，生成子进程失败
  perror("fork error: ");
  return EXIT_FAILURE;
  }
  else /* vfork() returns 2 values: 0 for child pro and childpid for parent pro */
  if (childpid == 0) { /* This is child pro, parent hung up until child exit */
  count++;
  printf(“Child pro pid = %d, count = %d (addr = %p)\n", getpid(), count,
  &count);
  printf(“Child taking a nap ...\n");
  sleep(10); printf(“Child waking up!\n");
  _exit(0); /* or exec(0); "return" will cause stack smashing */
       //不能用return，如果是return，栈帧就弹出来了，栈被破坏
  }
  else { /* This is parent pro, start when the vforked child terminated */
      //子进程在执行过程中，父进程被挂起，因此不能执行
  printf("parent pro pid = %d, child pid = %d, count = %d (addr = %p)\n",
  getpid(), childpid, count, &count);
  wait(0); /* not waitting this vforked child terminated */
      //但是还是习惯性的写wait（），可能有其他子进程
  }
  printf("Testing point by %d\n", getpid()); /* executed by parent pro only */
  return EXIT_SUCCESS;
  }
  ```

  

* ![image-20210318104442490](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318104442490.png)

  分析：父亲进程和子进程共用同样的虚拟地址，因为vfork()使得两个进程共用同样的资源

* ![image-20210318104919518](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318104919518.png)

  分析：count与程序一执行同样的指令，但是不同的是子进程和父进程的count的值是相同的，原因是vfork ()是子进程与父进程**共享数据段**，而fork()是拷贝数据段。

* ![image-20210318105404455](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318105404455.png)

  分析：根据图中的结果可以观察到，在子进程休息的这段时间里面，父进程是没有在执行的，直到子进程结束后，父进程才开始执行。

* ![image-20210318105619087](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318105619087.png)

  分析：子进程在父进程通过测试点之前就退出了程序。

## Algorithm 6-3: fork-demo-nowait.c (fork, execv without waiting).  

* 知识点：

  1. 僵尸进程：已经终止但其父级尚未调用wait（）的进程，所有进程终止时都将转换为该状态，但通常它们只是短暂地以僵尸形式存在，父级调用wait（）后，将释放僵尸进程的进程标识符及其在进程表中的条目
  2. 孤儿进程：父级不调用wait（）而是终止, 孤儿的子代不是孤儿
  3. Linux和UNIX通过将初始化进程作为新的父进程分配给孤立进程（采用孤立进程）来解决这种情况。   
     * init进程是UNIX和Linux系统中进程层次结构的根。   
     * 初始化进程会定期调用wait（），从而允许收集任何孤立进程的退出状态并释放该孤立进程的进程标识符和进程表条目（效率要高及时收回孤儿才不会照成系统效率的下降）
  4. wait()要与fork()配套出现,如果在使用fork()之前调用wait(),wait()的返回值则为-1,正常情况下wait()的返回值为子进程的PID,
  5. ps主要用来查看进程信息,加 -l指的是使用长格式

* 源代码：

  ```c
  int main(void)
  {
  int count = 1;
  pid_t childpid;
  childpid = fork(); /* child duplicates parent’s address space */
  if (childpid < 0) {//返回值小于0，生成子进程失败
  perror("fork error: ");
  return EXIT_FAILURE;
  }
  else
  if (childpid == 0) { /* This is child pro */
  count++;
  printf("child pro pid = %d, count = %d (addr = %p)\n", getpid(), count,
  &count);
  printf("child sleeping ...\n");
  sleep(10); /* parent exites during this period, child became an orphan */
      //父进程更快的执行结束且没有调用wait()函数来等待子进程的结束，子进程变成孤儿进程
  printf("\nchild waking up!\n");
  }
  else { /* This is parent pro */
  printf("parent pro pid = %d, child pid = %d, count = %d (addr = %p)\n",
  getpid(), childpid, count, &count);
  }
  printf("\nTesting point by %d\n", getpid()); /* executed by parent and child */
  return EXIT_SUCCESS;
  }
  ```

* ![image-20210318125043183](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318125043183.png)

  分析：使用ps查看进程信息发现，父进程结束（输出testing point）后，子进程还在执行，成为孤儿进程。

  

  ![image-20210318125611649](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318125611649.png)

  分析：孤儿进程资源没有被回收，依旧在执行。（终端异常）

  

## Algorithm 6-4: fork-demo-wait.c (fork and wait)  

* 知识点：无

* 源代码：

  ```c
  int main(void)
  {
  int count = 1;
  pid_t childpid, terminatedid;
  childpid = fork(); /* child duplicates parent’s address space */
  if (childpid < 0) {//返回值小于0，生成子进程失败
  perror("fork error: ");
  return EXIT_FAILURE;
  }
  else
  if (childpid == 0) { /* This is child pro */
  count++;
  printf("child pro pid = %d, count = %d (addr = %p)\n", getpid(), count,
  &count);
  printf("child sleeping ...\n");
  sleep(5); /* parent wait() during this period */
  printf("\nchild waking up!\n");
  }
  else { /* This is parent pro */
  terminatedid = wait(0);//父进程等待子进程结束，terminatedid是简单的标记位
  printf("parent pro pid = %d, terminated pid = %d, count = %d (addr =
  %p)\n", getpid(), terminatedid, count, &count);
  }
  printf("\nTesting point by %d\n", getpid()); /* executed by child and parent */
      //子进程先执行这条指令
  return EXIT_SUCCESS;
  }
  ```

  

* ![image-20210318125904315](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318125904315.png)

  分析：父进程在wait()处等待子进程结束（输出testing point）后。

  ![image-20210318130113257](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318130113257.png)

  分析：子进程先结束，父进程后结束，因为父进程的wait()会等待子进程执行结束

  

## Algorithm 6-5-0: sleeper.c (a demo process sleeping for 5 seconds).（一个进程）

* 知识点：atoi()函数使用

  1. 函数功能：把字符串转换成整型数.

  2. 原型:int atoi(const char *nptr);

  3. 函数说明: 参数nptr字符串，如果第一个非空格字符不存在或者不是数字也不是正负号则返回零，否则开始做类型转换，之后检测到非数字(包括结束符 \0) 字符时停止转换，返回整型数

* 源代码：

   sleeper.c

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  
  int main(int argc, char* argv[])
  {
      int secnd = 5;
  
      if (argc > 1) {
          secnd = atoi(argv[1]);//把字符串转换成整型数.该整型数为输入的数值
          if ( secnd <= 0 || secnd > 10)//如果休眠时间不合法，就令休眠时间为5秒
              secnd = 5;
      }
      
      printf("\nsleeper pid = %d, ppid = %d\nsleeper is taking a nap for %d seconds\n", getpid(), getppid(), secnd); /* ppid - its parent pro id */
  
      sleep(secnd);//sleep输入的指定秒数
      printf("\nsleeper wakes up and returns\n");
  
      return 0;
  }
  
  ```

* ![image-20210318154721264](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318154721264.png)

  分析：执行程序休眠了5秒

## Algorithm 6-5: vfork-execv-wait.c (vfork, execv and wait).

* 知识点：

  * execv

  > 1. 函数原型：**int execv(const char \****progname***, char \*const** *argv***[]);  //#include <**[**unistd.h**](https://blog.csdn.net/west_609/article/details/include/unistd.h)**>**
  >
  >    其中progname: 被执行的应用程序。
  >
  >    argv: 传递给应用程序的参数列表，注意，这个数组的第一个参数应该是应用程序名字本身，并且最后一个参数应该为NULL。
  >
  > 2. 功能：execv会停止执行当前的进程，并且以progname应用进程替换被停止执行的进程，进程ID没有改变。

  * stat

  > 1. 定义函数：int stat(const char * file_name, struct stat *buf);
  > 2. 函数说明：stat()用来将参数file_name 所指的文件状态, 复制到参数buf 所指的结构中。
  > 3. 执行成功则返回0，失败返回-1，错误代码存于errno。

* 源代码：

  ```c
  Algorithm 6-5: vfork-execv-wait.c (vfork, execv and wait) (1)
  int main(void)
  {
      pid_t childpid;
  
      childpid = vfork();
          /* child shares parent's address space */
      if (childpid < 0) {
          perror("fork()");
          return EXIT_FAILURE;
      }
      else
          if(childpid == 0) { /* This is child pro */
              printf("This is child, pid = %d, taking a nap for 2 sencods \n", getpid());
              sleep(2); /* parent hung up and do nothing */
  
              char filename[80];
              struct stat buf;
              strcpy(filename, "./alg.6-5-0-sleeper.o");//赋值filename调用文件的路径
              if(stat(filename, &buf) == -1) {
                  perror("\nsleeper stat()");
                  _exit(0);
              }//stat返回-1，说明文件路径存在问题或者文件存在问题使得文件的状态查看不了
              char *argv1[] = {filename, argv[1], NULL};//execv规定传入的第二个参数是一个参数列表，该参数列表的第一个参数是调用的函数名，最后一个参数应该为NULL
              printf("child waking up and again execv() a sleeper: %s %s\n\n", argv1[0], argv1[1]);
              execv(filename, argv); /* parent resume at the point 'execv' called */
              //使用execv，子进程跳转到另一个新的进程（sleeper），父进程与之异步执行
          }      
          else { /* This is parent pro, start when the vforked child terminated */
              printf("This is parent, pid = %d, childpid = %d \n", getpid(), childpid); /* parent executed this statement during the EXECV time */
              int retpid = wait(0); /* without wait(), the spawned EXECV may became an orphan */
              printf("\nwait() returns childpid = %d\n", retpid);
          }
         
      return EXIT_SUCCESS;
  }
  
  ```

* ![image-20210318163537829](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318163537829.png)

  分析：调用的sleep进程继承沿用的子进程的pid和ppid，sleep进程成为和父进程异步执行的子进程。

  ![image-20210318164006244](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318164006244.png)

  分析：父进程在子进程调用execv后恢复执行，同时子进程中止，sleeper继承了子进程的pid，作为父进程的子代产生，但具有重复的独立的地址空间，因此返回父级而没有任何堆栈砸坏。 父级和子级异步执行。

* ![image-20210318164302808](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210318164302808.png)

  分析：父进程调用wait(),等待sleeper运行结束，说明父进程将sleeper进程当作自己的子进程。因此我们以后执行vfork（）指令也一定要调用wait（）函数，以防止在子进程中调用的正常进程变成僵尸进程。

## **Algorithm 6-6**: **vfork-execv-nowait.c** **(**vfork, **execv** **without waiting)**

* 源代码：

  ```c
  int main(int argc, char* argv[])
  {
      pid_t childpid;
  
      childpid = vfork(); 
          /* child shares parent's address space */
      if (childpid < 0) {
          perror("fork()");
          return EXIT_FAILURE;
      }
      else
          if (childpid == 0) { /* This is child pro */
              printf("This is child, pid = %d, taking a nap for 2 seconds ... \n", getpid());
              sleep(2); /* parent hung up and do nothing */
  
              char filename[80];
              struct stat buf;
              strcpy(filename, "./alg.6-5-0-sleeper.o");//赋值filename调用文件的路径
              if(stat(filename, &buf) == -1) {
                  perror("\nsleeper stat()");
                  _exit(0);
              }//stat返回-1，说明文件路径存在问题或者文件存在问题使得文件的状态查看不了
              char *argv1[] = {filename, argv[1], NULL};
              printf("child waking up and again execv() a sleeper: %s %s\n\n", argv1[0], argv1[1]);
              execv(filename, argv); /* parent resume at the point 'execv' called */
          } 
      else { /* This is parent pro, start when the vforked child terminated */
              printf("This is parent, pid = %d, childpid = %d \n",getpid(), childpid); 
                /* parent executed this statement during the EXECV time */
          //在执行sleeper过程中
              printf("parent calling shell ps\n");
              system("ps -l");
          //系统调用ps -l查看当前的进程列表
              sleep(1);
              return EXIT_SUCCESS; 
                  /* parent exits without wait() and child may become an orphan */
      }
  }
  
  ```
  
  

* ![image-20210319005334589](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210319005334589.png)

  分析:bash进程是main函数进程的父进程

  ![image-20210319005747426](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210319005747426.png)

  分析：被进程调用的sleeper进程继承了子进程的pid值，同时也继承了他的父进程。

  ![image-20210319005958685](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210319005958685.png)

  分析：系统调用的父进程也为main函数的进程

  ![image-20210319010056061](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210319010056061.png)

  分析：ps（查看所有进程表的进程）的父进程为系统调用进程

  ![image-20210319010249112](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210319010249112.png)

  分析：main函数进程结束，子进程sleeper进程还在执行，该进程变成孤儿进程。

  ![image-20210319010417388](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210319010417388.png)	![image-20210319010438726](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210319010438726.png)

  观察上面两张图可以得到，在main函数进程结束前后sleeper进程的父进程发生了改变。可见sleeper进程在失去父进程之后，被pid为1126的进程收留作为子进程。

  ![image-20210319010657133](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210319010657133.png)

  分析：bash进程和sleeper进程是异步执行的

## **修改代码思考其他的情况：**

## Algorithm 6-7:vfork-noexit-noexecv.c（观察vfork直接返回会发生什么错误）

* 源代码：

  ```c
  #include <stdio.h> 
  #include <stdlib.h> 
  #include <sys/types.h> 
  #include <unistd.h> 
  void test() 
  { 
      pid_t childpid; 
      childpid=vfork(); /* child shares parent's address space */
      if(childpid<0)  //失败 
      { 
          perror("fork()");
          exit(0);
      } 
      else if(childpid==0) 
      { 
          printf("This is child,child pid=%d,ppid=%d\n",getpid(),getppid()); 
          return;  //return执行完后，把控制权交给调用函数，而exit()执行完后把控制权交给系统 
      } 
      else 
      { 
          printf("This is parent,pid=%d,ppid=%d\n",getpid(),getppid()); 
      } 
  } 
  void funtion() 
  { 
      int i=0; 
      int buf[100]; 
      for(;i<100;i++) 
      { 
          buf[i]=0; 
      } 
      printf("funtion:child pid=%d,ppid=%d\n",getpid(),getppid()); 
  } 
  int main() 
  { 
      test(); 
      funtion(); 
      return 0; 
  } 
  ```

  

* ![image-20210323154810892](http://hurq5.gitee.io/os-labwork/LabWeek05/pictures/image-20210323154810892.png)

  分析：程序在后续执行时出现了错误，并且可知道是在父进程中出现的错误
  vfork函数调用时，子进程比父进程先运行，在调用test()函数执行时，子进程执行完之后，将清理test函数的栈空间，然后子进程再调用fun()函数，将覆盖掉test的栈空间，继续执行fun函数。

  但是，当子进程退出后，执行父进程，但是，在test函数返回的时候该栈空间已经被子进程破坏了，不存在了，所以就出现了栈错误.