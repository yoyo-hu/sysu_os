# Lab Week 17.

## 1 实验内容：虚拟存储管理。

编写一个 C程序模拟实现课件Lecture24中的请求页面置换算法,包括FIFO、LRU (stack and matrix implementation)、Secondchance，并设计输入用例验证结果。

## 2 程序所涉及到的算法介绍以及相关实现函数

在计算机操作系统使用分页的虚拟内存管理，页面置换算法决定哪些内存页页出，有时也被称为换出，或写入到磁盘上，当一个页面被分配的内存需求。当请求的页面不在内存中（页面错误）并且空闲页面无法用于满足分配时，就会发生页面替换，要么是因为没有页面，要么是因为空闲页面的数量低于某个阈值。

### 2.1 先进先出(FIFO)算法

#### 2.1.1 介绍：

1. 将页面内存视为循环缓冲区。
2. 当缓冲区满时，替换最旧的页面,即先进先出。

#### 2.1.2 程序中该算法的实现思路如下：

使用`isNeedReplace`来标记是否需要进行置换,使用`pointer`来指向下一个页表号应该存放的地方，
对于想要访问的每一个页面，首先先判断是否需要进行置换，通过依次遍历内存数组，

1. 如果该内存数组中存在页面则标记`isNeedReplace`为0，表示不需要进行置换，并打印该页面命中并跳出循环
2. 如果该内存数组中不存在该页面且内存数组中存在空位，则将页表插入到空位中，同样标记`isNeedReplace`为0，表示不需要进行置换，接着打印该页面缺失并跳出循环。

接着通过判断`isNeedReplace`是否为1来判断是否需要进行页面的替换，为1则表示该页表没有存在于内存中且内存中没有空位使其直接插入，页面替代队列的头结点，即`pointer`所指向的点，`pointer`++,因为程序使用的是循环队列，因此当pointer到达最后一个队列位置之后时，需要重新设置其指向队列的第一个位置，接着打印该页面缺失。

#### 2.1.3 相关代码以及详细注释如下：

```c
/*先进先出置换算法FIFO*/
void replaceAlgFIFO(){
    int pointer = 0;//指向下一个页表号应该存放的地方
    int isNeedReplace = 1;//是否需要进行页面置换，如果内存中没有该页表号且内存已满，则需要，为1
    for (int i = 0; i < pageNum; i ++) {
    isNeedReplace = 1;
    //判断是否需要置换，内存已满且该页面不在内存中，则需要
        for (int j = 0; j < phyBlockNum; j ++) {
            if (memList[j] == pageToVisit[i]) {
                isNeedReplace = 0;//该页面在内存中,不需要进行页面置换
                printf("%d hit\n",pageToVisit[i]);//命中
                break;
               }
            if (memList[j] == -1) {//页面不在内存中且内存未满
                memList[j] = pageToVisit[i];
                isNeedReplace = 0;//内存未满,不需要进行页面置换
                printf("%d miss\n",pageToVisit[i]);//缺失
                printList();
                break;
            }
        }
        if (isNeedReplace) {//需要进行页面置换
            //如果pointer指向最后一个之后的区域，则归零
            if (pointer > phyBlockNum-1) pointer = 0;

            memList[pointer] = pageToVisit[i];//按照FIFO的原则替代pointer所指的节点
            pointer ++;

            printf("%d miss\n",pageToVisit[i]);//缺失
            printList();
        }
    }
}
```

### 2.2 LRU(stack实现）

#### 2.2.1 介绍：

1. 如果要访问的页面已在内存中，则直接通过移动将该也面放在循环队列或者栈的最后，表示此时最难被替换出去。
2. 当页面出现替换时，则将队列或者栈顶的头元素弹出，其后的元素依次往前移动，将需要插入的页面插入到数组的最后。

#### 2.2.2 程序中该算法的实现思路如下：

使用`isNeedReplace`来标记是否需要进行置换,使用`count`来表示当前在内存中的页面数量，
对于想要访问的每一个页面，首先先判断是否需要进行置换，通过依次遍历内存数组，

1. 如果该内存数组中存在该页面，则将其后的元素往前推一个单位，然后将该页面放置在栈底（队列的底部），并标记`isNeedReplace`为0，表示不需要进行置换，最后打印该页面命中并跳出循环
2. 如果该内存数组中不存在该页面且内存数组中存在空位，则将页表插入到空位中，同样标记`isNeedReplace`为0，表示不需要进行置换，接着将count+1，表示当前在内存中的页面数量增加一个，最后打印该页面缺失并跳出循环。

接着通过判断`isNeedReplace`是否为1来判断是否需要进行页面的替换，为1则表示该页表没有存在于内存中且内存中没有空位使其直接插入，将数组整体往前移一位，过程中队列队头（栈头）弹出，在队列的结尾加上该页面作为新节点，接着打印该页面缺失。

#### 2.2.3 相关代码以及详细注释如下：

```c
/*最近最久未使用置换算法LRU-stack实现*/
void replaceAlgStackLRU(){

   int isNeedReplace = 1;//是否需要进行页面置换，如果内存中没有该页表号且内存已满，则需要，为1
   int count = 0;//在内存中的页面数量
   for (int i = 0; i < pageNum; i ++) {
       isNeedReplace = 1;
       //判断是否需要置换，内存已满且该页面不在内存中，则需要
       for (int j = 0; j < phyBlockNum; j ++) {
           if (memList[j] == pageToVisit[i]) {
               //已经存在于内存中，把它换到队列尾部
               int temp = memList[j];
               for (int k = j; k < count-1; k ++) {
                   memList[k] = memList[k+1];
               }
               memList[count-1] = temp;

               isNeedReplace = 0;//该页面在内存中,不需要进行页面置换
               printf("%d hit\n",pageToVisit[i]);//命中
               printList();
               break;
           }

           if (memList[j] == -1) {//页面不在内存中且内存未满
               memList[j] = pageToVisit[i];
               isNeedReplace = 0;//内存未满,不需要进行页面置换
               count ++;
               printf("%d miss\n",pageToVisit[i]);
               printList();
               break;
           }
       }

       if (isNeedReplace) {//需要进行页面置换
           //将数组整体往前移一位
           for (int k = 0; k < phyBlockNum; k ++) {
               memList[k] = memList[k+1];
           }
           //将当前页面加到队尾
           memList[phyBlockNum-1] = pageToVisit[i];
           printf("%d miss\n",pageToVisit[i]);
           printList();
       }
   }
}
```

### 2.3 LRU(matrix）

#### 2.3.1 介绍：

用矩阵的方法来实现LRU算法的思想是使用矩阵来记录页面使用的频率和时间。设矩阵是n×n维的,n是相关程序当前驻内存的页面数。矩阵的初值为0,每次访问一个页面,例如第i个虛拟页被访问时,可对矩阵进行如下操作：

1. 将第i行的值全部置1;
2. 将第i列的值全部置0;

在每次需要更换页面时,选择矩阵里对应行值最小的页面淘汰
,该页面及时最近最少使用的页面。原理如下：

* 当一个页面被访问时,该页面所对应的行值将被置1，这样就保证了该页面对应的行值为最大之一,随后将该页面的对应列值置0,以保证该页面对应的行值为唯一最大。每次访问都将某一列置0,长时间没有被访问的页面,所对应的行元素里面被置0的列个数就越多,即它对应的行值就越小。因此,用矩阵的方法可以实现接近理想算法的页面置换。

#### 2.3.2 程序中该算法的实现思路如下：

使用`Matrix`来记录页面使用的频率和时间，其功能在上述介绍中已经详细介绍，先对其中的全部元素初始化为0，使用`isNeedReplace`来标记是否需要进行置换,使用`count`来表示当前在内存中的页面数量，对于想要访问的每一个页面，首先该页面所对应的行值将被置1，这样就保证了该页面对应的行值为最大之一,随后将该页面的对应列值置0,以保证该页面对应的行值为唯一最大。
然后再判断是否需要进行置换，通过依次遍历内存数组，

1. 如果该内存数组中存在该页面，则标记`isNeedReplace`为0，表示不需要进行置换，然后打印该页面命中并跳出循环
2. 如果该内存数组中不存在该页面且内存数组中存在空位，则将页表插入到空位中，同样标记`isNeedReplace`为0，表示不需要进行置换，接着将count+1，表示当前在内存中的页面数量增加一个，最后打印该页面缺失并跳出循环。

接着通过判断`isNeedReplace`是否为1来判断是否需要进行页面的替换，为1则表示该页表没有存在于内存中且内存中没有空位使其直接插入，遍历矩阵中在内存快中存在的页面的行，寻找行的零数最多的即为最久未访问的页面所在的内存下标，将下标对应的队列中的节点替换成该页面，接着打印该页面缺失。

#### 2.3.3 相关代码以及详细注释如下：

```c
//最近最久未使用置换算法LRU-矩阵实现
void replaceAlgMatrixLRU(){

    //矩阵，用来记录节点最近最久未被使用的程度大小
    int Matrix[pageNum][pageNum];
    //初始化矩阵
    for(int i=0;i<pageNum;i++){
        for(int j=0;j<pageNum;j++)
            Matrix[i][j]=0;
    }
    int isNeedReplace = 1;//是否需要进行页面置换，如果内存中没有该页表号且内存已满，则需要，为1

    int count = 0;//在内存中的页面数量
    for (int i = 0; i < pageNum; i ++) {
        isNeedReplace = 1;
        
        //pageToVisit[i]行置1，pageToVisit[i]列置0
        for (int p = 0; p < pageNum; p ++) {
            Matrix[pageToVisit[i]][p]=1;
        }
        for (int p = 0; p < pageNum; p ++) {
            Matrix[p][pageToVisit[i]]=0;
        }

        //判断是否需要置换，内存已满且该页面不在内存中，则需要
        for (int j = 0; j < phyBlockNum; j ++) {
            if (memList[j] == pageToVisit[i]) {
                isNeedReplace = 0;//该页面在内存中,不需要进行页面置换
                printf("%d hit\n",pageToVisit[i]);//命中
                break;
            }
            if (memList[j] == -1) {//页面不在内存中且内存未满
                memList[j] = pageToVisit[i];
                count ++;//在内存中的页面数量+1
                isNeedReplace = 0;//内存未满,不需要进行页面置换
                
                printf("%d miss\n",pageToVisit[i]);//缺失
                printList();
                break;
            }
        }

        if (isNeedReplace) {//需要进行页面置换
            //遍历矩阵中在内存快中存在的页面的行，寻找行的零数最多的即为最久未访问的页面所在的内存下标
            int max = 0;//记录内存块中的页面对应到矩阵中的行的零数总和最大的节点
            int sum[phyBlockNum];//记录内存块中的页面对应到矩阵中的行的零数总和
            //找出max节点，即最近最久未使用的页面
            for (int k = 0; k < phyBlockNum; k ++) {
                sum[k]=0;
                for(int p=0;p<pageNum;p++){
                    if(Matrix[memList[k]][p]==0)sum[k]+=1;
                }
                if(sum[k]>sum[max]){
                    max=k;
                }
            }

            memList[max] = pageToVisit[i];//替换掉最近最久未使用的页面

            printf("%d miss\n",pageToVisit[i]);//缺失
            printList();
            
        }
    }
}

```

### 2.4 二次机会算法Secondchance

#### 2.4.1 介绍：

该算法仍然使用标准的FIFO队列。
每个帧(frame)有一个second chance位，也叫做引用位。

* 当一个frame被引用到，它的second chance位设置为1。这表示该frame后面还有可能会被引用到，所以下次置换先跳过这个frame，也就是再给它一次机会留在内存中。这样可以减少frame置换，提高页面操作效率。
* 当一个新的页面被读到内存中时，它的second chance被设置为0。（页面首先加载到框架中会设置为1）
* 当需要替换内存中的一个页面时，使用轮询的方式来查找可以被替换的页面：
  如果页面的second chance是1，那么置为0，继续查找；
  如果页面的second chance是0，那么将这个页面置换出去。

如果一个页面被频繁使用以保持其引用
位设置，它永远不会被替换。
#### 2.4.2 程序中该算法的实现思路如下：

使用`secondChance`数组来记录内存中的页表是否有不被替换出去的机会，其功能在上述介绍中已经详细介绍，先对其中的全部元素初始化为0，使用`isNeedReplace`来标记是否需要进行置换,使用`pointer`来指向下一个页表号应该存放的地方，使用`count`来表示当前在内存中的页面数量。
对于想要访问的每一个页面，首先判断是否需要进行置换，通过依次遍历内存数组，

1. 如果该内存数组中存在该页面，则标记`isNeedReplace`为0，表示不需要进行置换，并设置该页表的secondChance为1，表示该页表在下一次将要被替换时，有不被替换出去的机会，然后打印该页面命中并跳出循环
2. 如果该内存数组中不存在该页面且内存数组中存在空位，则将页表插入到空位中，同样标记`isNeedReplace`为0，表示不需要进行置换，接着将count+1，表示当前在内存中的页面数量增加一个，最后打印该页面缺失并跳出循环。

接着通过判断`isNeedReplace`是否为1来判断是否需要进行页面的替换，为1则表示该页表没有存在于内存中且内存中没有空位使其直接插入，先判断当前要替换的页面（pointer指向的页面）是否有不被替换出去的机会，即其对应的secondChange的值是否为一，如果为1则修改其为0并且将point加一，循环这个过程知道找到secondChange不为1的节点，将该节点替换成访问的页面，接着打印该页面缺失。

#### 2.4.3 相关代码以及详细注释如下：

```c
/*二次机会算法SCR*/
void replaceAlgSecondchance(){
    //记录内存中的页表是否有不被替换出去的机会
    int secondChance[phyBlockNum];
    //初始化
    for(int i=0;i<phyBlockNum;i++){
        secondChance[i]=0;
    }

    int isNeedReplace = 0;//是否需要进行页面置换，如果内存中没有该页表号且内存已满，则需要，为1
    int pointer=0;//指向下一个页表号应该存放的地方
    int count = 0;//在内存中的页面数量
    for (int i = 0; i < pageNum; i ++) {
        isNeedReplace = 1;
        //判断是否需要置换，内存已满且该页面不在内存中，则需要
        for (int j = 0; j < phyBlockNum; j ++) {
            if (memList[j] == pageToVisit[i]) {
                isNeedReplace = 0;//该页面在内存中,不需要进行页面置换
                //设置被访问过的内存中的页表有不被替换出去的机会
                secondChance[j] = 1;
                
                printf("%d hit\n",pageToVisit[i]);
                break;
            }
            if (memList[j] == -1) {//页面不在内存中且内存未满
                memList[j] = pageToVisit[i];
                count ++;
				secondChance[j] = 1;//首先被加入的位
                isNeedReplace = 0;//内存未满,不需要进行页面置换
                
                printf("%d miss\n",pageToVisit[i]);
                printList();
                break;
            }
        }
        if (isNeedReplace) {//需要进行页面置换
        //如果point指向的节点在这一轮有不被替换出去的机会，则point++，判断下一个是否需要被替换出去
            while(secondChance[pointer]==1){
                secondChance[pointer]=0;
                pointer++;
                if (pointer > phyBlockNum-1) {
                    pointer = 0;
                }
            }

            if (pointer > phyBlockNum-1) {
                pointer = 0;
            }
            memList[pointer] = pageToVisit[i];
            pointer ++;
            
            printf("%d miss\n",pageToVisit[i]);
            printList();
        }
    }
}

```

## 3 主函数介绍：

程序一开始要求输入置换算法类型`strategy`，物理块数量`phyBlockNum`,准备访问的页面总数`pageNum`,然后输入`pageNum`数量的待访问的页面号，再根据`strategy`选择进入置换算法函数（在上述有提到）进行依次的置换。

## 4 运行结果分析。
### 4.1 测试FIFO算法
按照要求选择FIFO算法，使用课件上的样例，选择3块物理块，输入20次要访问页面的次数，并依次输入每一次要访问的页面编号，如下图所示：

![1.png](https://z3.ax1x.com/2021/06/14/2TYlm4.png)

执行结果如下：

![2.png](https://z3.ax1x.com/2021/06/14/2TYMXF.png) ![3.png](https://z3.ax1x.com/2021/06/14/2TYulT.png) ![4.png](https://z3.ax1x.com/2021/06/14/2TYK6U.png)

和课本中的结果一致，如下，都满足先进先出的原则：

![5.png](https://z3.ax1x.com/2021/06/14/2TYnpV.png)

### 4.2 测试LRU（stack) 算法
按照要求选择LRU（stack)算法，使用课件上的样例，选择5块物理块，输入13次要访问页面的次数，并依次输入每一次要访问的页面编号，如下图所示：

![6.png](https://z3.ax1x.com/2021/06/14/2TY379.png)

执行结果如下：

![7.png](https://z3.ax1x.com/2021/06/14/2TY10J.png) ![8.png](https://z3.ax1x.com/2021/06/14/2TYGkR.png) ![9.png](https://z3.ax1x.com/2021/06/14/2TYJt1.png) ![10.png](https://z3.ax1x.com/2021/06/14/2TYYfx.png)

和课本中的结果基本一致（程序将下面作为栈的头部或队列的尾部，而课件中将上面作为栈的头部或队列的尾部），都满足LRU的原则：

![11.png](https://z3.ax1x.com/2021/06/14/2TYNp6.png)

### 4.3 测试LRU（matrix) 算法
按照要求选择LRU（matrix)算法，使用课件上的样例，选择3块物理块，输入12次要访问页面的次数，并依次输入每一次要访问的页面编号，如下图所示：
![12.png](https://z3.ax1x.com/2021/06/14/2TYa6O.png)

执行结果如下：

![13.png](https://z3.ax1x.com/2021/06/14/2TYU1K.png) ![14.png](https://z3.ax1x.com/2021/06/14/2TYdXD.png)

和课本中的结果一致，都满足LRU的原则：

![15.png](https://z3.ax1x.com/2021/06/14/2TY0ne.png)

### 4.4 测试secondChance算法
按照要求选择LRU（matrix)算法，使用课件上的样例，选择3块物理块，输入12次要访问页面的次数，并依次输入每一次要访问的页面编号，如下图所示：

![16.png](https://z3.ax1x.com/2021/06/14/2TYB0H.png)

执行结果如下：

![17.png](https://z3.ax1x.com/2021/06/14/2TYD7d.png) ![18.png](https://z3.ax1x.com/2021/06/14/2TYsAA.png)

和课本中的结果基本一致，都满足secondChance的原则：

![19.png](https://z3.ax1x.com/2021/06/14/2TYytI.png)