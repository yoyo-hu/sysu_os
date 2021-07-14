/*
简介：程序模拟调度器调度线程任务的过程，可以选择不同的调度策略，包括先到先服务调度策略，
抢占式最短作业优先调度策略，非抢占式最短作业优先调度策略，抢占式优先级调度策略
，非抢占式优先级调度策略，程序每隔3秒随机创建三个线程并运行,对于抢占式的调度策略，
如果新线程具有更短的WCT或者更高的优先级，则线程被抢占（通过中断结合信号量实现），对于非抢占式的调度策略，
中断机制可有可无，这里是依旧存在中断过程，也可以加条件判断不执行中断过程，程序的最后
输出使用该调度策略执行完毕所有的预设线程任务后的所有线程任务的平均等待时间。
*/
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define NUM_OF_THREAD_NUM 3

/*线程任务状态*/
#define WAITING 0
#define READY 1
#define RUNNING 2

/*调度器使用到的调度策略*/
#define FCFS 1
#define PEM_SJF 2
#define NOT_PEM_SJF 3
#define PEM_PRIORITY 4
#define NOT_PEM_PRIORITY 5

static int CompletedThreadNum = 0;
struct timeval ttime;
sigset_t NullMask;  // sigsuspend()的参数

/*管理链队列节点*/
typedef struct threadNode {
    long arrTime;      //到达时间
    long lastingTime;  //持续时间
    pthread_t tid;
    int id;      //线程编号
    int WCT;     //最坏执行时间
    int status;  //调度状态
    sem_t sem;   //调度信号量
    int isRun;  //是否已经被开始运行，1为已经开始，0为尚未开始

    struct threadNode *next;
    struct threadNode *top;
    struct threadNode *rear;
    bool completed;              //该线程任务是否已经完成
    int priority;                // 优先级
    void *(*threadfun)(void *);  //线程函数

} threadNode;

threadNode *thr;

/* 调度队列节点 */
typedef struct threadQueueNode {
    struct threadNode *tNode;
    struct threadQueueNode *next;
} threadQueueNode;

/* 调度队列，调度队列中最符合调度策略的节点被调度到头结点执行 */
typedef struct threadQueue {
    threadQueueNode *top;
    pthread_mutex_t mutex;  //互斥锁

} threadQueue;
threadQueue Queue;

/*使线程阻塞的线程函数*/
void wait_fun(int signo) {
    printf("The above program is suspendedop\n");
    sigsuspend(&NullMask);  // 阻塞线程直到接收到一个新的信号
}

/*使得阻塞线程继续执行的函数*/
void cont_fun(int signo) { printf("The following procedure continues\n"); }
void fcfs(threadNode *thrNode) {
    threadQueueNode *temp = Queue.top;
    if (temp == NULL) {
        Queue.top = (threadQueueNode *)malloc(sizeof(threadQueueNode));
        Queue.top->tNode = thrNode;
        Queue.top->next = NULL;
        return;
    }
    while (temp->next != NULL) temp = temp->next;

    temp->next = (threadQueueNode *)malloc(sizeof(threadQueueNode));
    temp->next->tNode = thrNode;
    temp->next->next = NULL;
}

/*各个调度策略的执行函数*/
//抢占式有机会插在已经存在的头结点前面，作为新的头结点被调度器调度执行
void pem_sjf_push(threadNode *thrNode) {
    if (!Queue.top) {
        Queue.top = (threadQueueNode *)malloc(sizeof(threadQueueNode));
        Queue.top->tNode = thrNode;
        Queue.top->next = NULL;
    } else {
        threadQueueNode *temp;
        threadQueueNode *temp2;
        if (thrNode->WCT < Queue.top->tNode->WCT) {  //头插
            temp = Queue.top;
            threadQueueNode *temp2 = malloc(sizeof(threadQueueNode));
            temp2->tNode = thrNode;
            temp2->next = NULL;
            Queue.top = temp2;
            Queue.top->next = temp;
        } else {
            temp = Queue.top;
            while ((temp->next != NULL) &&
                   (temp->next->tNode->WCT <=
                    thrNode->WCT))  //插在WCT恰好比其小的结点后
                temp = temp->next;
            temp2 = temp->next;
            temp->next = (threadQueueNode *)malloc(sizeof(threadQueueNode));
            temp->next->tNode = thrNode;
            temp->next->next = temp2;
        }
    }
}
//非抢占式不可以插在已经存在的头结点前面，不可以抢占正在被调度的线程任务的CPU
void not_pem_sjf_push(threadNode *thrNode) {
    if (!Queue.top) {
        Queue.top = (threadQueueNode *)malloc(sizeof(threadQueueNode));
        Queue.top->tNode = thrNode;
        Queue.top->next = NULL;
    } else {
        threadQueueNode *temp;
        threadQueueNode *temp2;
        temp = Queue.top;
        while ((temp->next != NULL) &&
               (temp->next->tNode->WCT <=
                thrNode->WCT))  //插在WCT恰好比其小的结点后
            temp = temp->next;
        temp2 = temp->next;
        temp->next = (threadQueueNode *)malloc(sizeof(threadQueueNode));
        temp->next->tNode = thrNode;
        temp->next->next = temp2;
    }
}
//同理上面的抢占式
void pem_priority_push(threadNode *thrNode) {
    if (!Queue.top) {
        Queue.top = (threadQueueNode *)malloc(sizeof(threadQueueNode));
        Queue.top->tNode = thrNode;
        Queue.top->next = NULL;
    } else {
        threadQueueNode *temp;
        threadQueueNode *temp2;
        if (thrNode->priority > Queue.top->tNode->priority) {  //头插
            temp = Queue.top;
            threadQueueNode *temp2 = malloc(sizeof(threadQueueNode));
            temp2->tNode = thrNode;
            temp2->next = NULL;
            Queue.top = temp2;
            Queue.top->next = temp;
        } else {
            temp = Queue.top;
            while ((temp->next != NULL) &&
                   (temp->next->tNode->priority >= thrNode->priority))
                temp = temp->next;  //插在优先级恰好比其大的结点后
            temp = temp->next;
            temp2 = temp->next;
            temp->next = (threadQueueNode *)malloc(sizeof(threadQueueNode));
            temp->next->tNode = thrNode;
            temp->next->next = temp2;
        }
    }
}
//同理上面的非抢占式
void not_pem_priority_push(struct threadNode *thrNode) {
    if (!Queue.top) {
        Queue.top = (threadQueueNode *)malloc(sizeof(threadQueueNode));
        Queue.top->tNode = thrNode;
        Queue.top->next = NULL;
    } else {
        threadQueueNode *temp;
        threadQueueNode *temp2;
        temp = Queue.top;
        while ((temp->next != NULL) &&
               (temp->next->tNode->priority >=
                thrNode->priority))  //插在优先级恰好比其大的结点后
            temp = temp->next;
        temp2 = temp->next;
        temp->next = (threadQueueNode *)malloc(sizeof(threadQueueNode));
        temp->next->tNode = thrNode;
        temp->next->next = temp2;
    }
}

/*将到来的线程任务（转变为就绪态的线程任务）插入到调度队列中*/
void pushThread(struct threadNode *thrNode, int policy) {
    pthread_mutex_lock(&Queue.mutex);  //互斥锁防止多个线程同时访问调度队列

    //根据调度策略插入方法
    switch (policy) {
        case FCFS:
            fcfs(thrNode);
            break;
        case PEM_SJF:
            pem_sjf_push(thrNode);
            break;
        case NOT_PEM_SJF:
            not_pem_sjf_push(thrNode);
            break;
        case PEM_PRIORITY:
            pem_priority_push(thrNode);
            break;
        case NOT_PEM_PRIORITY:
            not_pem_priority_push(thrNode);
            break;
    }
    sleep(1);

    pthread_mutex_unlock(&Queue.mutex);
    return;
}

/* 在线程任务已经完成后，弹出调度队列 */
void popThread(void) {
    pthread_mutex_lock(&Queue.mutex);  //互斥锁防止多个线程同时访问调度队列

    if (Queue.top) {
        threadQueueNode *temp = Queue.top->next;
        free(Queue.top);
        Queue.top = temp;
    }

    if (Queue.top) {  //引发下一个被调度器调度的线程任务执行
        if (Queue.top->tNode->isRun == 1) {
            pthread_kill(Queue.top->tNode->tid, SIGUSR2);
            Queue.top->tNode->status = RUNNING;
        } else {
            sem_post(&(Queue.top->tNode->sem));
            Queue.top->tNode->status = RUNNING;
        }
    }

    pthread_mutex_unlock(&Queue.mutex);
}

/* 1号线程任务执行函数 */
void *fun1(void *arg) {
    static int i = 0;
    threadNode *Thr = (threadNode *)arg;
    Thr->isRun = 1;
    sem_wait(&(Thr->sem));  //等待调度启用
    Thr->status = RUNNING;
    // 1号线程任务的时间为10，用10次sleep（1）的循环来模拟
    for (; i < 10; ++i) {
        printf("Thread 1 's %dth second\n", i + 1);
        sleep(1);
    }
    //任务执行结束，从调度队列中弹出，设置任务持续的时间
    Thr->completed = true;
    popThread();
    gettimeofday(&ttime, 0);
    Thr->lastingTime =
        (long)(ttime.tv_sec * 1000 * 1000) + ttime.tv_usec - Thr->arrTime;
    CompletedThreadNum++;
    return NULL;
}
/* 2号线程任务执行函数 */
void *fun2(void *arg) {
    static int i = 0;
    threadNode *Thr = (threadNode *)arg;
    Thr->isRun = 1;
    sem_wait(&(Thr->sem));  //等待调度启用
    Thr->status = RUNNING;
    // 2号线程任务的时间为5，用5次sleep（1）的循环来模拟
    for (; i < 5; ++i) {
        printf("Thread 2 's %dth second\n", i + 1);
        sleep(1);
    }
    //任务执行结束，从调度队列中弹出，设置任务持续的时间
    Thr->completed = true;
    popThread();
    gettimeofday(&ttime, 0);
    Thr->lastingTime =
        (long)(ttime.tv_sec * 1000 * 1000) + ttime.tv_usec - Thr->arrTime;
    CompletedThreadNum++;
    return NULL;
}
/* 3号线程任务执行函数 */
void *fun3(void *arg) {
    static int i = 0;
    threadNode *Thr = (threadNode *)arg;
    Thr->isRun = 1;
    sem_wait(&(Thr->sem));  //等待调度启用
    Thr->status = RUNNING;
    // 3号线程任务的时间为2，用2次sleep（1）的循环来模拟
    for (; i < 2; ++i) {
        printf("Thread 3 's %dth second\n", i + 1);
        sleep(1);
    }
    //任务执行结束，从调度队列中弹出，设置任务持续的时间
    Thr->completed = true;
    popThread();
    gettimeofday(&ttime, 0);
    Thr->lastingTime =
        (long)(ttime.tv_sec * 1000 * 1000) + ttime.tv_usec - Thr->arrTime;
    CompletedThreadNum++;
    return NULL;
}

int uncall_thread_num = NUM_OF_THREAD_NUM;  //未到达的线程数
/* 生成随机线程的函数，使线程任务队列中的元素随机到达 */
threadNode *GetRandThread(void) {
    static int uncall_thread_id[] = {1, 2, 3};
    clock_t t;
    int i, num;
    threadNode *tagetThr = NULL;
    if (uncall_thread_num > 0) {
        srand(time(0));
        //生成随机线程编号uncall_thread_id[num]
        num = rand() % (uncall_thread_num);
        //找到随机编号的线程任务节点
        tagetThr = thr->top;
        while (tagetThr != NULL && tagetThr->id != uncall_thread_id[num]) {
            tagetThr = tagetThr->next;
        }
        if (tagetThr == NULL) return NULL;

        //将已经到达的线程任务编号从uncall_thread_id中删除
        for (i = num; i < uncall_thread_num - 1; ++i) {
            uncall_thread_id[i] = uncall_thread_id[i + 1];
        }
        uncall_thread_num--;
    }
    return tagetThr;
}
/* 模拟调度器*/
/* 以3秒的间隔随机顺序调进三个线程任务，使用信号中断实现可能的抢占 */
void Scheduler(int policy) {
    clock_t t;
    threadNode *tagetThr;

    while (1) {
        /*如果线程任务队列中得到线程任务没有被全部调入调度队列，
        则选择未被调入的随机线程任务，并根据调度策略调入调度队列*/
        if (uncall_thread_num > 0) {
            tagetThr = GetRandThread();
            // 如果有新的线程任务进入且此时有任务正在运行，那么发送SIGUSR1阻塞这个任务，并修改其调度状态为就绪态
            if (Queue.top != NULL) {
                pthread_kill(Queue.top->tNode->tid, SIGUSR1);
                Queue.top->tNode->status = READY;
            }
            pushThread(tagetThr, policy);
            //设置被调入的随机线程到达的时间为此时的时间，单位是微秒
            gettimeofday(&ttime, 0);
            tagetThr->arrTime =
                (long)(ttime.tv_sec * 1000 * 1000) + ttime.tv_usec;
            printf("Thread %d is called\n", tagetThr->id);
            pthread_create(&(tagetThr->tid), NULL, tagetThr->threadfun,
                           tagetThr);

            //调度队列插入新的线程任务且调度器完成调度后，唤醒被调度器选中的线程任务执行
            if (Queue.top->tNode->isRun == 1) {
                pthread_kill(Queue.top->tNode->tid, SIGUSR2);
            } else {
                sem_post(&(Queue.top->tNode->sem));
            }
            Queue.top->tNode->status = RUNNING;
        } else
            break;
        sleep(3);
        //当所有的线程任务都被执行完毕后，退出调度器的模拟函数
        if (CompletedThreadNum == NUM_OF_THREAD_NUM) return;
    }
    while (1) {
        if (CompletedThreadNum == NUM_OF_THREAD_NUM) return;
    }
}

/*预设的3个线程任务的相关信息*/
void Init(void) {
    struct threadNode *thr1 =
        (struct threadNode *)malloc(sizeof(struct threadNode));
    thr = thr1;
    thr->top = thr1;
    thr1->id = 1;
    thr1->next = NULL;
    thr1->isRun = 0;
    thr1->status = WAITING;
    thr1->completed = false;
    thr1->threadfun = fun1;
    sem_init(&(thr1->sem), 0,
             0);  //初始化任务的调度信号量为0，表示需要等待被调度
    thr1->priority = 0;
    thr1->WCT = 10;  //这里指相对时间，任务1执行了10轮循环
    thr->rear = thr1;

    struct threadNode *thr2 =
        (struct threadNode *)malloc(sizeof(struct threadNode));
    thr->rear->next = thr2;
    thr2->id = 2;
    thr2->next = NULL;
    thr2->isRun = 0;
    thr->rear = thr2;
    thr2->status = WAITING;
    thr2->completed = false;
    thr2->threadfun = fun2;
    sem_init(&(thr2->sem), 0, 0);
    thr2->priority = 1;
    thr1->WCT = 5;  //这里指相对时间，任务2执行了5轮循环

    struct threadNode *thr3 =
        (struct threadNode *)malloc(sizeof(struct threadNode));
    thr->rear->next = thr3;
    thr3->id = 3;
    thr3->next = NULL;
    thr3->isRun = 0;
    thr->rear = thr3;
    thr3->status = WAITING;
    thr3->completed = false;
    thr3->threadfun = fun3;
    sem_init(&(thr3->sem), 0, 0);
    thr3->priority = 2;
    thr3->WCT = 2;  //这里指相对时间，任务3执行了2轮循环
}

/*回收预设的3个线程任务的内存空间*/
void ReclaimMem() {
    threadNode *tmp1 = thr->top;
    threadNode *tmp2 = thr->top;
    while (tmp1 != NULL) {
        tmp2 = tmp1;
        sem_destroy(&(tmp1->sem));  //销毁信号量
        free(tmp1);
        tmp1 = tmp2->next;
    }
}

/*计算任务平均等待时间*/
long Cal_aver_wait_time() {
    long sumTime = 0;
    threadNode *tmp = thr->top;
    while (tmp != NULL) {
        sumTime = sumTime + tmp->lastingTime - (tmp->WCT * 1000 * 1000);
        tmp = tmp->next;
    }
    return sumTime / NUM_OF_THREAD_NUM;
}

int main() {
    int i;
    /*设置程序使用信号的处理函数*/
    struct sigaction act1, act2;
    memset(&act1, 0, sizeof(act1));
    memset(&act2, 0, sizeof(act2));

    act1.sa_flags = 0;
    act2.sa_flags = 0;
    sigemptyset(&act1.sa_mask);
    sigemptyset(&act2.sa_mask);
    act1.sa_handler = wait_fun;
    act2.sa_handler = cont_fun;

    sigaction(SIGUSR1, &act1,
              NULL);  // 设置SIGUSR1引发act1的处理函数，使线程阻塞
    sigaction(
        SIGUSR2, &act2,
        NULL);  // 设置捕捉到SIGUSR2引发act1的处理函数，使阻塞中的线程继续执行

    /*初始化线程任务属性*/
    Init();

    /*初始化调度队列，其中头结点为占用CPU的线程任务*/
    Queue.top = NULL;
    pthread_mutex_init(&Queue.mutex, NULL);  //初始化互斥锁

    /* 调用调度器 */
    int strategy;
    printf("1.FCFS\n");
    printf("2.PEM_SJF\n");
    printf("3.NOT_PEM_SJF\n");
    printf("4.PEM_PRIORITY\n");
    printf("5.NOT_PEM_PRIORITY\n");
    printf("Choose a scheduling strategy:");  //根据提示选择策略
    scanf("%d", &strategy);
    if (strategy >= 1 && strategy <= 5)  //策略合法，调用调度器
        Scheduler(strategy);
    else {
        printf("Illegal choice!");  //策略不合法，打印出错信息，不调用调度器
    }
    threadNode *tmp = thr->top;
    while (tmp != NULL) {
        pthread_join(tmp->tid, NULL);
        tmp = tmp->next;
    }
    /*计算任务平均等待时间*/
    printf("Calculation task average waiting time is %ld mrs\n",
           Cal_aver_wait_time());
    /*资源处理以及回收*/
    pthread_mutex_destroy(&Queue.mutex);  //销毁互斥锁
    ReclaimMem();                         //回收资源
    return 0;
}
