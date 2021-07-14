#include <stdio.h>
#include <stdlib.h>
#define MAX_PHY_BLOCK_NUM 100
#define MAX_PAGE_VISIT_NUM 1000
#define FIFO 1
#define LRU_STACK 2
#define LRU_MATRIX 3
#define SECOND_CHANCE 4

int phyBlockNum;
int pageNum;
int memList[100];       //内存队列
int pageToVisit[1000];  //保存要访问的页面号

/*按照课件给的形式打印内存块信息*/
void printList() {
    for (int i = 0; i < phyBlockNum; i++) {
        if (memList[i] == -1) {
            printf("| |\n");
        } else
            printf("|%d|\n", memList[i]);
    }
    printf("\n");
}

/*先进先出置换算法FIFO*/
void replaceAlgFIFO() {
    int pointer = 0;  //指向下一个页表号应该存放的地方
    int isNeedReplace =
        1;  //是否需要进行页面置换，如果内存中没有该页表号且内存已满，则需要，为1
    for (int i = 0; i < pageNum; i++) {
        isNeedReplace = 1;
        //判断是否需要置换，内存已满且该页面不在内存中，则需要
        for (int j = 0; j < phyBlockNum; j++) {
            if (memList[j] == pageToVisit[i]) {
                isNeedReplace = 0;  //该页面在内存中,不需要进行页面置换
                printf("%d hit\n", pageToVisit[i]);  //命中
                break;
            }
            if (memList[j] == -1) {  //页面不在内存中且内存未满
                memList[j] = pageToVisit[i];
                isNeedReplace = 0;  //内存未满,不需要进行页面置换
                printf("%d miss\n", pageToVisit[i]);  //缺失
                printList();
                break;
            }
        }
        if (isNeedReplace) {  //需要进行页面置换
            //如果pointer指向最后一个之后的区域，则归零
            if (pointer > phyBlockNum - 1) pointer = 0;

            memList[pointer] =
                pageToVisit[i];  //按照FIFO的原则替代pointer所指的节点
            pointer++;

            printf("%d miss\n", pageToVisit[i]);  //缺失
            printList();
        }
    }
}

/*最近最久未使用置换算法LRU-stack实现*/
void replaceAlgStackLRU() {
    int isNeedReplace =
        1;  //是否需要进行页面置换，如果内存中没有该页表号且内存已满，则需要，为1
    int count = 0;  //在内存中的页面数量
    for (int i = 0; i < pageNum; i++) {
        isNeedReplace = 1;
        //判断是否需要置换，内存已满且该页面不在内存中，则需要
        for (int j = 0; j < phyBlockNum; j++) {
            if (memList[j] == pageToVisit[i]) {
                //已经存在于内存中，把它换到队列（栈）尾部
                int temp = memList[j];
                for (int k = j; k < count - 1; k++) {
                    memList[k] = memList[k + 1];
                }
                memList[count - 1] = temp;

                isNeedReplace = 0;  //该页面在内存中,不需要进行页面置换
                printf("%d hit\n", pageToVisit[i]);  //命中
                printList();
                break;
            }

            if (memList[j] == -1) {  //页面不在内存中且内存未满
                memList[j] = pageToVisit[i];
                isNeedReplace = 0;  //内存未满,不需要进行页面置换
                count++;
                printf("%d miss\n", pageToVisit[i]);
                printList();
                break;
            }
        }

        if (isNeedReplace) {  //需要进行页面置换
            //将数组整体往前移一位
            for (int k = 0; k < phyBlockNum; k++) {
                memList[k] = memList[k + 1];
            }
            //将当前页面加到队尾
            memList[phyBlockNum - 1] = pageToVisit[i];
            printf("%d miss\n", pageToVisit[i]);
            printList();
        }
    }
}
//最近最久未使用置换算法LRU-矩阵实现
void replaceAlgMatrixLRU() {
    //矩阵，用来记录节点最近最久未被使用的程度大小
    int Matrix[pageNum][pageNum];
    //初始化矩阵
    for (int i = 0; i < pageNum; i++) {
        for (int j = 0; j < pageNum; j++) Matrix[i][j] = 0;
    }
    int isNeedReplace =
        1;  //是否需要进行页面置换，如果内存中没有该页表号且内存已满，则需要，为1

    int count = 0;  //在内存中的页面数量
    for (int i = 0; i < pageNum; i++) {
        isNeedReplace = 1;

        // pageToVisit[i]行置1，pageToVisit[i]列置0
        for (int p = 0; p < pageNum; p++) {
            Matrix[pageToVisit[i]][p] = 1;
        }
        for (int p = 0; p < pageNum; p++) {
            Matrix[p][pageToVisit[i]] = 0;
        }

        //判断是否需要置换，内存已满且该页面不在内存中，则需要
        for (int j = 0; j < phyBlockNum; j++) {
            if (memList[j] == pageToVisit[i]) {
                isNeedReplace = 0;  //该页面在内存中,不需要进行页面置换
                printf("%d hit\n", pageToVisit[i]);  //命中
                break;
            }
            if (memList[j] == -1) {  //页面不在内存中且内存未满
                memList[j] = pageToVisit[i];
                count++;            //在内存中的页面数量+1
                isNeedReplace = 0;  //内存未满,不需要进行页面置换

                printf("%d miss\n", pageToVisit[i]);  //缺失
                printList();
                break;
            }
        }

        if (isNeedReplace) {  //需要进行页面置换
            //遍历矩阵中在内存快中存在的页面的行，寻找行的零数最多的即为最久未访问的页面所在的内存下标
            int max =
                0;  //记录内存块中的页面对应到矩阵中的行的零数总和最大的节点
            int sum
                [phyBlockNum];  //记录内存块中的页面对应到矩阵中的行的零数总和
            //找出max节点，即最近最久未使用的页面
            for (int k = 0; k < phyBlockNum; k++) {
                sum[k] = 0;
                for (int p = 0; p < pageNum; p++) {
                    if (Matrix[memList[k]][p] == 0) sum[k] += 1;
                }
                if (sum[k] > sum[max]) {
                    max = k;
                }
            }

            memList[max] = pageToVisit[i];  //替换掉最近最久未使用的页面

            printf("%d miss\n", pageToVisit[i]);  //缺失
            printList();
        }
    }
}
/*二次机会算法SCR*/
void replaceAlgSecondchance() {
    //记录内存中的页表是否有不被替换出去的机会
    int secondChance[phyBlockNum];
    //初始化
    for (int i = 0; i < phyBlockNum; i++) {
        secondChance[i] = 0;
    }

    int isNeedReplace =
        0;  //是否需要进行页面置换，如果内存中没有该页表号且内存已满，则需要，为1
    int pointer = 0;  //指向下一个页表号应该存放的地方
    int count = 0;    //在内存中的页面数量
    for (int i = 0; i < pageNum; i++) {
        isNeedReplace = 1;
        //判断是否需要置换，内存已满且该页面不在内存中，则需要
        for (int j = 0; j < phyBlockNum; j++) {
            if (memList[j] == pageToVisit[i]) {
                isNeedReplace = 0;  //该页面在内存中,不需要进行页面置换
                //设置被访问过的内存中的页表有不被替换出去的机会
                secondChance[j] = 1;

                printf("%d hit\n", pageToVisit[i]);
                break;
            }
            if (memList[j] == -1) {  //页面不在内存中且内存未满
                memList[j] = pageToVisit[i];
                count++;
                secondChance[j] = 1;  //首先被加入的位
                isNeedReplace = 0;    //内存未满,不需要进行页面置换

                printf("%d miss\n", pageToVisit[i]);
                printList();
                break;
            }
        }
        if (isNeedReplace) {  //需要进行页面置换
            //如果point指向的节点在这一轮有不被替换出去的机会，则point++，判断下一个是否需要被替换出去
            while (secondChance[pointer] == 1) {
                secondChance[pointer] = 0;
                pointer++;
                if (pointer > phyBlockNum - 1) {
                    pointer = 0;
                }
            }

            if (pointer > phyBlockNum - 1) {
                pointer = 0;
            }
            memList[pointer] = pageToVisit[i];
            pointer++;

            printf("%d miss\n", pageToVisit[i]);
            printList();
        }
    }
}

int main(int argc, const char* argv[]) {
    int strategy;
    printf(
        "1.FIFO\n2.LRU(stack implementation)\n3.LRU(matrix "
        "implementation)\n4.Secondchance\n");
    printf("请选择所需的置换算法:\n");
    scanf("%d", &strategy);

    printf("请输入物理块数量:\n");
    scanf("%d", &phyBlockNum);
    // phyBlockNum是否合法
    if (phyBlockNum <= 0 || phyBlockNum > MAX_PHY_BLOCK_NUM) {
        printf("illegal phyBlockNum");
        return 0;
    }

    printf("请输入准备访问的页面总数:\n");
    scanf("%d", &pageNum);
    // pageNum是否合法
    if (pageNum <= 0 || pageNum > MAX_PAGE_VISIT_NUM) {
        printf("illegal pageNum");
        return 0;
    }
    //初始化内存队列为-1
    for (int i = 0; i < phyBlockNum; i++) {
        memList[i] = -1;
    }

    printf("请输入要访问的页面号:\n");
    for (int i = 0; i < pageNum; i++) {
        scanf("%d", &pageToVisit[i]);
    }

    //选择页表置换算法
    switch (strategy) {
        case FIFO:
            replaceAlgFIFO();
            break;
        case LRU_STACK:
            replaceAlgStackLRU();
            break;
        case LRU_MATRIX:
            replaceAlgMatrixLRU();
            break;
        case SECOND_CHANCE:
            replaceAlgSecondchance();
            break;
        default:
            break;
    }
    return 0;
}
