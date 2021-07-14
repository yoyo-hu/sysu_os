#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_TASK_NUM 30
#define MAX_THREAD_NUM 10
#define gettid() syscall(__NR_gettid)
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/* 任务,任务回调函数,任务队列 */
typedef struct worker {
    void* (*callback)(void* arg); /*任务回调函数*/
    void* arg;                    /*回调函数的参数*/
    struct worker* next;          /*任务队列链表*/
} CThread_worker;
static int tesk_count = 0;
/*回调函数*/
static void* callback(void* arg) {
    printf("threadid:%ld, working on task %d\n", (long int)pthread_self(),
           *(int*)arg);
    tesk_count++;
    sleep(1);
    return (NULL);
}

/*线程池结构*/
typedef struct {
    CThread_worker* queue_head; /*线程池的任务队列*/
    sem_t* queue_sem;
    int shutdown;        /*是否摧毁线程池 0:不摧毁 1:摧毁 */
    pthread_t* threadid; /*线程ID数组*/
    int max_thread_num;  /*线程池最大线程数*/
    int cur_queue_size;  /*任务队列在任务数目*/

} CThread_pool;
/*线程函数*/
void* thread_routine(void* arg);

/*线程池实例*/
static CThread_pool* pool = NULL;

/*线程池初始化*/
void pool_init(int max_thread_num) {
    /*一些列初始化*/
    pool = (CThread_pool*)malloc(sizeof(CThread_pool));

    pool->queue_sem = (sem_t*)malloc(sizeof(sem_t));
    int ret = sem_init(pool->queue_sem, 0, 0);
    if (ret == -1) {
        perror("sem_init()");
        return;
    }
    pool->queue_head = NULL;
    pool->max_thread_num = max_thread_num;
    pool->shutdown = 0; /*0打开1关闭*/
    pool->cur_queue_size = 0;
    pool->threadid = (pthread_t*)malloc(max_thread_num * sizeof(pthread_t));

    /*创建工作线程*/
    int i = 0;
    for (i = 0; i < max_thread_num; ++i) {
        pthread_create(&(pool->threadid[i]), NULL, thread_routine, NULL);
    }
}

/*将任务加入队列*/
int pool_add_worker(void* (*callback)(void* arg), void* arg) {
    /*构造一个新任务*/
    printf("pool add worker arg:%d\n", *(int*)arg);
    CThread_worker* newworker = (CThread_worker*)malloc(sizeof(CThread_worker));
    newworker->callback = callback;
    newworker->arg = arg;
    newworker->next = NULL; /*SET NULL*/

    pthread_mutex_lock(&mutex);

    /*将任务加入到任务队列中,也就是链表末端*/
    CThread_worker* worker = pool->queue_head;
    if (worker != NULL) {
        while (worker->next != NULL) worker = worker->next;
        worker->next = newworker;
    } else {
        pool->queue_head = newworker;
    }

    pool->cur_queue_size += 1; /*计数+1*/
    sem_post(pool->queue_sem);
    pthread_mutex_unlock(&mutex);

    return 0;
}

/*销毁线程池*/
int pool_destroy() {
    printf("pool destroy now\n");

    /*启用关闭开关*/
    if (pool->shutdown) return -1; /*防止两次调用*/
    pool->shutdown = 1;

    /*唤醒所有等待线程*/
    int i;
    /*因为创建线程比作业多，未执行任务，一直循环等待信号量的线程将会成为僵尸，
    所以通过给出足够的信号，来唤醒所有线程从而避免这种情况的发生*/
    for (i = 0; i < pool->max_thread_num; i++) {
        sem_post(pool->queue_sem);
    }
    sleep(1);
    sem_destroy(pool->queue_sem);

    /*阻塞等待线程退出回收资源，还有另一种办法就是线程分离*/
    for (i = 0; i < pool->max_thread_num; ++i)
        pthread_join(pool->threadid[i], NULL);
    free(pool->threadid);
    pool->threadid = NULL;

    /*销毁任务队列*/
    CThread_worker* head = NULL;
    while (pool->queue_head != NULL) {
        head = pool->queue_head;
        pool->queue_head = pool->queue_head->next;
        free(head);
        head = NULL;
    }

    /*dealloc semphore*/
    free(pool->queue_sem);
    free(pool);
    pool = NULL;
    printf("pool destroy end\n");
    return 0;
}

/*工作线程函数*/
void* thread_routine(void* arg) {
    printf("starting threadid:%ld\n", (long int)pthread_self());
    if (sem_wait(pool->queue_sem)) {
        perror("thread waiting for semaphore");
        exit(EXIT_FAILURE);
    }
    for (;;) {
        pthread_mutex_lock(&mutex);
        /*任务队列为空时wait唤醒,当销毁线程池时跳出循环*/
        while (pool->cur_queue_size == 0 && !pool->shutdown) {
        }

        /*线程池要销毁了*/
        if (pool->shutdown) {
            pthread_mutex_unlock(&mutex);
            printf("threadid:%ld will exit\n", (long int)pthread_self());
            pthread_exit(NULL);
        }

        /*开始执行任务*/
        printf("threadid:%ld is starting to work\n", (long int)pthread_self());

        /*等待队列长度减去1，并取出链表中的头元素*/
        pool->cur_queue_size -= 1;
        CThread_worker* worker = pool->queue_head;
        pool->queue_head = worker->next;
        /*调用回调函数，执行任务*/
        pthread_mutex_unlock(&mutex);
        (*(worker->callback))(worker->arg);
        free(worker);
        worker = NULL;
    }
    return (NULL);
}

/*测试*/
int main(int argc, char const* argv[]) {
    printf("./a.out threadNum taskNum\n");
    int threadNum = 4;  //默认线程数为4
    if (argc > 1)
        threadNum = atoi(
            argv[1]);  //如果执行的命令行存在第二个参数时，第二个参数为线程数
    if (threadNum < 1 && threadNum > MAX_THREAD_NUM) {
        printf("0 < threadNum < %d", MAX_THREAD_NUM);
        return 0;
    }
    // initPool(threadNum); //创建指定线程数个线程*/
    int taskNum = 5;  //默认任务数为5
    if (argc > 2) taskNum = atoi(argv[2]);
    if (taskNum < 1 && taskNum > MAX_TASK_NUM) {
        printf("0 < taskNum < %d", MAX_TASK_NUM);
        return 0;
    }
    pool_init(threadNum); /*创建n个线程*/
    /*添加n个任务*/
    int* workingnum = (int*)malloc(sizeof(int) * taskNum); /* 一定要动态创建 */
    printf("threadNum=%d,taskNum=%d\n", threadNum, taskNum);
    int i;
    for (i = 0; i < taskNum; ++i) {
        workingnum[i] = i;
        pool_add_worker(callback, &workingnum[i]);
    }
    if (taskNum % threadNum == 0)
        sleep(taskNum / threadNum);
    else
        sleep(taskNum / threadNum + 1);  //等待所有任务完成

    pool_destroy(); /*销毁线程池*/
    free(workingnum);
    workingnum = NULL;
    printf("The total number of tasks executed by the thread pool is %d\n",
           tesk_count);
    return 0;
}