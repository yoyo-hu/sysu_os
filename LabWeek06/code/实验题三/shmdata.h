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

