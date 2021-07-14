# Lab Week 14.

## 一. 实验要求：

把 Lecture08 示例 alg.8-1~8-3 拓展到多个读线程和多个写线程， 应用 Peterson 算法原理设计实现共享内存互斥。

## 二. 实验内容

### 1. 相关知识点：Peterson算法

* 简单介绍：

  `Peterson`算法是一种用于互斥的并发编程算法，该算法允许两个或多个进程共享一次使用的资源而不会发生冲突，仅使用共享内存进行通信。

* 算法实现：

  该算法使用两个变量`flag`和`turn`。一个`flag[n]`的值`true`表示进程n想进入临界区。如果`P1`不想进入它的关键部分，或者如果`P1`通过设置`turn`为`P0`赋予了`P0`优先权，则允许该进程`P0`进入关键部分0。

  ```
  bool flag[2] = {false, false};
  int turn;
  ```

  ```
  P0:      flag[0] = true;
  P0_gate: turn = 1;
    while (flag[1] == true && turn == 1)
    {
        // busy wait
    }
    // critical section
    ...
    // end of critical section
    flag[0] = false;
  ```

  ```
  P1:      flag[1] = true;
  P1_gate: turn = 0;
    while (flag[0] == true && turn == 0)
    {
        // busy wait
    }
    // critical section
    ...
    // end of critical section
    flag[1] = false;
  ```

  该算法满足解决关键部分问题的三个基本条件，只要变量`turn`为0，`flag`为true，`while`条件甚至可以抢占。

  该算法满足解决临界区问题的三个必须标准：进步和有限等待。

  由于`turn`可以取两个值之一，因此可以用一个位代替它，这意味着该算法仅需要三个位的内存。

  **互斥访问**
  `P0`与`P1`显然不会同时在临界区: 如果进程`P0`在临界区内，那么或者`flag[1]`为假(意味着`P1`已经离开了它的临界区)，或者`turn`为0(意味着`P1`只能在临界区外面等待，不能进入临界区).

  **空闲让进**
  Progress定义为：如果没有进程处于临界区内且有进程希望进入临界区, 则只有那些不处于剩余区的进程可以参与到哪个进程获得进入临界区这个决定中，且这个决定不能无限推迟。剩余区是指进程已经访问了临界区，并已经执行完成退出临界区的代码，即该进程当前的状态与临界区关系不大。

  **有限等待**
  有限等待(Bounded waiting)意味着一个进程在提出进入临界区请求后，只需要等待临界区被使用有上限的次数后，该进程就可以进入临界区,即进程不论其优先级多低，不应该饿死在该临界区入口处。`Peterson`算法显然让进程等待不超过1次的临界区使用，即可获得权限进入临界区。

  **扩展到N个线程互斥访问一个资源的filter算法**

  ```
  // initialization
  level[N] = { -1 };     // current level of processes 0...N-1
  waiting[N-1] = { -1 }; // the waiting process of each level 0...N-2
  
  // code for process #i
  for(i = 0; i < N-1; ++i) {
      level[i] = l;
      waiting[l] = i;
      while(waiting[l] == i &&
            (there exists k ≠ i, such that level[k] ≥ l)) {
          // busy wait
      }
  }
  
  // critical section
  
  level[i] = -1; // exit section
  ```

  数组`level`表示每个线程的等待级别，最小为0，最高为N-1，-1表示未设置。数组`waiting`模拟了一个阻塞（忙等待）的线程队列，从位置0为入队列，位置越大则入队列的时间越长。每个线程为了进入临界区，需要在队列的每个位置都经过一次，如果没有更高优先级的线程（考察数组`level`），cd 或者被后入队列的线程推着走（上述程序`waiting[l] ≠ i`），则当前线程在队列中向前走过一个位置。可见该算法满足互斥性。

  由`filter`算法去反思`Peterson`算法，可见其中的`flags`数组表示两个进程的等待级别，而`turn`变量则是阻塞（忙等待）的线程队列，这个队列只需要容纳一个元素。	

### 2. 程序关键代码以及思路：

* **程序实现多个写线程和读线程的共享内存互斥的思路：**通过`peterson`算法来实现写进程之间和读进程之间的互斥，通过共享结构体中的`written`变量实现读写进程之间的互斥。

* **在程序的主函数中：**

  程序的线程数默认为20，也可以通过命令行的第二个参数设置线程数量，输入的线程数必须合法（满足大于0小于程序限制的最大线程数`MAX_N`），后续程序根据输入值或者默认值创建`max_num`个读线程和`max_num`个写线程。

  ```c
      printf("Usage: ./a.out total_thread_num\n");
      if(argc > 1) {
          max_num = atoi(argv[1]);
      }
      if (max_num < 0 || max_num > MAX_N) {
          printf("invalid max_num\n");
          exit(1);
      }
  ```

  将全局变量`level_write`，`waiting_write`，`level_read`，`waiting_read`进行初始化，其中`level_write`表示写线程的等待级别，`level_read`表示读线程的等待级别，`waiting_write`表示阻塞（忙等待）的线程队列，`waiting_read`表示阻塞（忙等待）的线程队列。

  ```c
      memset(level_write, (-1), sizeof(level_write));
      memset(waiting_write, (-1), sizeof(waiting_write));
      memset(level_read, (-1), sizeof(level_read));
      memset(waiting_read, (-1), sizeof(waiting_read));
  ```

  依旧关键的一步，因为要给线程编号，为了避免直接在传递的参数中传递发生改变的量，导致结果和预期不同，线程未获取传递参数时，线程获取的变量值已经被主线程进行了修改，因此重新申请一块内存，存入需要传递的参数，再将这个地址作为参数传入线程。

  ```c
   for (i = 0; i < max_num; i++) {
          thread_num[i] = i;
      }
  ```

  程序将当前目录下的`myshm`文件作为共享文件对象，如果该目录下没有该文件对象就调用函数该名字的共享文件对象`creat(pathname, O_RDWR)`，调用``ftok(pathname, 0x27)·``指定系统建立共享内存时的ID值。

  ```c
      shmsize = TEXT_NUM*sizeof(struct shared_struct);
      strcpy(pathname, "myshm");
  
      if(stat(pathname, &fileattr) == -1) {
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
  ```

  程序调用```shmget((key_t)key, shmsize, 0666|PERM)```创建一个共享内存对象，返回共享存储的ID，进程对共享内存有读写的权限，```shmptr = shmat(shmid, 0, 0)```把共享内存区对象映射到调用进程的地址空间，允许本进程访问共享内存。`shmptr`强制转换为`(struct shared_struct *)`类型的指针并赋值给`shared`，`written`初始为0，因为一开始共享结构体中的`mtext`成员变量为空，处于可写状态。

  ```c
      shmid = shmget((key_t)key, shmsize, 0666|PERM);
      if(shmid == -1) {
          ERR_EXIT("shmcon: shmget()");
      }
  	printf("shmcon: shmid = %d\n", shmid);
      shmptr = shmat(shmid, 0, 0); /* returns the virtual base address mapping to the shared memory, *shmaddr=0 decided by kernel */
      shared = (struct shared_struct *)shmptr;
      shared->written = 0;
  ```

  程序分别使用for循环调用`max_num`次函数`pthread_create(&ptidWrite[i], NULL, &wirteFtn, (void *)&thread_num[i]);`创建指定线程数个写线程，使用for循环调用`max_num`次函数`ret = pthread_create(&ptidRead[i], NULL, &readFtn, (void *)&thread_num[i]);`创建指定线程数个读线程。

  **其中写线程调用的线程函数wirteFtn的关键代码解读如下：**

  写函数调用函数```shmget((key_t)key, TEXT_NUM*sizeof(struct shared_struct), 0666|PERM);```创建一个共享内存对象，，返回共享存储的ID值赋给变量shmid，调用函数```shmat(shmid, 0, 0)```把共享内存区对象映射到调用进程的地址空间，允许本进程访问共享内存，`shmptr`强制转换为`(struct shared_struct *)`类型的指针赋给`shared`，打印写进程开始工作，即等待或者直接进入临界区。

  ```c
      shmid = shmget((key_t)key, TEXT_NUM*sizeof(struct shared_struct), 0666|PERM);
      if (shmid == -1) {
          ERR_EXIT("shmwite: shmget()");
      }
      shmptr = shmat(shmid, 0, 0);
      if(shmptr == (void *)-1) {
          ERR_EXIT("shmwrite: shmat()");
      }   
      shared = (struct shared_struct *)shmptr;
      printf("write_thread-%d, ptid = %lu working\n", thread_num, pthread_self( ));
  
  ```

  写线程函数通过`peterson`算法来实现写线程之间的互斥

  数组`level_write`表示写线程的等待级别，最小为0，最高为max_num-1，-1表示未设置。数组`waiting_write`模拟了一个阻塞的线程队列，从位置0为入队列，`waiting_write`位置越大则入队列的时间越长。每个线程为了进入临界区，需要在队列的每个位置都经过一次，如果没有更高优先级的线程则当前线程在队列中向前走过一个位置。具有`level [j] <lev`的任何其他过程将其级别升级到或高于`lev`，则必须将线程thread_num退出线程队列并等待`waiting_write[lev]！= thread_num。`

  ```c
      for (lev = 0; lev < max_num-1; ++lev) { /* there are at least max_num-1 waiting rooms */
          level_write[thread_num] = lev;
          waiting_write[lev] = thread_num;
          while (waiting_write[lev] == thread_num) { /* busy waiting */
              /*  && (there exists k != thread_num, such that level[k] >= lev)) */
              for (k = 0; k < max_num; k++) {
                  if(level_write[k] >= lev && k != thread_num) {
                      break;
                  }
                  if(waiting_write[lev] != thread_num) { /* check again */
                      break;
                  }
              } /* if any other proces j with level[j] < lev upgrades its level to or greater than lev during this period, then process thread_num must be kicked out the waiting room and waiting[lev] != thread_num, and then exits the while loop when scheduled */
              if(k == max_num) { /* all other processes have level of less than process thread_num */
                  break;
              } 
          }
      } 
  ```

  当某一写线程有机会得到访问共享内存的机会时，还需要等待判断是否该共享空间是可写的

  ```
  while(shared->written == 1)
        		sleep(1);
  ```

  当写进程可访问共享空间并且该空间可写时，程序打印该线程（包含线程编号，和线程标识符）进入临界区，`counter++`表示进入临界区的线程个数+1，并将`"The message writed by write_thread-%d",thread_num`的信息写入到共享内存块中，设置共享结构体中的已写标志位written为1，即该共享结构体可读，等待读进程来访问该共享结构体。

  ```c
      printf("write_thread-%d, ptid = %lu entering the critical section\n", thread_num, pthread_self( ));
      counter++;
      sprintf(buffer,"The message writed by write_thread-%d",thread_num);
      strncpy(shared->mtext, buffer, TEXT_SIZE);
      printf("write_thread-%d write: %s\n",thread_num,shared->mtext);
      shared->written = 1;  /* message prepared */
  ```

  使用`counter`变量来判断进入临界区的线程个数，当`counter>1`的时候表明临界区有多个线程进入，这不符合互斥的原则，因此调用`kill(getpid(), SIGKILL);`杀死关闭进程。

  ```c
          printf("ERROR! more than one processes in their critical sections\n");
          kill(getpid(), SIGKILL);
      }
  
      counter--;  
        /* end of crictical section */
  ```

  函数执行`level_write[thread_num] = -1; `语句将该写线程的优先级降低到最低，允许其他写线程有条件退出等待循环进入临界区。

  **其中读线程调用的线程函数readFtn的关键代码解读如下：**

  和写线程程相同，读线程调用相同的相关函数，创建共享内存对象，返回共享存储的ID值赋给变量`shmid`，把共享内存区对象映射到调用进程的地址空间，允许本进程访问共享内存。

  同理，读线程函数也通过`peterson`算法来实现写线程之间的互斥，其中数组`level_read`表示读线程的等待级别，最小为0，最高为`max_num-1`，-1表示未设置。数组`waiting_read`模拟了一个阻塞的线程队列。

  ```c
      for (lev = 0; lev < max_num-1; ++lev) { /* there are at least max_num-1 waiting rooms */
          level_read[thread_num] = lev;
          waiting_read[lev] = thread_num;
  		//while(shared->written == 0)
  			//sleep(1);
          while (waiting_read[lev] == thread_num) { /* busy waiting */
              /*  && (there exists k != thread_num, such that level[k] >= lev)) */
              for (k = 0; k < max_num; k++) {
                  if(level_read[k] >= lev && k != thread_num) {
                      break;
                  }
                  if(waiting_read[lev] != thread_num) { /* check again */
                      break;
                  }
              }/* 如果在此期间，级别为[j] <lev的其他任何过程j将其级别升级到或大于lev，则必须将线程thread_num踢出等候室并等待[lev]！= thread_num，然后退出while循环 */
              if(k == max_num) { /* all other processes have level of less than process thread_num */
                  break;
              } 
          }
      }
  ```

  当某一读线程有机会得到访问共享内存的机会时，还需要等待判断是否该共享空间是已写可读的。

  ```c
      while(shared->written == 0)
        	sleep(1);
  ```

  当写进程可访问共享空间并且该空间可写时，程序打印该线程（包含线程编号，和线程标识符）进入临界区，`counter++`表示进入临界区的线程个数+1，读线程接收并打印共享结构体中的信息，的信息写入到共享内存块中，设置共享结构体中的已写标志位`written`为0，即该共享结构体中的信息已读出，可写入新的信息，等待写进程来访问该共享结构体。

  ```c
      counter++;
  	 printf("%*sread_thread-%d, ptid = %lu entering the critical section\n", 20, " ",thread_num, pthread_self( ));
  	printf("%*sThe read_thread-%d: read: %s\n", 20, " ",thread_num, shared->mtext);
      shared->written = 0;
       /* it is not reliable to use shared->written for process synchronization */
       
  ```

  和写进程相同，使用`counter`变量来判断进入临界区的线程个数，当`counter>1`的时候表明临界区有多个线程进入，这不符合互斥的原则，因此调用`kill(getpid(), SIGKILL);`杀死关闭进程，函数执行`level_read[thread_num] = -1`; 语句将该读线程的优先级降低到最低，允许其他读线程有条件退出等待循环进入临界区。

  **回到主函数：**

  主函数循环调用`pthread_join(ptidWrite[i], NULL);`和`ret = pthread_join(ptidRead[i], NULL);`函数，阻塞等待线程执行结束。然后调用`shmctl(shmid, IPC_RMID, 0)`删除共享空间。

### 3. 执行结果分析：

* **样例一**：

  命令输入./a.out 2来创建2个读线程和2个写线程来对共享结构体进行读写操作。执行结果如下：

  ![image-20210522165521063](https://hurq5.gitee.io/os-pictures/image-20210522165521063.png)

  分析：程序开始执行时，先打印`./a.out write_or_read_thread_num.`提醒用户输入命令的第二个参数将作为读线程的线程数，和写线程的线程数,注意因为读写线程分开创建，因此实际创建的进程数是两倍的输入参数大小，主函数创建共享内存（该共享内存的标记符为1441805），执行的结果表明，先是读线程1，读线程0，写线程1进入等待队列。

  此时共享空间处于可写不可读状态，因此等待队列中的唯一写线程1进入临界区，写线程1在共享空间中写下信息`“The message writed by write_thread-1”`,写线程0也进入等待队列；

  此时共享结构体处于可读不可写的状态，因此等待队列中优先级更高的读线程1进入临界区，读线程1在共享空间中读出信息`“The message writed by write_thread-1”`；

  此时共享空间处于可写不可读状态，因此等待队列中的唯一写线程0进入临界区，写线程0在共享空间中写下信息`“The message writed by write_thread-0”`；

  此时共享结构体处于可读不可写的状态，因此等待队列中唯一的读线程0进入临界区，读线程0在共享空间中读出信息`“The message writed by write_thread-0”`；

  读写线程交错进入临界区，读写线程之间存在互斥，同时在一个时间点不会用多个读线程或者写线程同时进入临界区，可以观察到程序很好的解决了线程互斥的问题。

  线程执行完毕，主函数删除共享内存（该共享内存的标记符为1441805），该共享内存为一开始我们创建的共享内存。

* **样例二**：

  命令输入./a.out 4来创建4个读线程和4个写线程来对共享结构体进行读写操作。执行结果如下：

  ![image-20210522195414832](https://hurq5.gitee.io/os-pictures/image-20210522195414832.png)

  ![image-20210522195435445](https://hurq5.gitee.io/os-pictures/image-20210522195435445.png)
  
  分析：程序开始执行时，先打印`./a.out write_or_read_thread_num.`提醒用户输入命令的第二个参数将作为读线程的线程数，和写线程的线程数，主函数创建共享内存（该共享内存的标记符为1474573），执行的结果表明，读写进程异步进入等待队列
  
  此时共享空间一开始处于可写不可读状态，因此等待队列中的唯一写线程3进入临界区，写线程1在共享空间中写下信息`“The message writed by write_thread-3”`；
  
  此时共享结构体处于可读不可写的状态，因此等待队列中优先级更高的读线程1进入临界区，读线程1在共享空间中读出信息`“The message writed by write_thread-3”`；
  
  此时共享空间处于可写不可读状态，因此等待队列中的唯一写线程1进入临界区，写线程1在共享空间中写下信息`“The message writed by write_thread-1”`；
  
  此时共享结构体处于可读不可写的状态，因此等待队列中唯一的读线程0进入临界区，读线程0在共享空间中读出信息`“The message writed by write_thread-1”`；
  
  。。。。。。。。
  
  读写线程交错进入临界区，读写线程之间存在互斥，同时在一个时间点不会用多个读线程或者写线程同时进入临界区，可以观察到程序很好的解决了线程互斥的问题。
  
  线程执行完毕，主函数删除共享内存（该共享内存的标记符为1474573），该共享内存为一开始我们创建的共享内存。
  
* **样例三**：

  命令输入./a.out来创建默认的20个读线程和20个写线程来对共享结构体进行读写操作。执行结果如下：

  ![image-20210522200019915](https://hurq5.gitee.io/os-pictures/image-20210522200019915.png)

  ![image-20210522200105731](https://hurq5.gitee.io/os-pictures/image-20210522200105731.png)

  ![image-20210522200157607](https://hurq5.gitee.io/os-pictures/image-20210522200157607.png)

  ![image-20210522200257918](https://hurq5.gitee.io/os-pictures/image-20210522200257918.png)

  ![image-20210522200333178](https://hurq5.gitee.io/os-pictures/image-20210522200333178.png)

  分析：程序开始执行时，先打印`./a.out write_or_read_thread_num.`提醒用户输入命令的第二个参数将作为读线程的线程数，和写线程的线程数，主函数创建共享内存（该共享内存的标记符为1507341），执行的结果表明，读写进程异步进入等待队列

  此时共享空间一开始处于可写不可读状态，因此等待队列中的唯一写线程4进入临界区，写线程4在共享空间中写下信息`“The message writed by write_thread-4”`；

  此时共享结构体处于可读不可写的状态，因此等待队列中优先级更高的读线程2进入临界区，读线程2在共享空间中读出信息`“The message writed by write_thread-4”`；

  此时共享空间处于可写不可读状态，因此等待队列中的唯一写线程3进入临界区，写线程3在共享空间中写下信息`“The message writed by write_thread-3”`；

  此时共享结构体处于可读不可写的状态，因此等待队列中唯一的读线程1进入临界区，读线程1在共享空间中读出信息`“The message writed by write_thread-3”`；

  。。。。。。。。

  读写线程交错进入临界区，读写线程之间存在互斥，同时在一个时间点不会用多个读线程或者写线程同时进入临界区，可以观察到程序很好的解决了线程互斥的问题。

  线程执行完毕，主函数删除共享内存（该共享内存的标记符为1507341），该共享内存为一开始我们创建的共享内存。