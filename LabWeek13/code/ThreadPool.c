#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_TASK_NUM 50
#define MAX_THREAD_NUM 10
#define SLEEP_TIME 5
#define THREAD_WORK SLEEP_TIME
/* 任务的结构体*/
typedef struct task {
    void *(*calledFunc)(void *arg);  //任务调用的函数
    void *arg;                       //传入调用函数的参数
    struct task *next;               //指向队列的下一个任务
} threadTask;

/*任务调用的函数*/
static void *calledFunc(void *arg) {
    printf("Thread whose id is %ld is working on task %d.\n",
           (long int)pthread_self(), *(int *)arg);
    sleep(1);
    return (NULL);
}

/*线程池的结构体*/
typedef struct {
    threadTask *taskQueue;  //线程池的任务队列*/
    int ifShutdown;  //是否摧毁线程池,为0表示不摧毁，为1表示不摧毁
    pthread_t *threadId;  //线程的ID数组
    int maxThreadNum;     //线程池允许的最大线程数*/
    int curQueueNum;      //任务队列的当前任务数目*/

} threadPool;

/*线程函数*/
void *poolFunc(void *arg);

/*线程池实例*/
static threadPool *pool = NULL;

/*初始化线程池*/
void initPool(int maxThreadNum) {
    //申请一个线程池并设置其参数
    pool = (threadPool *)malloc(sizeof(threadPool));
    pool->taskQueue = NULL;
    pool->curQueueNum = 0;              //任务队列为空
    pool->maxThreadNum = maxThreadNum;  //设置最大的线程数为输入函数的参数
    pool->ifShutdown = 0;               //不摧毁线程池
    pool->threadId = (pthread_t *)malloc(maxThreadNum * sizeof(pthread_t));

    //创建线程
    for (int i = 0; i < maxThreadNum; i++) {
        //线程执行poolFunc工作函数
        pthread_create(&(pool->threadId[i]), NULL, poolFunc, NULL);
    }
}

/*任务加入队列*/
int addTaskToPool(void *(*calledFunc)(void *arg), void *arg) {
    //创建一个新任务，并设置其参数
    threadTask *aTask = (threadTask *)malloc(sizeof(threadTask));
    aTask->calledFunc = calledFunc;
    aTask->arg = arg;
    aTask->next = NULL;

    //将创建后的任务放在任务队列中
    threadTask *tempTask = pool->taskQueue;
    if (tempTask != NULL) {
        //队列有头结点
        while (tempTask->next != NULL) tempTask = tempTask->next;
        tempTask->next = aTask;
    }

    else {
        pool->taskQueue = aTask;
    }

    pool->curQueueNum++;  //当前任务队列数加一
    return 0;
}
/*销毁线程池*/
int destroyPool() {
    printf("-------------------------------------------------------\n");
    printf("Begin destroying pool.\n");
    //线程池已摧毁或者在摧毁，返回，防止多个函数同时摧毁线程池
    if (pool->ifShutdown) return -1;
    pool->ifShutdown = 1;

    //阻塞等待所有的线程退出并回收资源
    for (int i = 0; i < pool->maxThreadNum; i++)
        pthread_join(pool->threadId[i], NULL);

    //释放线程池空间，包括任务队列，id数组
    threadTask *head = NULL;
    while (pool->taskQueue != NULL) {
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

/*工作线程函数*/
void *poolFunc(void *arg) {
    printf("-------------------------------------------------------\n");
    printf("Thread whose id is %ld calls poolFunc.\n",
           (long int)pthread_self());

    while (1) {
        //当任务队列为空时，线程等待
        while (pool->curQueueNum <= 0 && !pool->ifShutdown) {
            printf("Thread whose id is %ld is waiting.\n",
                   (long int)pthread_self());
        }

        //当线程池被销毁或正在被摧毁时，退出工作程序
        if (pool->ifShutdown) {
            printf("Thread whose id is %ld exits.\n", (long int)pthread_self());
            pthread_exit(NULL);
        }

        //空闲线程开始执行任务
        printf("Thread whose id is %ld is starting to work.\n",
               (long int)pthread_self());

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

/*测试*/
int main(int argc, char const *argv[]) {
    printf("./a.out threadNum taskNum\n");
    int threadNum = 4;  //默认线程数为4
    if (argc > 1)
        threadNum = atoi(
            argv[1]);  //如果执行的命令行存在第二个参数时，第二个参数为线程数
    if (threadNum < 1 || threadNum > MAX_THREAD_NUM) {
        printf("0<threadNum<%d", MAX_THREAD_NUM);
        return 0;
    }
    initPool(threadNum);  //创建指定线程数个线程*/
    int taskNum = 5;      //默认任务数为5
    if (argc > 2) taskNum = atoi(argv[2]);
    if (taskNum < 1 || taskNum > THREAD_WORK * threadNum) {
        printf("0<taskNum<%d", 5 * threadNum);
        return 0;
    }
    printf("threadNum=%d,taskNum=%d\n", threadNum, taskNum);
    //添加指定任务数个任务到任务队列中
    int *func = (int *)malloc(sizeof(int) * taskNum); /* 一定要动态创建 */
    for (int i = 0; i < taskNum; i++) {
        func[i] = i;
    }
    for (int i = 0; i < taskNum; i++) {
        printf("Add task %d\n", i);
        addTaskToPool(calledFunc, &func[i]);
    }
    if (taskNum % threadNum == 0)
        sleep(taskNum / threadNum);
    else
        sleep(taskNum / threadNum + 1);  //等待所有任务完成
    destroyPool();                       //销毁线程池
    free(func);
    return 0;
}