#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>

#define PERM S_IRUSR|S_IWUSR|IPC_CREAT

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

#define TEST_SIZE 39999997

struct shared_struct {
    char test[TEST_SIZE];
    int lock;
};

int main(int argc, char *argv[])
{
    struct stat fileattr;
    key_t key; // of type int
    int shmid; // shared memory ID
    void *shmptr;
    struct shared_struct *shared; // structured shm
    pid_t childpid1, childpid2;
    char pathname[80], key_str[10], cmd_str[80];
    int shmsize, ret;

    shmsize = sizeof(struct shared_struct); //共享内存的大小
    
    // 在编译命令"./a.out"后面还要加上文件路径名
    if(argc <2) {
        printf("Usage: ./a.out pathname\n");
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
 
    key = ftok(pathname, 0x27); /* 0x27 a project ID 0x0001 - 0xffff, 8 least bits used */
    if(key == -1) {
        ERR_EXIT("shmcon: ftok()");
    }

    shmid = shmget((key_t)key, shmsize, 0666|PERM);
    if (shmid == -1) {
        printf("The shared memory size is %d, which is over the max limits\n", shmsize);
        ERR_EXIT("hread: shmget()");
    }

    shmptr = shmat(shmid, 0, 0);

    if(shmptr == (void *)-1) {
        ERR_EXIT("shread: shmat()");
    }
    
    shared = (struct shared_struct *)shmptr;
    shared->lock = 0;

        // detach the shared memory
    if (shmdt(shmptr) == -1) {
        ERR_EXIT("shmread: shmdt()");
    }
 
    printf("The shared memory size is %d, which is under the max limits\n", shmsize);
    exit(EXIT_SUCCESS);
}
