/*启动程序，程序要求把shmread.c程序编译成可执行
文件shmread.o，程序要求把shmwrite.c程序编译成可
执行文件shmwrite.o*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>

#include "shmdata.h"

int main(int argc, char *argv[])
{
    struct stat fileattr;
    key_t key; /* of type int */
    int shmid; /* shared memory ID *///进程私有
    void *shmptr;
    Queue *q; /* structured shm *///用户定义
    pid_t childpid1, childpid2;
    char pathname[80], key_str[10], cmd_str[80];
    int shmsize, ret;

    shmsize = TEXT_NUM*sizeof(struct shared_struct);//共享区域大小
    printf("max record number = %d, shm size = %d\n", TEXT_NUM, shmsize);

    if(argc <2) {
        printf("Usage: ./a.out pathname\n");
        return EXIT_FAILURE;
    }
    strcpy(pathname, argv[1]);
    //如果没有共享文件就创建
    if(stat(pathname, &fileattr) == -1) {
        //stat()用来将参数file_name 所指的文件状态, 复制到参数buf 所指的结构中。
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
    printf("key generated: IPC key = %x\n", key); /* can set any nonzero key without ftok()*/
    //创建共享内存
    shmid = shmget((key_t)key, shmsize, 0666|PERM);//0666是给全面的读写权限
    //shmid:共享内存标识符
    if(shmid == -1) {
        ERR_EXIT("shmcon: shmget()");
    }
    printf("shmcon: shmid = %d\n", shmid);

    shmptr = shmat(shmid, 0, 0); /* returns the virtual base address mapping to the shared memory, *shmaddr=0 decided by kernel */

    if(shmptr == (void *)-1) {
        ERR_EXIT("shmcon: shmat()");
    }
    printf("shmcon: shared Memory attached at %p\n", shmptr);
    //进行强制类型转换，防止编译器发出warming
   q  = (Queue*)shmptr;
   q->writelock = 1;

    sprintf(cmd_str, "ipcs -m | grep '%d'\n", shmid); 
    printf("\n------ Shared Memory Segments ------\n");
    system(cmd_str);
	
    if(shmdt(shmptr) == -1) {
        ERR_EXIT("shmcon: shmdt()");
    }

    printf("\n------ Shared Memory Segments ------\n");
    system(cmd_str);

    sprintf(key_str, "%x", key);
    char *argv1[] = {" ", key_str, 0};
    //生成子进程
    childpid1 = vfork();
    if(childpid1 < 0) {//错误
        ERR_EXIT("shmcon: 1st vfork()");
    } 
    else if(childpid1 == 0) {//进入子进程
        //执行外部的可进行代码
        execv("./shmread.o", argv1); /* call shm_read with IPC key */
    }
    else {
        //创建子进程
        childpid2 = vfork();
        if(childpid2 < 0) {
            ERR_EXIT("shmcon: 2nd vfork()");
        }
        else if (childpid2 == 0) {//进入子进程
        //执行外部的可进行代码
            execv("./shmwrite.o", argv1); /* call shmwrite with IPC key */
        }
        else {
            wait(&childpid1);
            wait(&childpid2);
                 /* shmid can be removed by any process knewn the IPC key */
            //删除共享内存段
            if (shmctl(shmid, IPC_RMID, 0) == -1) {
                ERR_EXIT("shmcon: shmctl(IPC_RMID)");
            }
            else {
                printf("shmcon: shmid = %d removed \n", shmid);
                printf("\n------ Shared Memory Segments ------\n");
                system(cmd_str);
                printf("nothing found ...\n"); 
            }
        }
    }
    exit(EXIT_SUCCESS);
}

