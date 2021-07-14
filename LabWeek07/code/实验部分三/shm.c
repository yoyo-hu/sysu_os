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
    
    shared = (StaticLinkList *)shmptr; // 创建共享内存中使用的结构体 */
    initialStaticLinkList(shared); //初始化结构体
    shared->lock = 0; //逻辑值lock设为0，代表进程可以执行
    shared->testtime = 0; //对结构体的操作次数
	
    //方便测试，当结构体中的元素个数为5时，就退出
    srand((unsigned) time(NULL));

    while (shared->testtime < MAX_TEST_TIME) {
	shared->testtime++;
        //当逻辑值lock等于1时，进程休眠不执行
        while (shared->lock == 1) {
            sleep(1);
        }
        wait(0);
        //进程执行时，将逻辑值lock设为1，防止其它进程进入共享内存
        shared->lock = 1;
        //随机产生操作种类，包括对小顶堆的插入、删除、查找、修改
        int Operand = rand() % 4 + 1;
	//根据op的值选择操作种类，1为插入，2为删除，3为查找，4为修改
	switch (Operand) {
        case 1: {
	    printf("Operand Is Push:\n");
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

        case 3: {
            printf("Operand Is Find:\n");
            int temp_id = rand() % 1000;
            int index = findHeapID(shared, temp_id, 0);
            if (index == -1) {
                printf("There is no student whose student ID is %d\n", temp_id);
            }
            else {
                printf("find->student(ID=%d)'s index is %d\n", temp_id, index);
            }
            break;
        }

        case 4: {
            printf("Operand Is Modify:\n");
            if (shared->size == 0) {
                printf("The heap is empty\n");
            }
            else {
                int modifyIndex = rand() % shared->size;
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
		modifyId(shared, &shared->list[modifyIndex], temp_id);
                modifyName(shared, &shared->list[modifyIndex], name);
                printf("AfterModify->student information：(id: %d  name: %s) \n", shared->list[modifyIndex].id, shared->list[modifyIndex].name);
            }
            break;
                
        }
    }
        //打印共享结构，方便测试用
        printHeap(shared);
        //休眠4s，尽量避免多进程冲突，同时方便测试
        sleep(4);
        //将逻辑值设为0，其它进程可进入共享内存执行
        shared->lock = 0;
	printf("-----------------------------------------------\n\n");
    }


    printf("Reached the upper limit of the number of tests 40, the final heap information is as follows：\n");
    printHeap(shared);

    if(shmdt(shmptr) == -1) {
        ERR_EXIT("shmcon: shmdt()");
    }
    //休眠10秒等待所有进程退出
    sleep(10);
    //等所有进程退出就释放会报错
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        ERR_EXIT("shmcon: shmctl(IPC_RMID)");
    }

    exit(EXIT_SUCCESS);
}
