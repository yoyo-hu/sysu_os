# Lab Week 18.
## 1 实验内容：硬盘调度。
编写 C 程序模拟实现课件 Lecture25 中的硬盘柱面访问调度算法包括 FCFS、SSTF、SCAN、C-SCAN、LOOK、C-LOOK，并设计输入用例验证结果。
## 2 程序所涉及到的算法介绍以及相关实现函数
### 2.1 FIFO
#### 2.1.1 介绍：
FCFS算法调度磁盘先服务于最先请求磁盘I／O的进程
#### 2.1.2 程序中该算法的实现思路如下：
程序定义`distance`来记录磁头扫过的寻道距离，定义`curCylinder`来实时记录经过的柱面号，`curCylinder`按顺序依次被请求队列中的元素赋值，表示当前的磁头依次扫过请求队列中的柱面，满足了先来先服务的原则，磁头的移位，`distance`都加上前一个结点和下一个结点的距离，过程中不断打印`curCylinder`的值表示磁头扫过的路径，最后打印`distance`的值表示总的寻道距离。
#### 2.1.3 算法代码以及详细注释如下：
```c
/*先来先服务调度算法FCFS*/
void FCFS_alg(){
	int distance=0;
	int j,i,curCylinder;
	printf("请输入当前的的柱面号: ");
	scanf("%d",&curCylinder);
	printf("依次经过的柱面为:\n");
	//打印依次经过的柱面号,curCylinder记录实时经过的柱面号
	for(i=0;i<CylindReqNum;i++){
		printf("%d ",curCylinder);
		distance+=abs(curCylinder-CylindReqList[i]);//移动一次的距离
		curCylinder=CylindReqList[i];
	}
	printf("%d ",curCylinder);
	printf("\n总寻道距离:%d\n",distance);
}
```
### 2.2 SSTF
#### 2.2.1 介绍：
该算法使得磁盘优先服务于要求访问的磁道与当前磁头所在的磁道距离最近的进程
#### 2.2.2 程序中该算法的实现思路如下：
程序定义`distance`来记录磁头扫过的寻道距离，定义`curCylinder`来实时记录经过的柱面号，对请求序列数组中的柱面号进行排序，便于找到距离当前磁头指向柱面号最近的请求柱面，使用while循环对排序好的队列进行顺序查找，找到恰好比初始结点大的最近的请求柱面下标`k`，定义`l=k-1`即恰好比当前结点号小的最近的请求柱面下标，定义`r=k`即恰好比当前结点号大的最近的请求柱面下标。
采用归并的方法，左右比较找到距离当前结点号最近的请求柱面，当左边结点`l`距离当前结点号最近更近时，将`curCylinder`赋值为`l`下标指向的请求柱面，即将磁头往左移，同时将`l`减一，使得`l`继续作为恰好比当前结点号小的最近的请求柱面下标,当右边结点`r`距离当前结点号最近更近时，同理将`curCylinder`赋值为`r`,并把`r`加一，当l<0的时候，磁头只能往右移动，当r>=CylindReqNum，磁头事能往左移动，这样就时刻满足了SSTF的原则。
对于磁头的每次移位，`distance`都加上前一个结点和下一个结点的距离，过程中不断打印`curCylinder`的值表示磁头扫过的路径，最后打印`distance`的值表示总的寻道距离。
#### 2.2.3 算法代码以及详细注释如下：
```c
/*最短寻道时间优先调度算法SSTF*/
void SSTF_alg(){
	int curCylinder;
	int i,j;
	int distance=0;
	//排序
	qsort(CylindReqList,CylindReqNum,sizeof(int),cmp); 
	printf("请输入当前柱面号: ");
	scanf("%d",&curCylinder);
	printf("依次经过的柱面为:\n");
	int k=0;//记录恰好比初始结点大的最近的请求柱面
	while(k<CylindReqNum&&CylindReqList[k]<curCylinder){
    	k++;
	}
	int l=k-1;//记录恰好比当前结点号小的最近的请求柱面
	int r=k;//记录恰好比当前结点号大的最近的请求柱面
	//采用归并的方法，左右比较找到当前结点号最近的请求柱面
	while((l>=0)&&(r<CylindReqNum)){
		if((curCylinder-CylindReqList[l])<=(CylindReqList[r]-curCylinder)){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[l]);
			curCylinder=CylindReqList[l];
			l=l-1;
		}
		else{
			printf("%d ",curCylinder);
			distance+=(CylindReqList[r]-curCylinder);
			curCylinder=CylindReqList[r];
			r=r+1;
		}
	}
	while(l>=0){
        printf("%d ",curCylinder);
        distance+=(curCylinder-CylindReqList[l]);
        curCylinder=CylindReqList[l];
        l=l-1;
	}
	while(r<CylindReqNum){
        printf("%d ",curCylinder);
        distance+=(CylindReqList[r]-curCylinder);
        curCylinder=CylindReqList[r];
        r=r+1;
	}
	printf("%d ",curCylinder);
	printf("\n总寻道距离:%d\n",distance);
}
```
### 2.3 SCAN
#### 2.3.1 介绍：
磁盘臂从圆盘的一端开始向圆盘移动，另一端，当它到达有请求服务的柱面时则为其服务，直到它到达磁盘的另一端，在那里磁头运动反向并继续回来为有请求服务的柱面服务，直到没有请求为止。
#### 2.3.2 程序中该算法的实现思路如下：
程序定义`distance`来记录磁头扫过的寻道距离，定义`curCylinder`来实时记录经过的柱面号，对请求序列数组中的柱面号进行排序，便于“电梯式”的滑动访问，使用while循环对排序好的队列进行顺序查找，找到恰好比初始结点大的最近的请求柱面下标`k`，定义`l=k-1`即恰好比初始结点号小的最近的请求柱面下标，定义`r=k`即恰好比初始结点号大的最近的请求柱面下标。
选择磁盘臂要向内移动还是向外移动，若向内移动，按照SCAN算法，一开始磁头向内滑动一直滑动到磁盘的头部即0处，一路上响应请求服务的柱面，到达0处，磁头反向滑动，向外滑动，一路上服务请求的柱面，直到响应全部服务结束，向外移动同理。
对于磁头的每次移位，`curCylinder`的值都要更新，`distance`都加上前一个结点和下一个结点的距离，过程中不断打印`curCylinder`的值表示磁头扫过的路径，最后打印`distance`的值表示总的寻道距离。
#### 2.3.3 算法代码以及详细注释如下：
```c
/*电梯算法一SCAN*/
void SCAN_alg(){
	int curCylinder,direction;
	int i,j,distance=0;
	//排序
	qsort(CylindReqList,CylindReqNum,sizeof(int),cmp); 
	printf("请输入当前柱面号: ");
	scanf("%d",&curCylinder);
	int k=0;//记录恰好比初始结点大的最近的请求柱面
	while(k<CylindReqNum&&CylindReqList[k]<curCylinder){
		k++;
	}
	int l=k-1;//记录恰好比初始结点小的最近的请求柱面
	int r=k;//记录恰好比初始结点大的最近的请求柱面
	printf("请输入当前移动臂的移动方向（0表示向内,1表示向外）:");
	scanf("%d",&direction);
	printf("依次经过的柱面为:\n");
	if(direction==0){//向内开始滑动
		//按照SCAN算法，一开始磁头向内滑动，一路上响应请求服务的柱面
		for(j=l;j>=0;j--){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[j]);
			curCylinder=CylindReqList[j];
		}
		//磁头一直滑动到磁盘的头部即0处
		if(CylindReqList[0]!=0){
			printf("%d ",curCylinder);
			distance+=(curCylinder-0);
			curCylinder=0;
		}
		//磁头反向滑动，向外滑动，一路上服务请求的柱面，直到响应全部服务结束
		for(j=r;j<CylindReqNum;j++){
			printf("%d ",curCylinder);
			distance+=(CylindReqList[j]-curCylinder);
			curCylinder=CylindReqList[j];
		}
		printf("%d ",curCylinder);
	}
	else{//向外开始滑动
		//按照SCAN算法，一开始磁头向外滑动，一路上响应请求服务的柱面
		for(j=r;j<CylindReqNum;j++){
			printf("%d ",curCylinder);
			distance+=(CylindReqList[j]-curCylinder);
			curCylinder=CylindReqList[j];
		}
		//磁头一直滑动到磁盘的尾部即CylindReqNum-1处
		if(CylindReqList[CylindReqNum-1]!=CylindNum-1){
			printf("%d ",curCylinder);
			distance+=(CylindNum-1-curCylinder);
			curCylinder=CylindNum-1;
		}
		//磁头反向滑动，向内滑动，一路上服务请求的柱面，直到响应全部服务结束
		for(j=l;j>=0;j--){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[j]);
			curCylinder=CylindReqList[j];
		}
		printf("%d ",curCylinder);
	}
	printf("\n总寻道距离:%d\n",distance);
}
```
### 2.4 C-SCAN
#### 2.4.1 介绍：
SCAN 的一种变体，与 SCAN 一样，C-SCAN 移动从磁盘的一端到另一端，一路上完成请求的服务。不同的是，当头部到达另一端时，它立即返回到磁盘的开头，无需对回程时的任何请求提供服务，再重新从磁盘的一断出发，直到完成所有的剩余请求。
#### 2.4.2 程序中该算法的实现思路如下：
程序定义`distance`来记录磁头扫过的寻道距离，定义`curCylinder`来实时记录经过的柱面号，对请求序列数组中的柱面号进行排序，便于“电梯式”的滑动访问，使用while循环对排序好的队列进行顺序查找，找到恰好比初始结点大的最近的请求柱面下标`k`，定义`l=k-1`即恰好比初始结点号小的最近的请求柱面下标，定义`r=k`即恰好比初始结点号大的最近的请求柱面下标。
选择磁盘臂要向内移动还是向外移动，若向内移动，按照C-SCAN算法，一开始磁头向内滑动，一路上响应请求服务的柱面，磁头一直滑动到磁盘的头部即0处,接着磁头反向滑动，向外滑动，一直滑动到磁盘的尾部即CylindReqNum-1处，一路上不响应请求的服务，最后磁头再次反向滑动，向内滑动，一路上服务请求的柱面，直到响应全部服务结束，向外移动同理。
对于磁头的每次移位，`curCylinder`的值都要更新，`distance`都加上前一个结点和下一个结点的距离，过程中不断打印`curCylinder`的值表示磁头扫过的路径，最后打印`distance`的值表示总的寻道距离。
#### 2.4.3 算法代码以及详细注释如下：
```c
/*电梯算法二CSCAN*/
void CSCAN_alg(){
	int curCylinder,direction;
	int i,j,distance=0;
	//排序
	qsort(CylindReqList,CylindReqNum,sizeof(int),cmp); 
	printf("请输入当前柱面号: ");
	scanf("%d",&curCylinder);
	int k=0;//记录恰好比初始结点大的最近的请求柱面
	while(k<CylindReqNum&&CylindReqList[k]<curCylinder){
		k++;
	}
	int l=k-1;//记录恰好比初始结点小的最近的请求柱面
	int r=k;//记录恰好比初始结点大的最近的请求柱面
	printf("请输入当前移动臂的移动方向（0表示向内，1表示向外):");
	scanf("%d",&direction);
	printf("依次经过的柱面为:\n");
	if(direction==0){//向内开始滑动
		//按照C-SCAN算法，一开始磁头向内滑动，一路上响应请求服务的柱面
		for(j=l;j>=0;j--){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[j]);
			curCylinder=CylindReqList[j];
		}
		//磁头一直滑动到磁盘的头部即0处
		if(CylindReqList[0]!=0){
			printf("%d ",curCylinder);
			distance+=(curCylinder-0);
			curCylinder=0;
		}
		//磁头反向滑动，向外滑动，一直滑动到磁盘的尾部即CylindReqNum-1处，一路上不响应请求的服务
		if(CylindReqList[CylindReqNum-1]!=curCylinder-1){
			printf("%d ",curCylinder);
			distance+=(CylindNum-1-curCylinder);
			curCylinder=CylindNum-1;
		}
		//磁头再次反向滑动，向内滑动，一路上服务请求的柱面，直到响应全部服务结束
		for(j=CylindReqNum-1;j>=r;j--){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[j]);
			curCylinder=CylindReqList[j];
		}
		printf("%d ",curCylinder);
	}
	else{//向外开始滑动
		//按照C-SCAN算法，一开始磁头向外滑动，一路上响应请求服务的柱面
		for(j=r;j<CylindReqNum;j++){
			printf("%d ",curCylinder);
			distance+=(CylindReqList[j]-curCylinder);
			curCylinder=CylindReqList[j];
		}
		//磁头一直滑动到磁盘的尾部即CylindReqNum-1处
		if(CylindReqList[CylindReqNum-1]!=curCylinder-1){
			printf("%d ",curCylinder);
			distance+=(CylindNum-1-curCylinder);
			curCylinder=CylindNum-1;
		}
		//磁头反向滑动，向内滑动，一直滑动到磁盘的头部即0处，一路上不响应请求的服务
		if(CylindReqList[0]!=0){
			printf("%d ",curCylinder);
			distance+=(curCylinder-0);
			curCylinder=0;
		}
		//磁头再次反向滑动，向外滑动，一路上服务请求的柱面，直到响应全部服务结束
		for(j=0;j<=l;j++){
			printf("%d ",curCylinder);
			distance+=(CylindReqList[j]-curCylinder);
			curCylinder=CylindReqList[j];
		}
		printf("%d ",curCylinder);
	}
	printf("总寻道距离:%d\n",distance);
}
```
### 2.5 LOOK
#### 2.5.1 介绍：
和SCAN类似，不同的是磁盘臂不是达到磁盘的另一端运动才反向，而是到达该方向的最后一个 I/O 请求的时候，运动就反向。
#### 2.5.2 程序中该算法的实现思路如下：
程序定义`distance`来记录磁头扫过的寻道距离，定义`curCylinder`来实时记录经过的柱面号，对请求序列数组中的柱面号进行排序，便于“电梯式”的滑动访问，使用while循环对排序好的队列进行顺序查找，找到恰好比初始结点大的最近的请求柱面下标`k`，定义`l=k-1`即恰好比初始结点号小的最近的请求柱面下标，定义`r=k`即恰好比初始结点号大的最近的请求柱面下标。
选择磁盘臂要向内移动还是向外移动，若向内移动，按照LOOK算法，一开始磁头向内滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求被响应，接着磁头反向滑动，向外滑动，一路上服务请求的柱面，直到响应全部服务结束，向外移动同理。
对于磁头的每次移位，`curCylinder`的值都要更新，`distance`都加上前一个结点和下一个结点的距离，过程中不断打印`curCylinder`的值表示磁头扫过的路径，最后打印`distance`的值表示总的寻道距离。
#### 2.5.3 算法代码以及详细注释如下：
```c
/*电梯算法三LOOK*/
void LOOK_alg(){
	int curCylinder,direction;
	int i,j,distance=0;
	//排序
	qsort(CylindReqList,CylindReqNum,sizeof(int),cmp); 
	printf("请输入当前柱面号: ");
	scanf("%d",&curCylinder);
	int k=0;//记录恰好比初始结点大的最近的请求柱面
	while(k<CylindReqNum&&CylindReqList[k]<curCylinder){
		k++;
	}
	int l=k-1;//记录恰好比初始结点小的最近的请求柱面
	int r=k;//记录恰好比初始结点大的最近的请求柱面
	printf("请输入当前移动臂的移动方向（0表示向内,1表示向外）:");
	scanf("%d",&direction);
	printf("依次经过的柱面为:\n");
	if(direction==0){//向内开始滑动
		//按照LOOK算法，一开始磁头向内滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求被响应
		for(j=l;j>=0;j--){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[j]);
			curCylinder=CylindReqList[j];
		}
		//磁头反向滑动，向外滑动，一路上服务请求的柱面，直到响应全部服务结束
		for(j=r;j<CylindReqNum;j++){
			printf("%d ",curCylinder);
			distance+=(CylindReqList[j]-curCylinder);
			curCylinder=CylindReqList[j];
		}
		printf("%d ",curCylinder);
	}
	else{//向外开始滑动
		//按照LOOK算法，一开始磁头向外滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求被响应
		for(j=r;j<CylindReqNum;j++){
			printf("%d ",curCylinder);
			distance+=(CylindReqList[j]-curCylinder);
			curCylinder=CylindReqList[j];
		}
		//磁头反向滑动，向内滑动，一路上服务请求的柱面，直到响应全部服务结束
		for(j=l;j>=0;j--){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[j]);
			curCylinder=CylindReqList[j];
		}
		printf("%d ",curCylinder);
	}
	printf("总寻道距离:%d\n",distance);
}
```
### 2.6 C-LOOK
#### 2.6.1 介绍：
和C-SCAN类似，不同的是磁盘臂不是达到磁盘的另一端运动才反向，而是到达该方向的最后一个 I/O 请求的时候，运动就反向。
#### 2.6.2 程序中该算法的实现思路如下：
程序定义`distance`来记录磁头扫过的寻道距离，定义`curCylinder`来实时记录经过的柱面号，对请求序列数组中的柱面号进行排序，便于“电梯式”的滑动访问，使用while循环对排序好的队列进行顺序查找，找到恰好比初始结点大的最近的请求柱面下标`k`，定义`l=k-1`即恰好比初始结点号小的最近的请求柱面下标，定义`r=k`即恰好比初始结点号大的最近的请求柱面下标。
选择磁盘臂要向内移动还是向外移动，若向内移动，按照C-LOOK算法，一开始磁头向内滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求的位置，接着磁头反向滑动，向外滑动，一直滑动到该方向上的最后一个请求的位置，一路上不响应请求的服务，最后磁头再次反向滑动，向内滑动，一路上响应请求的服务，直到响应全部服务结束，向外移动同理。
对于磁头的每次移位，`curCylinder`的值都要更新，`distance`都加上前一个结点和下一个结点的距离，过程中不断打印`curCylinder`的值表示磁头扫过的路径，最后打印`distance`的值表示总的寻道距离。
#### 2.6.3 算法代码以及详细注释如下：
```c
/*电梯算法四CLOOK*/
void CLOOK_alg(){
	int curCylinder,direction;
	int i,j,distance=0;
	//排序
	qsort(CylindReqList,CylindReqNum,sizeof(int),cmp); 
	printf("请输入当前柱面号: ");
	scanf("%d",&curCylinder);
	int k=0;//记录恰好比初始结点大的最近的请求柱面
	while(k<CylindReqNum&&CylindReqList[k]<curCylinder){
		k++;
	}
	int l=k-1;//记录恰好比初始结点小的最近的请求柱面
	int r=k;//记录恰好比初始结点大的最近的请求柱面
	printf("请输入当前移动臂的移动方向（0表示向内，1表示向外):");
	scanf("%d",&direction);
	printf("依次经过的柱面为:\n");
	if(direction==0){//向内开始滑动
		//按照C-LOOK算法，一开始磁头向内滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求的位置
		for(j=l;j>=0;j--){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[j]);
			curCylinder=CylindReqList[j];
		}
		//磁头反向滑动，向外滑动，一直滑动到该方向上的最后一个请求的位置，一路上不响应请求的服务
		if(r<=CylindReqNum-1){
			printf("%d ",curCylinder);
			distance+=(CylindReqList[CylindReqNum-1]-curCylinder);
			curCylinder=CylindReqList[CylindReqNum-1];
		}
		//磁头再次反向滑动，向内滑动，一路上响应请求的服务，直到响应全部服务结束
		for(j=CylindReqNum-2;j>=r;j--){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[j]);
			curCylinder=CylindReqList[j];
		}
		printf("%d ",curCylinder);
	}
	else{//向外开始滑动
		//按照C-LOOK算法，一开始磁头向外滑动，一路上响应请求服务的柱面,一直到该方向上的最后一个请求的位置
		for(j=r;j<CylindReqNum;j++){
			printf("%d ",curCylinder);
			distance+=(CylindReqList[j]-curCylinder);
			curCylinder=CylindReqList[j];
		}
		//磁头反向滑动，向内滑动，一直滑动到该方向上的最后一个请求的位置，一路上不响应请求的服务
		if(l>=0){
			printf("%d ",curCylinder);
			distance+=(curCylinder-CylindReqList[0]);
			curCylinder=CylindReqList[0];
		}
		//磁头再次反向滑动，向外滑动，一路上响应请求的服务，直到响应全部服务结束
		for(j=1;j<=l;j++){
			printf("%d ",curCylinder);
			distance+=(CylindReqList[j]-curCylinder);
			curCylinder=CylindReqList[j];
		}
		printf("%d ",curCylinder);
	}
	printf("总寻道距离:%d\n",distance);
}
```
## 3 主函数解释以及输入输出说明：
### 3.1 主函数解释
程序文字提示用户输入初始的变量，如模拟硬盘的柱面个数，请求到达的柱面个数，请求到达的柱面序列，柱面访问调度算法，使用switch结果来实现调用用户选择的策略函数。
### 3.2 输入输出说明：
* **输入：** 按照程序的文字说明输入需要的初始值；
* **输出：** 输出在指定的柱面访问调度算法下，程度调度后的请求柱面序列（磁头滑动的路线），以及完成所有请求后的总寻道距离。

## 4 运行结果分析。
### 4.1 测试FIFO算法
使用课件上的样例,选择模拟硬盘的柱面个数为200个，即磁头再200个柱面之内移动，设置请求访问的柱面个数为8个，依次为98，183，37，122，14，124，65，67，选择算法为FIFO，设置初始磁头位置在柱面53中，那么它首先从53移到98,接着再到183、37、122、14、124、65，最后到67，磁头移动柱面的总数为640，执行结果如下图：

![[1](pic\1.png)](https://z3.ax1x.com/2021/06/15/2qgBUs.png)

可以观察到该结果与分析的结果一致，且与课件结果图（如下）一致：

![[2](pic\2.png)](https://z3.ax1x.com/2021/06/15/2qgdbQ.png)

### 4.2 测试SSTF算法
使用课件上的样例,选择模拟硬盘的柱面个数为200个，即磁头再200个柱面之内移动，设置请求访问的柱面个数为8个，依次为98，183，37，122，14，124，65，67，选择算法为SSTF，设置初始磁头位置在柱面53中，与开始磁头位置（53）的最近请求位于柱面65,因此磁头移动到柱面65处，一旦位于柱面65，下个最近请求位于柱面67。当磁头移动到柱面67时候，由于柱面37比98还要近，所以下次处理37。如此，会处理位于柱面14的请求，接着处理98，122，124，最后处理183。

![[3](pic\3.png)](https://z3.ax1x.com/2021/06/15/2qgD5n.png)

可以观察到该结果与分析的结果一致，且与课件结果图（如下）一致：

![[4](pic\4.png)](https://z3.ax1x.com/2021/06/15/2qgsCq.png)

### 4.3 测试SCAN算法
使用课件上的样例,选择模拟硬盘的柱面个数为200个，即磁头再200个柱面之内移动，设置请求访问的柱面个数为8个，依次为98，183，37，122，14，124，65，67，选择算法为SCAN，设置初始磁头位置在柱面53中，磁头朝0移动，因此磁头接下来处理的是对于柱面37的请求，然后是14，当磁头位于柱面0时，磁头会反转，移向磁盘的另一端，并处理柱面65、67、98、122、124、183上的请求。

![[5](pic\5.png)](https://z3.ax1x.com/2021/06/15/2qg0Ej.png)

可以观察到该结果与分析的结果一致，且与课件结果图（如下）一致：

![[6](pic\6.png)](https://z3.ax1x.com/2021/06/15/2qgy80.png)

### 4.4 测试C-SCAN算法
使用课件上的样例,选择模拟硬盘的柱面个数为200个，即磁头再200个柱面之内移动，设置请求访问的柱面个数为8个，依次为98，183，37，122，14，124，65，67，选择算法为C-SCAN，设置初始磁头位置在柱面53中，磁头朝外移动，因此磁头接下来处理的是对于柱面65的请求，然后是67，98，122，124，183，当磁头位于柱面199时，磁头会反转，它立即返回到磁盘的开头0处，而并不处理任何回程上的请求,磁头再重新朝外移动，并处理剩余的请求，即柱面14，37上的请求。

![[7](pic\7.png)](https://z3.ax1x.com/2021/06/15/2qg62V.png)

可以观察到该结果与分析的结果一致，且与课件结果图（如下）一致：

![[8](pic\8.png)](https://z3.ax1x.com/2021/06/15/2qgcvT.png)

### 4.5 测试LOOK算法
使用课件上的样例,选择模拟硬盘的柱面个数为200个，即磁头再200个柱面之内移动，设置请求访问的柱面个数为8个，依次为98，183，37，122，14，124，65，67，选择算法为LOOK，设置初始磁头位置在柱面53中，磁头朝0移动，因此磁头接下来处理的是对于柱面37的请求，然后是14，当磁头位于该方向上的最后一个请求位于的柱面14时，磁头会反转，移向磁盘的另一端，并处理柱面65、67、98、122、124、183上的请求。

![[9](pic\9.png)](https://z3.ax1x.com/2021/06/15/2qg2KU.png)

可以观察到该结果与分析的结果一致，且与课件结果图（如下）一致：

![[10](pic\10.png)](https://z3.ax1x.com/2021/06/15/2qgWb4.png)

### 4.5 测试C-LOOK算法
使用课件上的样例,选择模拟硬盘的柱面个数为200个，即磁头再200个柱面之内移动，设置请求访问的柱面个数为8个，依次为98，183，37，122，14，124，65，67，选择算法为C-LOOK，设置初始磁头位置在柱面53中，磁头朝外移动，因此磁头接下来处理的是对于柱面65的请求，然后是67，98，122，124，183，当磁头位于该方向上最后一个请求位于的柱面183时，磁头会反转，它立即返回到返回方向上的第一个请求14处，而并不处理任何回程上的请求,磁头再重新朝外移动，并处理剩余的请求，即柱面14，37上的请求。

![[11](pic\11.png)](https://z3.ax1x.com/2021/06/15/2qgRrF.png)

可以观察到该结果与分析的结果一致，且与课件结果图（如下）一致：

![[12](pic\12.png)](https://z3.ax1x.com/2021/06/15/2qghVJ.png)
