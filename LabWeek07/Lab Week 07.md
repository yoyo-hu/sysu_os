# lab Week 07.

## 实验内容：

进程间通信—共享内存。实现一个带有 n 个单元的线性表的并发维护。 

## 实验实现：

### 实验部分一：

* 要求：

  建立一个足够大的共享内存空间 (lock, M)，逻辑值 lock 用来保证同一 时间只有一个进程进入 M；测试你的系统上 M 的上限。 

* 源代码以及详细注释：

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <sys/shm.h>
  #include <fcntl.h>
  #define TEST_SIZE 33554429
  //共享结构体
  struct shared_struct {
      char test[TEST_SIZE];/* buffer for message reading and writing */
      int lock;/* lock = 0: buffer writable; others: readable */
  };
  #define PERM S_IRUSR|S_IWUSR|IPC_CREAT
  
  #define ERR_EXIT(m) \
      do { \
          perror(m); \
          exit(EXIT_FAILURE); \
      } while(0)
  
  
  int main(int argc, char *argv[])
  {
      struct stat fileattr;
      key_t key; /* of type int*/
      int shmid; /* shared memory ID *////共享内存标识符,创建共享内存
      void *shmptr;
      struct shared_struct *shared; /* structured shm*/
      pid_t childpid1, childpid2;
      char pathname[80], key_str[10], cmd_str[80];
      int shmsize, ret;
  
      shmsize = sizeof(struct shared_struct); //共享内存的大小
      printf("shm size = %d\n", shmsize);
      if(argc <2) {
          printf("Usage: ./a.out pathname\n");
          return EXIT_FAILURE;
      }
  	//执行编译好的执行文件时要加上共享文件名
      strcpy(pathname, argv[1]);
  	//如果共享文件不符合要求或者没有共享文件
  	//则如下create共享文件
      if(stat(pathname, &fileattr) == -1) {
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
      //get共享内存,成功返回共享内存的ID，出错返回-1 
  
      shmid = shmget((key_t)key, shmsize, 0666|PERM);
      //0666是给全面的读写权限
      //shmid:共享内存标识符
  	if (shmid == -1) {
          printf("The shared memory size is %d, which exceeds the maximum shared memory limit\n", shmsize);//申请的共享内存过大超过了共享内存上限
          ERR_EXIT("hread: shmget()");
      }
      printf("shmcon: shmid = %d\n", shmid);
  	////将共享内存连接到当前进程的地址空间（挂接操作）
      //成功返回共享存储段的指针，出错返回-1
      shmptr = shmat(shmid, 0, 0);
  /* returns the virtual base address mapping to the shared memory, *shmaddr=0 decided by kernel */
  //shmat()函数的作用就是用来启动对该共享内存的访问，并把共享内存连接到当前进程的地址空间
      if(shmptr == (void *)-1) {
          ERR_EXIT("shread: shmat()");
      }
      printf("shmcon: shared Memory attached at %p\n", shmptr);
      //进行强制类型转换，防止编译器发出warming
      //将shmptr强制转换为struct shared_struct *类型的指针并赋给shared之后的目的是为了将shared看作为一个共享对象在共享内存区进行操作了。
      shared = (struct shared_struct *)shmptr;
      shared->lock = 0;
  	//设置lock使共享内存段可写
      sprintf(cmd_str, "ipcs -m | grep '%d'\n", shmid);
      //ipcs -m，查看共享内存
      //grep 指令用于查找内容包含指定的范本样式的文件，在这里显示所有包含共享内存标识符的行。
      printf("\n------ Shared Memory Segments ------\n");
      system(cmd_str);//调用系统命令查看IPC对象：显示所有包含共享内存标识符的行。
     //此处分别显示共享内存地址，共享内存标识符，进程拥有者，权限，字节连接数和状态
      if (shmdt(shmptr) == -1) {
  //把共享内存从当前进程中分离，使该共享内存对当前进程不再可用
          //调用失败返回-1
          ERR_EXIT("shmread: shmdt()");
      }
   
      printf("The shared memory size is %d, which is within the maximum shared memory range\n", shmsize);
      //删除共享内存段
        if (shmctl(shmid, IPC_RMID, 0) == -1) {
           ERR_EXIT("shmcon: shmctl(IPC_RMID)");
         }
  	exit(EXIT_SUCCESS);
  }
  
  ```

  

* 运行结果图以及分析：

  1. 使用以下两个命令行之一的命令行查看共享内存段的最大字节数：
  
     ```
     $ cat /proc/sys/kernel/shmmax
     $sysctl kernel.shmmax 
     ```
  
     ![image-20210406154828340](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210406154828340.png)
  
  2. 在33554432（共享内存段的最大字节数大小）上下给定共享内存段的字节数，测试系统的上限：如下图所示：
  
     ![image-20210406155201852](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210406155201852.png)
  
     ![image-20210406155120750](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210406155120750.png)
  
     分析：当共享数据段的大小为33554432时，创建的共享内存数据段大小在共享内存段的最大的字节数之内，因此可以创立成功；
  
     当共享数据段大小为33554436时，创建的共享内存数据段大小超过限制，因此创建（shmget）失败。

### 实验部分二：

*  要求：设计一个程序在 M 上建立一个结点信息结构为 (flag, 学号, 姓名) 的静态链表 L，逻辑值 flag 用作结点的删除标识；

   在 L 上建立一个以学号为 关键字的二元小顶堆，自行设计控制结构 (如静态指针数据域)。

*  相关知识点：静态链表，堆，栈相关结构和基本操作的知识点

* 源代码及其详细注释：

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #define MAXSIZE 1000 
  
  /*栈*/
  typedef struct Stack{
      int index[MAXSIZE];  //存储空闲坐标
      int top;     //栈顶
  } Stack;
  
  typedef struct Studentnode {
      int id;         //学号，结点信息
      char name[20];  //姓名，结点信息
      int pre;    //前一个结点的下标，结点为空则为-1
      int nex;   //后一个结点的下标，结点为空则为-1
      int flag;       //用作结点的删除标识,其中flag=0代表结点删除，flag=1代表结点未删除
  } Student;
  
  
  /*静态链表*/
  typedef struct ustatic_list {
      Student list[MAXSIZE];
      Stack nouse_index;     //静态链表中还未使用的下标
      int head;          //静态链表头节点在数组中的下标
      int size;               //结点数目
  } StaticLinkList;
  
  /*在静态链表上实现的最小堆*/
  typedef struct ustatic_list* MinHeap;
  
  
  /*栈结构的相关操作*/
  
  //初始化，备用结点使用栈的形式存储
  void initiateStack(Stack *ustack) {
      //一开始所有结点都处于备用的状态
      for (int i = 0; i < MAXSIZE; ++i) {
          ustack->index[i] = MAXSIZE - i - 1;
      }
      ustack->top = MAXSIZE - 1;
  }
  //进栈
  void pushStack(Stack *ustack, int unusedIndex) {
      ustack->index[++ustack->top] = unusedIndex;
  }
  //出栈
  void popStack(Stack *ustack) {
      if (ustack->top == -1)
          return;
      ustack->top--;
  }
   //取栈顶元素
  int topStack(const Stack *ustack) {
      if (ustack->top == -1)
          return -1;
      return ustack->index[ustack->top];
  }
  
  /* 存储学生信息的结构体的相关操作*/
  
  //初始化静态链表
  void initialStaticLinkList(StaticLinkList *ustatic_list) {
      //静态链表开始元素在数组中的下标设为-1
      ustatic_list->head = -1;
      //静态链表中存储的学生信息数目为0
      ustatic_list->size = 0;
      initiateStack(&ustatic_list->nouse_index);
  }
  
  //加入静态链表
  void insertListAtHead(StaticLinkList *ustatic_list, Student stu) {
      //在备用结点存放的栈中取出备用结点，采用头插法将学生插入到静态链表中
      int index = topStack(&ustatic_list->nouse_index);
      popStack(&ustatic_list->nouse_index);
      ustatic_list->list[index] = stu;
      ustatic_list->list[index].flag = 1;
      ustatic_list->list[index].pre = -1;
      ustatic_list->list[index].nex = ustatic_list->head;
      if (ustatic_list->head != -1) {
          ustatic_list->list[ustatic_list->head].pre = index;
      }
      ustatic_list->head = index;
      ustatic_list->size++;
  }
  
  /* 以学号为关键字的最小堆的操作 */
  
  //交换最小堆中的两个结点，但是不改变它们在静态链表中所处的顺序
  void swap(MinHeap h, int parent, int child) {
      Student t = h->list[parent];
      h->list[parent] = h->list[child];
      h->list[child] = t; 
  }
  
  //将未经过排序的堆结构重排变成堆
  void rearrangeHeap(MinHeap h) {
      int n = h->size;
      for (int i = n / 2 - 1; i >= 0; --i) {
          Student temp = h->list[i];
          int parent = i;
          int child = (i << 1) + 1;
          while (child < n) {
              //选择儿子结点中较小的那个结点
              if (child + 1 < n && h->list[child].id > h->list[child + 1].id) {
                  child = child + 1;
              }
              //当父节点大于子结点的时候要进行调整
              if (temp.id > h->list[child].id) {
                  swap(h, parent, child);
                  //及时更新头结点坐标
                  if (h->list[parent].pre == -1) h->head = parent;
                  if (h->list[child].pre == -1) h->head = child;
                  parent = child;
                  child = (parent << 1) + 1;
              } else
                  break;
          }
      }
  }
  
  //向最小堆中添加学生信息
  void pushHeap(MinHeap h, Student *stu) {
      int child = topStack(&h->nouse_index);
      popStack(&h->nouse_index);
      //向数组的空闲下标的位置插入数据
      h->list[child].id = stu->id;
      strcpy(h->list[child].name, stu->name);
      //对学生信息进行加工调整后再添加
      h->list[h->head].pre = child;
      h->list[child].flag = 1;
      h->list[child].pre = -1;
      h->list[child].nex = h->head;
      h->size++;
      int parent = (child - 1) / 2;
      //如果此时插入元素的学号比它的父亲大，同时插入数据还没有成为根结点，那么将它的父亲保存到小顶堆的位置，插入元素继续向上比较
      while (stu->id < h->list[parent].id && child > 0) {
          swap(h, parent, child);
          child = parent;
          parent = (child - 1) / 2;
      }
      h->head = child;
  }
  
  //从小顶堆中删除根结点的数据
  void popHeap(MinHeap h) {
      //小顶堆为空，直接返回
      if (h->size == 0) return;
      int high, low;
      //获取小顶堆数组的最后一个元素
      Student temp = h->list[--h->size];
      pushStack(&h->nouse_index, h->size);
      h->list[h->head].flag = 0;
      if (h->size == 0) return;
      swap(h, 0, h->size);
      for (high = 0; high * 2 + 1 < h->size; high = low) {
          //从根结点开始，high为父亲结点的位置，low为左儿子的位置
          low = high * 2 + 1;
          //选择儿子结点中学号较小的那个结点
          if (low != h->size - 1 && h->list[low].id > h->list[low + 1].id) {
              low += 1;
          }
          //如果这个结点比小顶堆数组的最后一个元素的学号小，那么将这个结点放到它的父亲结点
          if (h->list[low].id < temp.id) {
              swap(h, high, low);
              if (h->list[high].pre == -1) h->head = high;
              if (h->list[low].pre == -1) h->head = low;
              break;
          }
      }
      //调整被删除结点前后结点的坐标相对顺序和表头结点
      int pre_index = h->list[h->size].pre;
      int nex_index = h->list[h->size].nex;
      if (pre_index != -1) h->list[pre_index].nex = nex_index;
      if (nex_index != -1) h->list[nex_index].pre = pre_index; 
      if (pre_index == -1) h->head = nex_index;
  }
  
  //从小顶堆返回最小元素
  Student topHeap(MinHeap t) {
      return t->list[0];
  }
  
  //判断小顶堆是否为空
  int isEmptyHeap(MinHeap t) {
      return t->size == 0;
  }
  
  //从最小堆中根据学生信息找到该学生在静态链表中的下标
  int findHeap(MinHeap h, Student *stu, int curIndex) {
      //当前角标不合法
      if (curIndex >= h->size||curIndex<0) return -1;
      //根节点即为要找的坐标位置
      if (h->list[curIndex].id == stu->id && strcmp(h->list[curIndex].name, stu->name) == 0) {
          return curIndex;
      }
      int l_index = -1;
      int r_index = -1;
      //先找左子數
      if (curIndex * 2 + 1 < h->size) {
          l_index = findHeap(h, stu, curIndex * 2 + 1);
      }
      //左子數找到了就返回坐标
      if (l_index != -1) return l_index;
      //左子树找不到，再找右子树
      if (curIndex * 2 + 2 < h->size) {
          r_index = findHeap(h, stu, curIndex * 2 + 2);
      }
       //右子树找到了则返回坐标
      if (r_index != -1) return r_index;
      //都找不到返回-1
      return -1;
  }
  //修改学生姓名
  void modifyName(MinHeap h, Student *stu, char *name) {
      int index = findHeap(h, stu, 0);
      if (index == -1) {
          printf("can't find the target student!\n");
      } else {
          strcpy(h->list[index].name, name);
      }
  }
  
  //修改学生学号
  void modifyId(MinHeap h, Student *stu, int id) {
      int index = findHeap(h, stu, 0);
      if (index == -1) {
          printf("can't find the target student!\n");
      } else {
          h->list[index].id = id;
          rearrangeHeap(h);
      }
  }
  
  //把堆中的元素按照一维数组的顺序打印出来，并显示它们在一维数组中的下标
  void printHeap(MinHeap h) {
      for(int i = 0; i < h->size; ++i) {
          printf("location info->(Index: %d  Pre: %d  Next: %d) \t student info->(Id: %d  Name: %s) \n",i, h->list[i].pre, h->list[i].nex, h->list[i].id, h->list[i].name);
      }
  }
  
  int main() {
      //静态链表
      StaticLinkList ustatic_list;
      initialStaticLinkList(&ustatic_list);
      //申请8个学生身份
      Student stu1;stu1.id=11;strcpy(stu1.name,"student1");
      Student stu2;stu2.id=20;strcpy(stu2.name,"student2");
      Student stu3;stu3.id=40;strcpy(stu3.name,"student3");
      Student stu4;stu4.id=50;strcpy(stu4.name,"student4");
      Student stu5;stu5.id=10;strcpy(stu5.name,"student5");
      Student stu6;stu6.id=5;strcpy(stu6.name,"student6");
      Student stu7;stu7.id=22;strcpy(stu7.name,"student7");
      Student stu8;stu8.id=1;strcpy(stu8.name,"student8");
  
      //先将前6个学生信息插入到静态链表中
      insertListAtHead(&ustatic_list, stu1);
      insertListAtHead(&ustatic_list, stu2);
      insertListAtHead(&ustatic_list, stu3);
      insertListAtHead(&ustatic_list, stu4);
      insertListAtHead(&ustatic_list, stu5);
      insertListAtHead(&ustatic_list, stu6);
  
      printf("--->Before rearrange:\n");//未重排前
      printHeap(&ustatic_list);
      printf("\n\n");//将不同状态的显示隔开
  
      /*测试可以实现重排*/
      rearrangeHeap(&ustatic_list);
      printf("--->After rearranging the static array into the smallest h:\n");
      printHeap(&ustatic_list);
      printf("\n\n");//将不同状态的显示隔开
  
      /*测试可以实现插入*/
      //向堆中插入元素
      printf("--->Insert the 7th student into the h\n");
      pushHeap(&ustatic_list, &stu7);
      printHeap(&ustatic_list);
      printf("\n\n");//将不同状态的显示隔开
  
      printf("--->Insert the 8th student into the h\n");
      pushHeap(&ustatic_list, &stu8);
      printHeap(&ustatic_list);
      printf("\n\n");//将不同状态的显示隔开
  
      /*测试可以实现弹出*/
      printf("--->Pop 1\n");
      popHeap(&ustatic_list);
      printHeap(&ustatic_list);
      printf("\n\n");//将不同状态的显示隔开
  
      printf("--->Pop 2\n");
      popHeap(&ustatic_list);
      printHeap(&ustatic_list);
      printf("\n\n");//将不同状态的显示隔开
  
      /*测试可以实现修改*/
      printf("--->Change the student ID of the first student to 77\n\n");
      modifyId(&ustatic_list, &stu1, 77);
      printHeap(&ustatic_list);
      printf("\n\n");//将不同状态的显示隔开
  
      printf("--->Change the name of the third student to 'STUDENT3'\n");
      modifyName(&ustatic_list, &stu3, "STUDENT3");
      printHeap(&ustatic_list);
      printf("\n\n");//将不同状态的显示隔开
  
      printf("--->Find the seventh student's index\n");
      int index = findHeap(&ustatic_list, &stu7, 0);
      printf("The stu7's index is %d\n", index);
  }
  ```


* 执行结果说明：

  1. 先申请8个学生身份待插入堆中：

     ```c
     //申请8个学生身份
         Student stu1;stu1.id=11;strcpy(stu1.name,"student1");
         Student stu2;stu2.id=20;strcpy(stu2.name,"student2");
         Student stu3;stu3.id=40;strcpy(stu3.name,"student3");
         Student stu4;stu4.id=50;strcpy(stu4.name,"student4");
         Student stu5;stu5.id=10;strcpy(stu5.name,"student5");
         Student stu6;stu6.id=5;strcpy(stu6.name,"student6");
         Student stu7;stu7.id=22;strcpy(stu7.name,"student7");
         Student stu8;stu8.id=1;strcpy(stu8.name,"student8");
     ```

     

  2. 在将静态链表重排成最小堆之前，先向静态链表中插入前6个学生信息；打印此时堆（未经过重排）中结点的位置和学生信息，如下图：

     ![image-20210407205856594](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210407205856594.png)

     分析：可以看到6个学生成功头插到静态链表中

  3. 将静态链表中的结点重排成堆后，打印此时堆中结点的位置和学生信息，如下图：

     ![image-20210407210617415](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210407210617415.png)

     分析：可以观察到

     1. 头结点为角标为0的点，其id值最小为5.其子结点（即坐标为1和2的结点），id的值为10，11；
     2. 角标为1的结点，id值为10，其子节点（即坐标为3，4），id值为50，20.
     3. 角标为2的结点，id值为11，其子节点（即坐标为5），id值为40.

     综上，满足小顶堆的结构，重排成功。

  4. 将申请好的第7个学生插入堆中，打印此时堆中结点的位置和学生信息，如下图：

     ![image-20210407211201245](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210407211201245.png)

     分析：可以观察到插入第7个学生信息后，堆仍然保持最小堆的结构

  5. 将申请好的第8个学生插入堆中，打印此时堆中结点的位置和学生信息，如下图：

     ![image-20210407211431152](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210407211431152.png)

     分析：可以观察到插入第8个学生信息后，堆仍然保持最小堆的结构

  6. 弹出一个结点，打印此时堆中结点的位置和学生信息，如下图：

     ![image-20210407214029873](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210407214029873.png)

     分析：可以观察到弹出的结点为id最小值的点，弹出后，堆仍然保持最小堆的结构。

  7. 再弹出一个结点，打印此时堆中结点的位置和学生信息，如下图：

     ![image-20210407214313700](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210407214313700.png)

     分析：可以观察到弹出的结点为id最小值的点，弹出后堆仍然保持最小堆的结构。

  8. 修改第一个学生的id为77，打印此时堆中结点的位置和学生信息，如下图：

     ![image-20210407214438437](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210407214438437.png)

     分析：修改后，堆仍然保持最小堆的结构。

  9. 修改第三个学生的名字为STUDENT3，打印此时堆中结点的位置和学生信息，如下图：

     ![image-20210407214634166](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210407214634166.png)

     分析：修改成功，堆的结构没有发生改变

  10. 查找第7个学生在静态链表中的角标：

      ![image-20210407214810520](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210407214810520.png)

      分析：正确的查找到了学生在静态链表中的位置。

### 实验部分三

* 要求：设计一个程序对上述堆结构的结点实现插入、删除、修改、查找、重 排等操作。该程序的进程可以在多个终端并发执行。

* 相关知识：

  1. 不同终端输入的共享文件名字相同，印次在创建共享内存时，ftok()函数会生成一个相同的IPC 键值，在多个终端运行程序时，就可以在程序中对同一个共享对象进行操作。
  2. lock在一定程度上避免进程冲突的机制：访问共享空间（堆）前，先检查冲突标志（这里是互斥锁lock），如果没有冲突则访问，并通过设置互斥锁为1，锁住共享空间的门，防止其它的所有进程访问共享空间；如果有冲突（即lock=1，有其他进程在执行）则阻塞等待。

* 源代码以及详细的代码：

  头文件（主要部分是步骤2数据结构的代码）shm.h
  
  ```c
#include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <time.h>
  #define MAXSIZE 1000 
  #define MAX_TEST_TIME 30
  /*栈*/
  typedef struct Stack{
      int index[MAXSIZE];  //存储空闲坐标
      int top;     //栈顶
  } Stack;
  
  typedef struct Studentnode {
      int id;         //学号，结点信息
      char name[20];  //姓名，结点信息
      int pre;    //前一个结点的下标，结点为空则为-1
      int nex;   //后一个结点的下标，结点为空则为-1
      int flag;       //用作结点的删除标识,其中flag=0代表结点删除，flag=1代表结点未删除
  } Student;
  
  
  /*静态链表*/
  typedef struct shared_struct {
      Student list[MAXSIZE];
      Stack nouse_index;     //静态链表中还未使用的下标
      int head;          //静态链表头节点在数组中的下标
      int size;               //结点数目
      int lock;		//进程锁
      int testtime;	//测试次数
  } StaticLinkList;
  
  /*在静态链表上实现的最小堆*/
  typedef struct shared_struct * MinHeap;
  /*栈结构的相关操作*/
  
  //共享的结构体
  #define PERM S_IRUSR|S_IWUSR|IPC_CREAT
  
  #define ERR_EXIT(m) \
      do { \
          perror(m); \
          exit(EXIT_FAILURE); \
      } while(0)
  //退出代码
  
  //初始化，备用结点使用栈的形式存储
  void initiateStack(Stack *ustack) {
      //一开始所有结点都处于备用的状态
      for (int i = 0; i < MAXSIZE; ++i) {
          ustack->index[i] = MAXSIZE - i - 1;
      }
      ustack->top = MAXSIZE - 1;
  }
  //进栈
  void pushStack(Stack *ustack, int unusedIndex) {
      ustack->index[++ustack->top] = unusedIndex;
  }
  //出栈
  void popStack(Stack *ustack) {
      if (ustack->top == -1)
          return;
      ustack->top--;
  }
   //取栈顶元素
  int topStack(const Stack *ustack) {
      if (ustack->top == -1)
          return -1;
      return ustack->index[ustack->top];
  }
  
  /* 存储学生信息的结构体的相关操作*/
  
  //初始化静态链表
  void initialStaticLinkList(StaticLinkList *ustatic_list) {
      //静态链表开始元素在数组中的下标设为-1
      ustatic_list->head = -1;
      //静态链表中存储的学生信息数目为0
      ustatic_list->size = 0;
      //静态链表被操作的次数为0
      ustatic_list->testtime=0;
      initiateStack(&ustatic_list->nouse_index);
  }
  
  //加入静态链表
  void insertListAtHead(StaticLinkList *ustatic_list, Student stu) {
      //在备用结点存放的栈中取出备用结点，采用头插法将学生插入到静态链表中
      int index = topStack(&ustatic_list->nouse_index);
      popStack(&ustatic_list->nouse_index);
      ustatic_list->list[index] = stu;
      ustatic_list->list[index].flag = 1;
      ustatic_list->list[index].pre = -1;
      ustatic_list->list[index].nex = ustatic_list->head;
      if (ustatic_list->head != -1) {
          ustatic_list->list[ustatic_list->head].pre = index;
      }
      ustatic_list->head = index;
      ustatic_list->size++;
  }
  
  /* 以学号为关键字的最小堆的操作 */
  
  //交换最小堆中的两个结点，但是不改变它们在静态链表中所处的顺序
  void swap(MinHeap h, int parent, int child) {
      Student t = h->list[parent];
      h->list[parent] = h->list[child];
      h->list[child] = t; 
  }
  
  //将未经过排序的堆结构重排变成堆
  void rearrangeHeap(MinHeap h) {
      int n = h->size;
      for (int i = n / 2 - 1; i >= 0; --i) {
          Student temp = h->list[i];
          int parent = i;
          int child = (i << 1) + 1;
          while (child < n) {
              //选择儿子结点中较小的那个结点
              if (child + 1 < n && h->list[child].id > h->list[child + 1].id) {
                  child = child + 1;
              }
              //当父节点大于子结点的时候要进行调整
              if (temp.id > h->list[child].id) {
                  swap(h, parent, child);
                  //及时更新头结点坐标
                  if (h->list[parent].pre == -1) h->head = parent;
                  if (h->list[child].pre == -1) h->head = child;
                  parent = child;
                  child = (parent << 1) + 1;
              } else
                  break;
          }
      }
  }
  
  //向最小堆中添加学生信息
  void pushHeap(MinHeap h, Student *stu) {
      int child = topStack(&h->nouse_index);
      popStack(&h->nouse_index);
      //向数组的空闲下标的位置插入数据
      h->list[child].id = stu->id;
      strcpy(h->list[child].name, stu->name);
      //对学生信息进行加工调整后再添加
      h->list[h->head].pre = child;
      h->list[child].flag = 1;
      h->list[child].pre = -1;
      h->list[child].nex = h->head;
      h->size++;
      int parent = (child - 1) / 2;
      //如果此时插入元素的学号比它的父亲大，同时插入数据还没有成为根结点，那么将它的父亲保存到小顶堆的位置，插入元素继续向上比较
      while (stu->id < h->list[parent].id && child > 0) {
          swap(h, parent, child);
          child = parent;
          parent = (child - 1) / 2;
      }
      h->head = child;
  }
  
  //从小顶堆中删除根结点的数据
  void popHeap(MinHeap h) {
      //小顶堆为空，直接返回
      if (h->size == 0) return;
      int high, low;
      //获取小顶堆数组的最后一个元素
      Student temp = h->list[--h->size];
      pushStack(&h->nouse_index, h->size);
      h->list[h->head].flag = 0;
      if (h->size == 0) return;
      swap(h, 0, h->size);
      for (high = 0; high * 2 + 1 < h->size; high = low) {
          //从根结点开始，high为父亲结点的位置，low为左儿子的位置
          low = high * 2 + 1;
          //选择儿子结点中学号较小的那个结点
          if (low != h->size - 1 && h->list[low].id > h->list[low + 1].id) {
              low += 1;
          }
          //如果这个结点比小顶堆数组的最后一个元素的学号小，那么将这个结点放到它的父亲结点
          if (h->list[low].id < temp.id) {
              swap(h, high, low);
              if (h->list[high].pre == -1) h->head = high;
              if (h->list[low].pre == -1) h->head = low;
              break;
          }
      }
      //调整被删除结点前后结点的坐标相对顺序和表头结点
      int pre_index = h->list[h->size].pre;
      int nex_index = h->list[h->size].nex;
      if (pre_index != -1) h->list[pre_index].nex = nex_index;
      if (nex_index != -1) h->list[nex_index].pre = pre_index; 
      if (pre_index == -1) h->head = nex_index;
  }
  
  //从小顶堆返回最小元素
  Student topHeap(MinHeap t) {
      return t->list[0];
  }
  
  //判断小顶堆是否为空
  int isEmptyHeap(MinHeap t) {
      return t->size == 0;
  }
  
  //根据学生信息找到其在静态链表中的下标
  int findHeap(MinHeap h, Student *stu, int curIndex) {
      //当前角标不合法
      if (curIndex >= h->size||curIndex<0) return -1;
      //根节点即为要找的坐标位置
      if (h->list[curIndex].id == stu->id && strcmp(h->list[curIndex].name, stu->name) == 0) {
          return curIndex;
      }
      int l_index = -1;
      int r_index = -1;
      //先找左子數
      if (curIndex * 2 + 1 < h->size) {
          l_index = findHeap(h, stu, curIndex * 2 + 1);
      }
      //左子數找到了就返回坐标
      if (l_index != -1) return l_index;
      //左子树找不到，再找右子树
      if (curIndex * 2 + 2 < h->size) {
          r_index = findHeap(h, stu, curIndex * 2 + 2);
      }
       //右子树找到了则返回坐标
      if (r_index != -1) return r_index;
      //都找不到返回-1
      return -1;
  }
  //根据学生的学号找到其在静态链表中的下标
  int findHeapID(MinHeap h, int id,int curIndex) {
      //当前角标不合法
      if (curIndex >= h->size||curIndex<0) return -1;
      //根节点即为要找的坐标位置
      if (h->list[curIndex].id == id ) {
          return curIndex;
      }
      int l_index = -1;
      int r_index = -1;
      //先找左子數
      if (curIndex * 2 + 1 < h->size) {
          l_index = findHeapID(h, id, curIndex * 2 + 1);
      }
      //左子數找到了就返回坐标
      if (l_index != -1) return l_index;
      //左子树找不到，再找右子树
      if (curIndex * 2 + 2 < h->size) {
          r_index = findHeapID(h, id, curIndex * 2 + 2);
      }
      //右子树找到了则返回坐标
      if (r_index != -1) return r_index;
      //都找不到返回-1
      return -1;
  }
  
  
  //修改学生姓名
  void modifyName(MinHeap h, Student *stu, char *name) {
      int index = findHeap(h, stu, 0);
      if (index == -1) {
          printf("can't find the target student!\n");
      } else {
          strcpy(h->list[index].name, name);
      }
  }
  
  //修改学生学号
  void modifyId(MinHeap h, Student *stu, int id) {
      int index = findHeap(h, stu, 0);
      if (index == -1) {
          printf("can't find the target student!\n");
      } else {
          h->list[index].id = id;
          rearrangeHeap(h);
      }
  }
  
  
  
  //把堆中的元素按照一维数组的顺序打印出来，并显示它们在一维数组中的下标
  void printHeap(MinHeap h) {
      for(int i = 0; i < h->size; ++i) {
          printf("location info->(Index: %d  Pre: %d  Next: %d) \t student info->(Id: %d  Name: %s) \n",i, h->list[i].pre, h->list[i].nex, h->list[i].id, h->list[i].name);
      }
  }
  
  ```
  
  主函数：shm.c
  
  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <sys/shm.h>
  #include <fcntl.h>
  #include "shm.h"
  
  int main(int argc, char *argv[]) {
      struct stat fileattr;
      key_t key; // of type int
      int shmid; // shared memory ID
      void *shmptr;
      struct shared_struct *shared; // structured shm
  
      char pathname[80];
      int shmsize, ret;
  
      shmsize = sizeof(struct shared_struct); // 共享内存的大小
  	//argc用来统计命令的单词数
      if(argc <2) {
          printf("Usage: ./a.out pathname\n");    //表示在编译命令后面还要加上共享文件路径名argv[]
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
   
      key = ftok(pathname, 0x27); // 0x27 a project ID 0x0001 - 0xffff, 8 least bits used
      if(key == -1) {
          ERR_EXIT("shmcon: ftok()");
      }
  
      shmid = shmget((key_t)key, shmsize, 0666|PERM);
      if(shmid == -1) {
          ERR_EXIT("shmcon: shmget()");
      }
  
      shmptr = shmat(shmid, 0, 0); // returns the virtual base address mapping to the shared memory, *shmaddr=0 decided by kernel
  
      if(shmptr == (void *)-1) {
          ERR_EXIT("shmcon: shmat()");
      }
      
      shared = (StaticLinkList *)shmptr; //将shmptr强制转换为struct shared_struct *类型的指针并赋给shared之后的目的是为了将shared看作为一个共享对象在共享内存区进行操作了
      initialStaticLinkList(shared); //初始化静态链表
      shared->lock = 0; //逻辑值lock设为0，代表进程可以执行
      shared->testtime = 0; //对结构体的操作次数
  	//播种随机种子
      srand((unsigned) time(NULL));
  	//当测试次数到达上限时候。退出循环，进程不能再操作，退出循环
      while (shared->testtime < MAX_TEST_TIME) {
          //测试次数+1
  		shared->testtime++;
          //当进程锁住的时候（lock==1）的时候，进程休眠
          while (shared->lock == 1) {
              sleep(1);
          }
          wait(0);
          //访问共享内存前，将共享内存锁住（将lock设为1），防止其它进程访问共享内存（使之休眠）
          shared->lock = 1;
       //使用随机数来堆对小顶堆进行插入、删除、查找、修改之一的随机操作
          int Operand = rand() % 4 + 1;
  	//根据Operand的值，进行堆操作的选择
  	switch (Operand) {
          case 1: {
          //Operand==1执行插入操作
  	    printf("Operand Is Push:\n");
              //id在最大学生数范围内随机产生
              int temp_id = rand() % 1000;
              char name[5];
              //根据一定的规则和id生成名字
              /*以下是命名的过程*/
              if(temp_id/100!=0){
                  name[0]=temp_id/100+'a';
                  name[1]=temp_id%100/10+'a';
                  name[2]=temp_id%10+'a';
                  name[3]='\0';
              }
              else if(temp_id/10!=0){
                  name[0]=temp_id/10+'a';
                  name[1]=temp_id%10+'a';
                  name[2]='\0';
              }
              else{
                  name[0]=temp_id+'a';
                  name[1]='\0';
              }
              /*以上是命名的过程*/
              Student temp_s;temp_s.id=temp_id;strcpy(temp_s.name,name);
              pushHeap(shared, &temp_s);
              printf("push->student information：(id: %d name: %s)\n ", temp_id, name);
              break;
          }
  		//Operand==2执行删除操作
          case 2: {
              printf("Operand Is Pop:\n");
              if (shared->size == 0) {
                  printf("The heap is empty\n");
              }
              else {
                  printf("pop->student information：(id: %d name: %s)\n", shared->list[0].id, shared->list[0].name);
                  popHeap(shared);
              }
              break;
          }
  		//Operand==3执行查找操作
          case 3: {
              printf("Operand Is Find:\n");
              //id在最大学生数范围内随机产生
              int temp_id = rand() % 1000;
              int index = findHeapID(shared, temp_id, 0);
              //当index返回-1的时候，没有找到拥有该id的学生
              if (index == -1) {
                  printf("There is no student whose student ID is %d\n", temp_id);
              }
              else {
                  printf("find->student(ID=%d)'s index is %d\n", temp_id, index);
              }
              break;
          }
  		//Operand==4执行修改操作
          case 4: {
              printf("Operand Is Modify:\n");
              if (shared->size == 0) {
                  printf("The heap is empty\n");
              }
              else {
                  int modifyIndex = rand() % shared->size;			
                  //id在最大学生数范围内随机产生
                  int temp_id = rand() % 1000;
                  char name[5];
              //根据一定的规则和id生成名字
              /*以下是命名的过程*/
  	    if(temp_id/100!=0){
                  name[0]=temp_id/100+'a';
                  name[1]=temp_id%100/10+'a';
                  name[2]=temp_id%10+'a';
                  name[3]='\0';
              }
              else if(temp_id/10!=0){
                  name[0]=temp_id/10+'a';
                  name[1]=temp_id%10+'a';
                  name[2]='\0';
              }
  	    else{
                  name[0]=temp_id+'a';
                  name[1]='\0';
              }
              /*以上是命名的过程*/
                  printf("The student(index:%d):\n",temp_id);
                  printf("BeforeModify->student information：(id: %d  name: %s) \n", shared->list[modifyIndex].id, shared->list[modifyIndex].name);
  		modifyId(shared, &shared->list[modifyIndex], temp_id);//修改id
          modifyName(shared, &shared->list[modifyIndex], name);//修改名字
                  printf("AfterModify->student information：(id: %d  name: %s) \n", shared->list[modifyIndex].id, shared->list[modifyIndex].name);
              }
              break;
                  
          }
      }
          printHeap(shared);
          //休眠4s，避免多进程冲突（只要睡眠时间相对进程执行足够长，进程冲突的几率就可以很低）
          sleep(4);
          shared->lock = 0;//解锁（lock设为0），允许其它进程访问共享内存
  	printf("-----------------------------------------------\n\n");//隔开不同的操作
      }
  
  	//当共享空间达到可以被操作次数的上限的时候，上面的循环退出
      //打印最终堆
      printf("Reached the upper limit of the number of tests 40, the final heap information is as follows：\n");
      printHeap(shared);
      if(shmdt(shmptr) == -1) {
          ERR_EXIT("shmcon: shmdt()");
      }
      //休眠10秒等待所有进程退出，为后面的共享空间删除过程提供便利，这样可以有效避免进程操作已经删除导致的程序出错
      sleep(10);
      if (shmctl(shmid, IPC_RMID, 0) == -1) {
          ERR_EXIT("shmcon: shmctl(IPC_RMID)");
      }
  
      exit(EXIT_SUCCESS);
  }
  
  ```
  
* 执行过程分析：

  执行下面命令行进行编译运行

  ```c
  gcc -std=c99 -D _SVID_SOURCE shm.c
  //使用c99标准，以及兼容不同系统的指令编译程序
  ./a.out 1
  //给定文件名1，执行指令
  ```

  1. 同时开5个进程同步执行该程序：如下图所示：

     ![image-20210408084722724](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210408084722724.png)

     分析：可以观察到5个进程在同步执行，且在同一个时间点只有一个终端在操作堆，可见进程的冲突得到了一定程度上的避免。

  2. 5个进程全部结束（结束标记：有新的命令行输入接口的出现）（在sleep10秒的过程中相继结束），输出同样的最终堆结构

     ![image-20210408084829770](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210408084829770.png)

  3. 分别观察5个终端的最后2个操作以及最终堆的输出：

     * 第一个终端：

       ![image-20210408085058210](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210408085058210.png)

       **分析：**可以观察到该进程最后两个操作都是push操作，第一个push是将学号为590，名字为fja的学生信息插入到堆中，打印的结构有学号为590，名字为fja的学生信息，并且打印的堆符合最小堆的结构，说明插入成功。

       第二个push是将学号为153，名字为bfd的学生信息插入到堆中，打印的结构有学号为153，名字为bfd的学生信息，并且打印的堆符合最小堆的结构，说明插入成功。

       最终输出的堆包括两个学生（一个学生的id为984，名字为jie，另一个学生的id为999，名字为jjj）

     * 第二个终端：

       ![image-20210408085550568](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210408085550568.png)

       **分析：**可以观察到该进程最后两个操作，一个是modify操作，将id=781，名字为hib的学生更改成名字为dfb，学号为351的学生，打印的结构没有id=781，名字为hib的学生信息，而存在名字为dfb，学号为351的学生信息，并且打印的堆符合最小堆的结构，说明更改成功。

       另一个操作是push操作，将学号为463，名字为egd的学生信息插入到堆中，打印的结构有学号为463，名字为egd的学生信息，并且打印的堆符合最小堆的结构，说明插入成功。

       最终输出的堆包括两个学生（一个学生的id为984，名字为jie，另一个学生的id为999，名字为jjj），和上面的终端的输出结果相同

     * 第三个终端：

       ![image-20210408090004052](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210408090004052.png)

       **分析：**可以观察到该进程最后两个操作，

       一个是pop操作，将id=351，名字为dfb的学生删除；打印的结构已经没有id=351，名字为dfb的学生信息，并且打印的堆符合最小堆的结构。说明删除成功。

       另一个操作是find操作，在堆中没有查找到id=83的学生；打印出来的堆确实没有id==83的学生，说明find结果正确。

       最终输出的堆包括两个学生（一个学生的id为984，名字为jie，另一个学生的id为999，名字为jjj），和上面的终端的输出结果相同

     * 第四个终端：

       ![image-20210408090317238](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210408090317238.png)

       **分析：**可以观察到该进程最后两个操作，

       一个是pop操作，将id=97，名字为jh的学生删除；打印的结构已经没有id=97，名字为jh的学生信息，并且打印的堆符合最小堆的结构，说明删除成功。

       另一个操作是modify修改操作，将id=590，名字为fja的学生更改成名字为999，学号为jjj的学生，打印的结构没有id=590，名字为fja的学生信息，而存在名字为999，学号为jjj的学生信息，并且打印的堆符合最小堆的结构，说明更改成功。

       最终输出的堆包括两个学生（一个学生的id为984，名字为jie，另一个学生的id为999，名字为jjj），和上面的终端的输出结果相同

     * 第五个终端：

       ![image-20210408090946238](http://hurq5.gitee.io/os-labwork/LabWeek07/pictures/image-20210408090946238.png)

       

       **分析：**可以观察到该进程最后两个操作，

       一个是find操作，在堆中没有查找到id=645的学生；打印出来的堆确实没有id==645的学生，说明find结果正确。

       另一个操作是modify修改操作，将id=153，名字为bfd的学生更改成名字为984，学号为jie的学生，打印的结构没有id=153，名字为bfd的学生信息，而存在名字为984，学号为jie的学生信息，并且打印的堆符合最小堆的结构，说明更改成功。

       最终输出的堆包括两个学生（一个学生的id为984，名字为jie，另一个学生的id为999，名字为jjj），和上面的终端的输出结果相同

     综上，所有进程最终输出的堆是相同的，进一步说明5个进程操作的是同一片共享空间（同一个堆），进程之间比较稳定的同步进行着。

### 实验部分四：

* 思考：使用逻辑值 lock 实现的并发机制不能解决条件冲突问题。

  使用lock逻辑值来防止多进程对共享空间的同时访问，以防止共享空间的混乱和异常，在单个cpu的状态下的效果是可观的，但是对于多cpu来说，效果并不是理想的，这是因为lock值的赋值更改需要经过内存到寄存器，寄存器到ALU，ALU到内存三个过程，当进程有多个cpu占用的时候，lock的值就会在寄存器操作的过程中被多次访问或者同时访问造成冲突，如内存，寄存器的同时读写，从而导致条件冲突问题。这就是为什么说使用逻辑值 lock 实现的并发机制不能解决条件冲突问题。
  
  
  
  