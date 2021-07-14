#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAXSIZE 1000 
#define MAX_TEST_TIME 30
/*栈*/
typedef struct Stack{
    int index[MAXSIZE];  //存储空闲坐标
    int top;     //栈顶
} Stack;

typedef struct Studentnode {
    int id;         //学号，结点信息
    char name[20];  //姓名，结点信息
    int pre;    //前一个结点的下标，结点为空则为-1
    int nex;   //后一个结点的下标，结点为空则为-1
    int flag;       //用作结点的删除标识,其中flag=0代表结点删除，flag=1代表结点未删除
} Student;


/*静态链表*/
typedef struct shared_struct {
    Student list[MAXSIZE];
    Stack nouse_index;     //静态链表中还未使用的下标
    int head;          //静态链表头节点在数组中的下标
    int size;               //结点数目
    int lock;		//进程锁
    int testtime;	//测试次数
} StaticLinkList;

/*在静态链表上实现的最小堆*/
typedef struct shared_struct * MinHeap;
/*栈结构的相关操作*/

//共享的结构体
#define PERM S_IRUSR|S_IWUSR|IPC_CREAT

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)
//退出代码

//初始化，备用结点使用栈的形式存储
void initiateStack(Stack *ustack) {
    //一开始所有结点都处于备用的状态
    for (int i = 0; i < MAXSIZE; ++i) {
        ustack->index[i] = MAXSIZE - i - 1;
    }
    ustack->top = MAXSIZE - 1;
}
//进栈
void pushStack(Stack *ustack, int unusedIndex) {
    ustack->index[++ustack->top] = unusedIndex;
}
//出栈
void popStack(Stack *ustack) {
    if (ustack->top == -1)
        return;
    ustack->top--;
}
 //取栈顶元素
int topStack(const Stack *ustack) {
    if (ustack->top == -1)
        return -1;
    return ustack->index[ustack->top];
}

/* 存储学生信息的结构体的相关操作*/

//初始化静态链表
void initialStaticLinkList(StaticLinkList *ustatic_list) {
    //静态链表开始元素在数组中的下标设为-1
    ustatic_list->head = -1;
    //静态链表中存储的学生信息数目为0
    ustatic_list->size = 0;
    //静态链表被操作的次数为0
    ustatic_list->testtime=0;
    initiateStack(&ustatic_list->nouse_index);
}

//加入静态链表
void insertListAtHead(StaticLinkList *ustatic_list, Student stu) {
    //在备用结点存放的栈中取出备用结点，采用头插法将学生插入到静态链表中
    int index = topStack(&ustatic_list->nouse_index);
    popStack(&ustatic_list->nouse_index);
    ustatic_list->list[index] = stu;
    ustatic_list->list[index].flag = 1;
    ustatic_list->list[index].pre = -1;
    ustatic_list->list[index].nex = ustatic_list->head;
    if (ustatic_list->head != -1) {
        ustatic_list->list[ustatic_list->head].pre = index;
    }
    ustatic_list->head = index;
    ustatic_list->size++;
}

/* 以学号为关键字的最小堆的操作 */

//交换最小堆中的两个结点，但是不改变它们在静态链表中所处的顺序
void swap(MinHeap h, int parent, int child) {
    Student t = h->list[parent];
    h->list[parent] = h->list[child];
    h->list[child] = t; 
}

//将未经过排序的堆结构重排变成堆
void rearrangeHeap(MinHeap h) {
    int n = h->size;
    for (int i = n / 2 - 1; i >= 0; --i) {
        Student temp = h->list[i];
        int parent = i;
        int child = (i << 1) + 1;
        while (child < n) {
            //选择儿子结点中较小的那个结点
            if (child + 1 < n && h->list[child].id > h->list[child + 1].id) {
                child = child + 1;
            }
            //当父节点大于子结点的时候要进行调整
            if (temp.id > h->list[child].id) {
                swap(h, parent, child);
                //及时更新头结点坐标
                if (h->list[parent].pre == -1) h->head = parent;
                if (h->list[child].pre == -1) h->head = child;
                parent = child;
                child = (parent << 1) + 1;
            } else
                break;
        }
    }
}

//向最小堆中添加学生信息
void pushHeap(MinHeap h, Student *stu) {
    int child = topStack(&h->nouse_index);
    popStack(&h->nouse_index);
    //向数组的空闲下标的位置插入数据
    h->list[child].id = stu->id;
    strcpy(h->list[child].name, stu->name);
    //对学生信息进行加工调整后再添加
    h->list[h->head].pre = child;
    h->list[child].flag = 1;
    h->list[child].pre = -1;
    h->list[child].nex = h->head;
    h->size++;
    int parent = (child - 1) / 2;
    //如果此时插入元素的学号比它的父亲大，同时插入数据还没有成为根结点，那么将它的父亲保存到小顶堆的位置，插入元素继续向上比较
    while (stu->id < h->list[parent].id && child > 0) {
        swap(h, parent, child);
        child = parent;
        parent = (child - 1) / 2;
    }
    h->head = child;
}

//从小顶堆中删除根结点的数据
void popHeap(MinHeap h) {
    //小顶堆为空，直接返回
    if (h->size == 0) return;
    int high, low;
    //获取小顶堆数组的最后一个元素
    Student temp = h->list[--h->size];
    pushStack(&h->nouse_index, h->size);
    h->list[h->head].flag = 0;
    if (h->size == 0) return;
    swap(h, 0, h->size);
    for (high = 0; high * 2 + 1 < h->size; high = low) {
        //从根结点开始，high为父亲结点的位置，low为左儿子的位置
        low = high * 2 + 1;
        //选择儿子结点中学号较小的那个结点
        if (low != h->size - 1 && h->list[low].id > h->list[low + 1].id) {
            low += 1;
        }
        //如果这个结点比小顶堆数组的最后一个元素的学号小，那么将这个结点放到它的父亲结点
        if (h->list[low].id < temp.id) {
            swap(h, high, low);
            if (h->list[high].pre == -1) h->head = high;
            if (h->list[low].pre == -1) h->head = low;
            break;
        }
    }
    //调整被删除结点前后结点的坐标相对顺序和表头结点
    int pre_index = h->list[h->size].pre;
    int nex_index = h->list[h->size].nex;
    if (pre_index != -1) h->list[pre_index].nex = nex_index;
    if (nex_index != -1) h->list[nex_index].pre = pre_index; 
    if (pre_index == -1) h->head = nex_index;
}

//从小顶堆返回最小元素
Student topHeap(MinHeap t) {
    return t->list[0];
}

//判断小顶堆是否为空
int isEmptyHeap(MinHeap t) {
    return t->size == 0;
}

//根据学生信息找到其在静态链表中的下标
int findHeap(MinHeap h, Student *stu, int curIndex) {
    if (curIndex >= h->size) return -1;
    if (h->list[curIndex].id == stu->id && strcmp(h->list[curIndex].name, stu->name) == 0) {
        return curIndex;
    }
    int l_index = -1;
    int r_index = -1;
    if (curIndex * 2 + 1 < h->size) {
        l_index = findHeap(h, stu, curIndex * 2 + 1);
    }
    if (l_index != -1) return l_index;
    if (curIndex * 2 + 2 < h->size) {
        r_index = findHeap(h, stu, curIndex * 2 + 2);
    }
    if (r_index != -1) return r_index;
    return -1;
}
//根据学生的学号找到其在静态链表中的下标
int findHeapID(MinHeap h, int id,int curIndex) {
    //当前角标不合法
    if (curIndex >= h->size||curIndex<0) return -1;
    //根节点即为要找的坐标位置
    if (h->list[curIndex].id == id ) {
        return curIndex;
    }
    int l_index = -1;
    int r_index = -1;
    //先找左子數
    if (curIndex * 2 + 1 < h->size) {
        l_index = findHeapID(h, id, curIndex * 2 + 1);
    }
    //左子數找到了就返回坐标
    if (l_index != -1) return l_index;
    //左子树找不到，再找右子树
    if (curIndex * 2 + 2 < h->size) {
        r_index = findHeapID(h, id, curIndex * 2 + 2);
    }
    //右子树找到了则返回坐标
    if (r_index != -1) return r_index;
    //都找不到返回-1
    return -1;
}


//修改学生姓名
void modifyName(MinHeap h, Student *stu, char *name) {
    int index = findHeap(h, stu, 0);
    if (index == -1) {
        printf("can't find the target student!\n");
    } else {
        strcpy(h->list[index].name, name);
    }
}

//修改学生学号
void modifyId(MinHeap h, Student *stu, int id) {
    int index = findHeap(h, stu, 0);
    if (index == -1) {
        printf("can't find the target student!\n");
    } else {
        h->list[index].id = id;
        rearrangeHeap(h);
    }
}



//把堆中的元素按照一维数组的顺序打印出来，并显示它们在一维数组中的下标
void printHeap(MinHeap h) {
    for(int i = 0; i < h->size; ++i) {
        printf("location info->(Index: %d  Pre: %d  Next: %d) \t student info->(Id: %d  Name: %s) \n",i, h->list[i].pre, h->list[i].nex, h->list[i].id, h->list[i].name);
    }
}
