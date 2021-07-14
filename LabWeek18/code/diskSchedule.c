#include <math.h>
#include <stdio.h>
#include <stdlib.h>
// FCFS、SSTF、SCAN、C-SCAN、LOOK、C-LOOK
#define FCFS 1
#define SSTF 2
#define SCAN 3
#define C_SCAN 4
#define LOOK 5
#define C_LOOK 6
#define MAX_SIZE 100

int CylindNum;                //模拟硬盘的柱面个数
int CylindReqList[MAX_SIZE];  //请求访问的柱面序列
int CylindReqNum;             //请求访问的柱面个数
int cmp(const void* a, const void* b) { return *(int*)a - *(int*)b; }
/*先来先服务调度算法FCFS*/
void FCFS_alg() {
    int distance = 0;
    int j, i, curCylinder;
    printf("请输入当前的的柱面号: ");
    scanf("%d", &curCylinder);
    printf("依次经过的柱面为:\n");
    //打印依次经过的柱面号,curCylinder记录实时经过的柱面号
    for (i = 0; i < CylindReqNum; i++) {
        printf("%d ", curCylinder);
        distance += abs(curCylinder - CylindReqList[i]);  //移动一次的距离
        curCylinder = CylindReqList[i];
    }
    printf("%d ", curCylinder);
    printf("\n总寻道距离:%d\n", distance);
}
/*最短寻道时间优先调度算法SSTF*/
void SSTF_alg() {
    int curCylinder;
    int i, j;
    int distance = 0;
    //排序
    qsort(CylindReqList, CylindReqNum, sizeof(int), cmp);
    printf("请输入当前柱面号: ");
    scanf("%d", &curCylinder);
    printf("依次经过的柱面为:\n");
    int k = 0;  //记录恰好比初始结点大的最近的请求柱面下标
    while (k < CylindReqNum && CylindReqList[k] < curCylinder) {
        k++;
    }
    int l = k - 1;  //记录恰好比当前结点号小的最近的请求柱面
    int r = k;      //记录恰好比当前结点号大的最近的请求柱面
    //采用归并的方法，左右比较找到当前结点号最近的请求柱面
    while ((l >= 0) && (r < CylindReqNum)) {
        if ((curCylinder - CylindReqList[l]) <=
            (CylindReqList[r] - curCylinder)) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[l]);
            curCylinder = CylindReqList[l];
            l = l - 1;
        } else {
            printf("%d ", curCylinder);
            distance += (CylindReqList[r] - curCylinder);
            curCylinder = CylindReqList[r];
            r = r + 1;
        }
    }
    while (l >= 0) {
        printf("%d ", curCylinder);
        distance += (curCylinder - CylindReqList[l]);
        curCylinder = CylindReqList[l];
        l = l - 1;
    }
    while (r < CylindReqNum) {
        printf("%d ", curCylinder);
        distance += (CylindReqList[r] - curCylinder);
        curCylinder = CylindReqList[r];
        r = r + 1;
    }
    printf("%d ", curCylinder);
    printf("\n总寻道距离:%d\n", distance);
}
/*电梯算法一SCAN*/
void SCAN_alg() {
    int curCylinder, direction;
    int i, j, distance = 0;
    //排序
    qsort(CylindReqList, CylindReqNum, sizeof(int), cmp);
    printf("请输入当前柱面号: ");
    scanf("%d", &curCylinder);
    int k = 0;  //记录恰好比初始结点大的最近的请求柱面
    while (k < CylindReqNum && CylindReqList[k] < curCylinder) {
        k++;
    }
    int l = k - 1;  //记录恰好比初始结点小的最近的请求柱面
    int r = k;      //记录恰好比初始结点大的最近的请求柱面
    printf("请输入当前移动臂的移动方向（0表示向内,1表示向外）:");
    scanf("%d", &direction);
    printf("依次经过的柱面为:\n");
    if (direction == 0) {  //向内开始滑动
        //按照SCAN算法，一开始磁头向内滑动，一路上响应请求服务的柱面
        for (j = l; j >= 0; j--) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[j]);
            curCylinder = CylindReqList[j];
        }
        //磁头一直滑动到磁盘的头部即0处
        if (CylindReqList[0] != 0) {
            printf("%d ", curCylinder);
            distance += (curCylinder - 0);
            curCylinder = 0;
        }
        //磁头反向滑动，向外滑动，一路上服务请求的柱面，直到响应全部服务结束
        for (j = r; j < CylindReqNum; j++) {
            printf("%d ", curCylinder);
            distance += (CylindReqList[j] - curCylinder);
            curCylinder = CylindReqList[j];
        }
        printf("%d ", curCylinder);
    } else {  //向外开始滑动
        //按照SCAN算法，一开始磁头向外滑动，一路上响应请求服务的柱面
        for (j = r; j < CylindReqNum; j++) {
            printf("%d ", curCylinder);
            distance += (CylindReqList[j] - curCylinder);
            curCylinder = CylindReqList[j];
        }
        //磁头一直滑动到磁盘的尾部即CylindReqNum-1处
        if (CylindReqList[CylindReqNum - 1] != CylindNum - 1) {
            printf("%d ", curCylinder);
            distance += (CylindNum - 1 - curCylinder);
            curCylinder = CylindNum - 1;
        }
        //磁头反向滑动，向内滑动，一路上服务请求的柱面，直到响应全部服务结束
        for (j = l; j >= 0; j--) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[j]);
            curCylinder = CylindReqList[j];
        }
        printf("%d ", curCylinder);
    }
    printf("\n总寻道距离:%d\n", distance);
}
/*电梯算法二CSCAN*/
void CSCAN_alg() {
    int curCylinder, direction;
    int i, j, distance = 0;
    //排序
    qsort(CylindReqList, CylindReqNum, sizeof(int), cmp);
    printf("请输入当前柱面号: ");
    scanf("%d", &curCylinder);
    int k = 0;  //记录恰好比初始结点大的最近的请求柱面
    while (k < CylindReqNum && CylindReqList[k] < curCylinder) {
        k++;
    }
    int l = k - 1;  //记录恰好比初始结点小的最近的请求柱面
    int r = k;      //记录恰好比初始结点大的最近的请求柱面
    printf("请输入当前移动臂的移动方向（0表示向内，1表示向外):");
    scanf("%d", &direction);
    printf("依次经过的柱面为:\n");
    if (direction == 0) {  //向内开始滑动
        //按照C-SCAN算法，一开始磁头向内滑动，一路上响应请求服务的柱面
        for (j = l; j >= 0; j--) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[j]);
            curCylinder = CylindReqList[j];
        }
        //磁头一直滑动到磁盘的头部即0处
        if (CylindReqList[0] != 0) {
            printf("%d ", curCylinder);
            distance += (curCylinder - 0);
            curCylinder = 0;
        }
        //磁头反向滑动，向外滑动，一直滑动到磁盘的尾部即CylindReqNum-1处，一路上不响应请求的服务
        if (CylindReqList[CylindReqNum - 1] != curCylinder - 1) {
            printf("%d ", curCylinder);
            distance += (CylindNum - 1 - curCylinder);
            curCylinder = CylindNum - 1;
        }
        //磁头再次反向滑动，向内滑动，一路上服务请求的柱面，直到响应全部服务结束
        for (j = CylindReqNum - 1; j >= r; j--) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[j]);
            curCylinder = CylindReqList[j];
        }
        printf("%d ", curCylinder);
    } else {  //向外开始滑动
        //按照C-SCAN算法，一开始磁头向外滑动，一路上响应请求服务的柱面
        for (j = r; j < CylindReqNum; j++) {
            printf("%d ", curCylinder);
            distance += (CylindReqList[j] - curCylinder);
            curCylinder = CylindReqList[j];
        }
        //磁头一直滑动到磁盘的尾部即CylindReqNum-1处
        if (CylindReqList[CylindReqNum - 1] != curCylinder - 1) {
            printf("%d ", curCylinder);
            distance += (CylindNum - 1 - curCylinder);
            curCylinder = CylindNum - 1;
        }
        //磁头反向滑动，向内滑动，一直滑动到磁盘的头部即0处，一路上不响应请求的服务
        if (CylindReqList[0] != 0) {
            printf("%d ", curCylinder);
            distance += (curCylinder - 0);
            curCylinder = 0;
        }
        //磁头再次反向滑动，向外滑动，一路上服务请求的柱面，直到响应全部服务结束
        for (j = 0; j <= l; j++) {
            printf("%d ", curCylinder);
            distance += (CylindReqList[j] - curCylinder);
            curCylinder = CylindReqList[j];
        }
        printf("%d ", curCylinder);
    }
    printf("总寻道距离:%d\n", distance);
}
/*电梯算法三LOOK*/
void LOOK_alg() {
    int curCylinder, direction;
    int i, j, distance = 0;
    //排序
    qsort(CylindReqList, CylindReqNum, sizeof(int), cmp);
    printf("请输入当前柱面号: ");
    scanf("%d", &curCylinder);
    int k = 0;  //记录恰好比初始结点大的最近的请求柱面
    while (k < CylindReqNum && CylindReqList[k] < curCylinder) {
        k++;
    }
    int l = k - 1;  //记录恰好比初始结点小的最近的请求柱面
    int r = k;      //记录恰好比初始结点大的最近的请求柱面
    printf("请输入当前移动臂的移动方向（0表示向内,1表示向外）:");
    scanf("%d", &direction);
    printf("依次经过的柱面为:\n");
    if (direction == 0) {  //向内开始滑动
        //按照LOOK算法，一开始磁头向内滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求被响应
        for (j = l; j >= 0; j--) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[j]);
            curCylinder = CylindReqList[j];
        }
        //磁头反向滑动，向外滑动，一路上服务请求的柱面，直到响应全部服务结束
        for (j = r; j < CylindReqNum; j++) {
            printf("%d ", curCylinder);
            distance += (CylindReqList[j] - curCylinder);
            curCylinder = CylindReqList[j];
        }
        printf("%d ", curCylinder);
    } else {  //向外开始滑动
        //按照LOOK算法，一开始磁头向外滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求被响应
        for (j = r; j < CylindReqNum; j++) {
            printf("%d ", curCylinder);
            distance += (CylindReqList[j] - curCylinder);
            curCylinder = CylindReqList[j];
        }
        //磁头反向滑动，向内滑动，一路上服务请求的柱面，直到响应全部服务结束
        for (j = l; j >= 0; j--) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[j]);
            curCylinder = CylindReqList[j];
        }
        printf("%d ", curCylinder);
    }
    printf("总寻道距离:%d\n", distance);
}
/*电梯算法四CLOOK*/
void CLOOK_alg() {
    int curCylinder, direction;
    int i, j, distance = 0;
    //排序
    qsort(CylindReqList, CylindReqNum, sizeof(int), cmp);
    printf("请输入当前柱面号: ");
    scanf("%d", &curCylinder);
    int k = 0;  //记录恰好比初始结点大的最近的请求柱面
    while (k < CylindReqNum && CylindReqList[k] < curCylinder) {
        k++;
    }
    int l = k - 1;  //记录恰好比初始结点小的最近的请求柱面
    int r = k;      //记录恰好比初始结点大的最近的请求柱面
    printf("请输入当前移动臂的移动方向（0表示向内，1表示向外):");
    scanf("%d", &direction);
    printf("依次经过的柱面为:\n");
    if (direction == 0) {  //向内开始滑动
        //按照C-LOOK算法，一开始磁头向内滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求的位置
        for (j = l; j >= 0; j--) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[j]);
            curCylinder = CylindReqList[j];
        }
        //磁头反向滑动，向外滑动，一直滑动到该方向上的最后一个请求的位置，一路上不响应请求的服务
        if (r <= CylindReqNum - 1) {
            printf("%d ", curCylinder);
            distance += (CylindReqList[CylindReqNum - 1] - curCylinder);
            curCylinder = CylindReqList[CylindReqNum - 1];
        }
        //磁头再次反向滑动，向内滑动，一路上响应请求的服务，直到响应全部服务结束
        for (j = CylindReqNum - 2; j >= r; j--) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[j]);
            curCylinder = CylindReqList[j];
        }
        printf("%d ", curCylinder);
    } else {  //向外开始滑动
        //按照C-LOOK算法，一开始磁头向外滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求的位置
        for (j = r; j < CylindReqNum; j++) {
            printf("%d ", curCylinder);
            distance += (CylindReqList[j] - curCylinder);
            curCylinder = CylindReqList[j];
        }
        //磁头反向滑动，向内滑动，一直滑动到该方向上的最后一个请求的位置，一路上不响应请求的服务
        if (l >= 0) {
            printf("%d ", curCylinder);
            distance += (curCylinder - CylindReqList[0]);
            curCylinder = CylindReqList[0];
        }
        //磁头再次反向滑动，向外滑动，一路上响应请求的服务，直到响应全部服务结束
        for (j = 1; j <= l; j++) {
            printf("%d ", curCylinder);
            distance += (CylindReqList[j] - curCylinder);
            curCylinder = CylindReqList[j];
        }
        printf("%d ", curCylinder);
    }
    printf("总寻道距离:%d\n", distance);
}
void main() {
    //输入初始信息
    int strategy;
    int i;
    printf("请输入模拟硬盘的柱面个数: ");
    scanf("%d", &CylindNum);
    printf("请输入请求到达的柱面个数: ");
    scanf("%d", &CylindReqNum);
    printf("请输入请求到达的柱面序列: ");
    for (i = 0; i < CylindReqNum; i++) {
        scanf("%d", &CylindReqList[i]);
    }
    printf("1.FCFS\n2.SSTF\n3.SCAN\n4.C_SCAN\n5.LOOK\n6.C_LOOK\n");
    printf("请选择硬盘柱面访问调度算法:\n");
    scanf("%d", &strategy);
    if (strategy > 6 || strategy < 1) {  //策略选择不合法
        printf("illegal strategy!");
        return;
    }
    //选择柱面访问调度算法
    switch (strategy) {
            // FCFS、SSTF、SCAN、C_SCAN、LOOK、C_LOOK
        case FCFS:
            FCFS_alg();
            break;
        case SSTF:
            SSTF_alg();
            break;
        case SCAN:
            SCAN_alg();
            break;
        case C_SCAN:
            CSCAN_alg();
            break;
        case LOOK:
            LOOK_alg();
            break;
        case C_LOOK:
            CLOOK_alg();
            break;
        default:
            break;
    }
}