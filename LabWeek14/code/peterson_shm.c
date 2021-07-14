
#define TEXT_SIZE 4 * 1024 /* = PAGE_SIZE, size of each message */
#define TEXT_NUM 1         /* maximal number of mesages */
/* total size can not exceed current shmmax,
   or an 'invalid argument' error occurs when shmget */

#define PERM S_IRUSR | S_IWUSR | IPC_CREAT

#define ERR_EXIT(m)         \
    do {                    \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)
#define MAX_N 1024
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
/* a demo structure, modified as needed */
struct shared_struct {
    int written;           /* flag = 0: buffer writable; others: readable */
    char mtext[TEXT_SIZE]; /* buffer for message reading and writing */
};

static int counter = 0;
/* number of process(s) in the critical section */
int level_read[MAX_N];
/* level number of processes 0 .. MAX_N-1 */
int waiting_read[MAX_N - 1];
/* waiting process of each level number 0 .. MAX_N-2 */
int level_write[MAX_N];
/* level number of processes 0 .. MAX_N-1 */
int waiting_write[MAX_N - 1];
/* waiting process of each level number 0 .. MAX_N-2 */
int max_num = 20; /* default max thread number */
key_t key;        /* of type int */
static void *wirteFtn(void *arg) {
    int *numptr = (int *)arg;
    int thread_num = *numptr;
    int lev, k, j;
    void *shmptr = NULL;
    struct shared_struct *shared = NULL;
    int shmid;
    char buffer[BUFSIZ + 1]; /* 8192bytes, saved from stdin */

    shmid = shmget((key_t)key, TEXT_NUM * sizeof(struct shared_struct),
                   0666 | PERM);
    if (shmid == -1) {
        ERR_EXIT("shmwite: shmget()");
    }
    shmptr = shmat(shmid, 0, 0);
    if (shmptr == (void *)-1) {
        ERR_EXIT("shmwrite: shmat()");
    }
    shared = (struct shared_struct *)shmptr;
    printf("write_thread-%d, ptid = %lu working\n", thread_num, pthread_self());

    for (lev = 0; lev < max_num - 1;
         ++lev) { /* there are at least max_num-1 waiting rooms */
        level_write[thread_num] = lev;
        waiting_write[lev] = thread_num;
        while (waiting_write[lev] == thread_num) { /* busy waiting */
            /*  && (there exists k != thread_num, such that level[k] >= lev)) */
            for (k = 0; k < max_num; k++) {
                if (level_write[k] >= lev && k != thread_num) {
                    break;
                }
                if (waiting_write[lev] != thread_num) { /* check again */
                    break;
                }
            } /* if any other proces j with level[j] < lev upgrades its level to
                 or greater than lev during this period, then process thread_num
                 must be kicked out the waiting room and waiting[lev] !=
                 thread_num, and then exits the while loop when scheduled */
            if (k == max_num) { /* all other processes have level of less than
                                   process thread_num */
                break;
            }
        }
    }
    /* critical section of process thread_num */
    while (shared->written == 1) sleep(1);
    printf("write_thread-%d, ptid = %lu entering the critical section\n",
           thread_num, pthread_self());
    counter++;
    sprintf(buffer, "The message writed by write_thread-%d", thread_num);
    strncpy(shared->mtext, buffer, TEXT_SIZE);
    printf("write_thread-%d write: %s\n", thread_num, shared->mtext);
    shared->written = 1; /* message prepared */

    /* detach the shared memory */
    if (shmdt(shmptr) == -1) {
        ERR_EXIT("shmwrite: shmdt()");
    }
    if (counter > 1) {
        printf("ERROR! more than one processes in their critical sections\n");
        kill(getpid(), SIGKILL);
    }

    counter--;
    /* end of critical section */

    level_write[thread_num] = -1;
    /* allow other process of level max_num-2 to exit the while loop
       and enter his critical section */
    pthread_exit(0);
}
static void *readFtn(void *arg) {
    int *numptr = (int *)arg;
    int thread_num = *numptr;
    int lev, k, j;
    void *shmptr = NULL;
    struct shared_struct *shared;
    int shmid;
    shmid = shmget((key_t)key, TEXT_NUM * sizeof(struct shared_struct),
                   0666 | PERM);
    if (shmid == -1) {
        ERR_EXIT("shread: shmget()");
    }

    shmptr = shmat(shmid, 0, 0);
    if (shmptr == (void *)-1) {
        ERR_EXIT("shread: shmat()");
    }

    shared = (struct shared_struct *)shmptr;
    printf("%*sread_thread-%d, ptid = %lu working\n", 20, " ", thread_num,
           pthread_self());

    for (lev = 0; lev < max_num - 1;
         ++lev) { /* there are at least max_num-1 waiting rooms */
        level_read[thread_num] = lev;
        waiting_read[lev] = thread_num;
        // while(shared->written == 0)
        // sleep(1);
        while (waiting_read[lev] == thread_num) { /* busy waiting */
            /*  && (there exists k != thread_num, such that level[k] >= lev)) */
            for (k = 0; k < max_num; k++) {
                if (level_read[k] >= lev && k != thread_num) {
                    break;
                }
                if (waiting_read[lev] != thread_num) { /* check again */
                    break;
                }
            } /* if any other proces j with level[j] < lev upgrades its level to
                 or greater than lev during this period, then process thread_num
                 must be kicked out the waiting room and waiting[lev] !=
                 thread_num, and then exits the while loop when scheduled */
            if (k == max_num) { /* all other processes have level of less than
                                   process thread_num */
                break;
            }
        }
    }
    /* critical section of process thread_num */
    while (shared->written == 0) sleep(1);
    counter++;
    printf("%*sread_thread-%d, ptid = %lu entering the critical section\n", 20,
           " ", thread_num, pthread_self());
    printf("%*sThe read_thread-%d: read: %s\n", 20, " ", thread_num,
           shared->mtext);
    shared->written = 0;
    /* it is not reliable to use shared->written for process synchronization */

    if (shmdt(shmptr) == -1) {
        ERR_EXIT("shmread: shmdt()");
    }
    if (counter > 1) {
        printf("ERROR! more than one processes in their critical sections\n");
        kill(getpid(), SIGKILL);
    }
    counter--;
    /* end of critical section */

    level_read[thread_num] = -1;
    /* allow other process of level max_num-2 to exit the while loop
       and enter his critical section */
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    printf("Usage: ./a.out total_thread_num\n");
    if (argc > 1) {
        max_num = atoi(argv[1]);
    }
    if (max_num < 0 || max_num > MAX_N) {
        printf("invalid max_num\n");
        exit(1);
    }
    memset(level_write, (-1), sizeof(level_write));
    memset(waiting_write, (-1), sizeof(waiting_write));
    memset(level_read, (-1), sizeof(level_read));
    memset(waiting_read, (-1), sizeof(waiting_read));

    int i, ret;
    int thread_num[max_num];
    pthread_t ptidRead[max_num];
    pthread_t ptidWrite[max_num];

    for (i = 0; i < max_num; i++) {
        thread_num[i] = i;
    }

    printf("total thread number = %d\n", max_num);
    printf("main(): pid = %d, ptid = %lu.\n", getpid(), pthread_self());
    struct stat fileattr;
    // key_t key; /* of type int */
    int shmid; /* shared memory ID */
    void *shmptr;
    struct shared_struct *shared; /* structured shm */
    char pathname[80];
    int shmsize;

    shmsize = TEXT_NUM * sizeof(struct shared_struct);
    strcpy(pathname, "myshm");

    if (stat(pathname, &fileattr) == -1) {
        ret = creat(pathname, O_RDWR);
        if (ret == -1) {
            ERR_EXIT("creat()");
        }
        printf("shared file object created\n");
    }

    key = ftok(pathname,
               0x27); /* 0x27 a project ID 0x0001 - 0xffff, 8 least bits used */
    if (key == -1) {
        ERR_EXIT("shmcon: ftok()");
    }
    shmid = shmget((key_t)key, shmsize, 0666 | PERM);
    if (shmid == -1) {
        ERR_EXIT("shmcon: shmget()");
    }
    printf("shmcon: shmid = %d\n", shmid);
    shmptr =
        shmat(shmid, 0, 0); /* returns the virtual base address mapping to the
                               shared memory, *shmaddr=0 decided by kernel */

    if (shmptr == (void *)-1) {
        ERR_EXIT("shmcon: shmat()");
    }
    shared = (struct shared_struct *)shmptr;
    shared->written = 0;

    if (shmdt(shmptr) == -1) {
        ERR_EXIT("shmcon: shmdt()");
    }

    for (i = 0; i < max_num; i++) {
        ret = pthread_create(&ptidWrite[i], NULL, &wirteFtn,
                             (void *)&thread_num[i]);
        if (ret != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        }
    }
    for (i = 0; i < max_num; i++) {
        ret = pthread_create(&ptidRead[i], NULL, &readFtn,
                             (void *)&thread_num[i]);
        if (ret != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        }
    }

    for (i = 0; i < max_num; i++) {
        ret = pthread_join(ptidWrite[i], NULL);
        if (ret != 0) {
            perror("pthread_join()");
        }
    }
    for (i = 0; i < max_num; i++) {
        ret = pthread_join(ptidRead[i], NULL);
        if (ret != 0) {
            perror("pthread_join()");
        }
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        ERR_EXIT("shmcon: shmctl(IPC_RMID)");
    } else {
        printf("shmcon: shmid = %d removed \n", shmid);
    }
    return 0;
}
