# Lab Week 02. 实验报告!

## 实验内容

1. 了解 Linux 下 x86 汇编语言编程环境； 
2. 验证实验 Blum’s Book: Sample programs in Chapter 04, 05 (Moving Data)。 

## 实验报告

相关内容的技术日志、遇到问题和解决方案

## 实践

### 1.了解 Linux 下 x86 汇编语言编程环境（进行实验使用到在相关知识点，下面运行过程也有补充知识）

利用Linux汇编语言编程环境进行汇编，最主要的是要协同利用汇编器、链接器和调试器。

  * 汇编器的作用：将汇编语言源程序转换成二进制形式的目标代码。*Linux* 平台的标准汇编器是 *GAS* ，它是 *GCC* 所依赖的后台汇编工具。具体命令行：

    ``$ as -o xxxx.o xxxx.s``

  * 链接器的作用：用来将多个目标代码连接成一个可执行文件， *Linux* 使用 *ld* 作为标准的链接程序：具体命令行：

    `$ld -o xxxx xxxx.o`

  * 调试器：*inux* 下调试汇编程序通常用 *GDB*、*DDD* 这类通用的调试器，在进行调试前， *as* 命令需要带上参数 *--gstabs* ，这是为了使得汇编器在生成的目标代码中加上符号表，具体命令行：

    ```
    $ as --gstabs -o xxxx.o xxxx.s
    $ ld -o xxxx xxxx.o
    ```

  * 也可以使用gcc在单一步骤内汇编和连接汇编语言程序，具体命令如下：

    `gcc -o xxxx xxxx.s`

    （注意需要把汇编程序中的_start改成main）

  * 考虑到Blum’s Book中采用的是32位的操作系统，而现在常用的操作系统为64位，运行书中的一些程序我们还需要使用到以下的一些命令行：

    1. 在程序的源代码开头之前加上：

       	.code32
       	
       		b. 安装所需的库：

       sudo apt-get update
       sudo apt install lib32z1 lib32ncurses5 g++-multilib libc6-dev-i386

       	c. 编译链接采用以下的命令：

    ```
    as --32 -o xxxx.o xxxx.s
    ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o xxxx -lc xxxx.o
    ```
  
  * 连接到c库语言
    在汇编语言程序中使用c库语言时，需要把c库文件连接到程序目标代码。
    采用的方法可以是静态链接和动态链接，因为静态链接会造成大量的内存浪费，
    因此最常用的还是动态链接。
  
    标准的c语言库位于libc.so.x文件中。为了连接该文件必须使用GNU连接器的-l参数。
    运行c语言函数时，还需要加载动态库在程序，对于linux该程序为linux.so.2，它通常在/lib目录下，完成动态链接需要用到的链接命令行为：
  
    ```
    ld -dynamic-linker /lib/ld-linux.so.2 -o xxxx -1c xxxx.o
    
    ```

    

### 2. 验证实验 Blum’s Book: Sample programs in Chapter 04, 05 (Moving Data)

#### 程序一：cpuid.s

  * 源程序：

```
  #cpuid.s Sample program to extract the processor Vendor ID
  .section .data
  output:	
  	.ascii "The processor Vendor ID is 'xxxxxxxxxxxx'\n"
  .section .text
  .globl _start
  _start:
  	nop
  	movl $0 , %eax
  	#%eax存储系统调用值，决定CPUID生成什么信息 
  	cpuid
  movl $output, %edi
  movl %ebx, 28(%edi)
  movl %edx,32(%edi)
  movl %ecx, 36(%edi)
  movl $4, %eax
  movl $1, %ebx
  movl $output, %ecx
  movl $42, %edx
  int $0x80
  movl $1, %eax
  movl $0, %ebx
  int $0x80

```

  * 使用常用步骤汇编链接该程序：
  
    ```
    as -o cpuid.o cpuid.s
    ld -o cpuid cpuid.o
    ./cpuid
    ```

    执行结果图：

    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311103758611.png)


* 使用编译器汇编链接该程序
  
    (注意要把原程序中的_start改成main)
  
    ```
  gcc -o cpuid cpuid.s
    ./cpuid
  ```
  
  执行结果图：
  
  ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311103831410.png)
  
  
  * 使用gdb对程序进行调试：
  
    ```
    as -gstabs -o cpuid.o cpuid.s
    ld -o cpuid cpuid.o
    gdb cpuid
    ```
  
  * 补充gdb命令的相关知识点
  
    1. 使用run命令运行程序
  
    2. 采用break设置断定，断点位置需要是最近标签的相对位置，命令行格式如下：
  
       ```
       break * label+offset
       ```
  
    3. 设置*_start作为断点程序会忽略掉该断点，因此我们常常在*_start后面加一个nop空指令，断点位置设置为*_start+1
  
    4. next/step/n/s都可以作为单步调试命令
  
    5. cont指令按照正常方式继续执行该程序
  
    6. 查看数据
  
       ```
       info registers->显示所有寄存器在值
       print->显示特定寄存器或者变量值
       x->显示指定内存位置在内容
       ```
  
       print命令相关知识点

       ```
     print/d->显示十进制的值
       print/t->显示二进制的值
     print/x->显示十六进制的值
       ```

       x命令相关知识点

       ```
     x/nyz
       其中n表示显示的数
     y表示输出格式
       y可以取到：c(字符），d（十进制），x（十六进制）
     z表示要显示在字段的度
       z可以取到：b(字节），h（16位），w（32位）
       ```
     ```
    
     ```
  
  调试过程图：
  
  ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/202103111039066.png)
  
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311103914837.png)
  
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311103935162.png)
  
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311103948148.png)
  
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104001930.png)
  
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/2021031110401618.png)
  
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104029697.png)
  ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104039179.png)
  
  
  #### 程序二：cpuid2.s
  
  * 源程序：
  
    ```
  #cpuid2.s View the CPUID Vendor ID string USing C library calls
    .code32
  .section .data
    output:
    	.asciz "The processor Vendor ID is '%s'\n"
    .section .bss
    	.lcomm buffer, 12
    .section .text
  .globl main 
    main:
  	movl $0,%eax
    	cpuid
  	movl $buffer,%edi
    	movl %ebx,(%edi)
  	movl %edx,4(%edi)
    	movl %ecx,8(%edi)
  	push $buffer
    	push $output
    	call printf
    	addl $8,%esp//用于清空printf函数放入堆栈在参数
    	push $0
    	call exit
    	call exit
    
    ```
  ```
  
  * 采取32位操作系统的汇编方式汇编链接程序（注意源代码前面要加上.code32)
  
  ```
  as --32 -o cpuid2.o cpuid2.s
    ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o cpuid2 -lc cpuid2.o
  ./cpuid2
    ```
  
  执行结果图：
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104105520.png)
  
  
    ```
#### 程序三：sizetest1.s

* 源代码：
  
  ```
    #sizetest1.s - A sample program to view the executable size
  .section .text
    .globl _start
  _start:
    
    movl $1,%eax
    movl $0,%ebx
    int $0x80
  ```
  
    
  
  * 编译链接，然后查看它的长度
  
    ```
    as -o sizetest1.o sizetest1.s
  ld -o sizetest1 sizetest1.o
    ls -al sizetest1
    ```
  ```
  
    执行结果图：
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104123919.png)

  
  
  
  ```
#### 程序四：sizetest2.s

* 源代码：
  
  ```
    # sizetest2.s -A sample program to view the executable size
    .section .bss
    	.lcomm buffer,10000
    .section .text
    .globl _start
    _start: 
    	movl $1,%eax
    	movl $0,%ebx
    	int $0x80
    
  ```
  
  * 汇编链接，然后查看它的长度

        as -o sizetest2.o sizetest2.s
      ld -o sizetest2 sizetest2.o
        ls -al sizetest2
          
        	执行结果图：
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104142358.png)

    相对于sizetext1.s，我们添加了10000字节在缓冲区，但是可执行程序文件在长度只是增加了23个字节

  #### 程序五：sizetest3.s
  
* 源代码：
  
  ```
    #sizetest3.s -A sample program to view the executable size
  .section .data
    	.fill 10000
    .section .text
    .globl _start
    _start:
            movl $1,%eax
            movl $0,%ebx
            int $0x80
    
    
  ```
  
  * 汇编链接，然后查看它的长度
  
    ```
  as -o sizetest3.o sizetest3.s
    ld -o sizetest3 sizetest3.o
  ls -al sizetest3
    ```
  
    执行结果图：
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104158989.png)
  
    观察可执行程序在长度，发现缓冲区空间在10000字节被添加进了可执行程序
  
  #### 程序六：movetest1.s
  
  * 源代码：
  
  ```
    #movtest1.s - An example of moving data from memory to a register
      .section .data
            value:
                .int 1
      .section .text
        .globl _start
          _start:
                nop
              movl value,%ecx
                movl $1,%eax
                movl $0,%ebx
                int $0x80
  ```
  
  * 汇编链接
  
    ```
    as -gstabs -o movtest1.o movtest1.s
    ld -o movtest1 movtest1.o
    gdb -q movtest1
    ```
  
  * 补充知识
    mov通过增加x后缀来声明要传送在数据元素长度，具体命令行：
  
  ```
    movx
  其中x可以取到：l（32位的长字值），w（16位的字值），b（8位的字节值）
  ```
  
    执行结果图：
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104221392.png)

  
  ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104236893.png)
  
  可见，内存位置中存储的值被传送给了ECX寄存器
  
  #### 程序七：movetest2.s
  
  * 源代码：

    ```
  #movtest2.s
    .section .data
    	value:
    		.int 1
    .section .text
    .globl _start
    	_start:
    		nop
    		movl $100,%eax
    		movl %eax,value
    		movl $1,%eax
    		movl $0,%ebx
    		int $0x80
    
    ```
  
  * 汇编链接
  
    ```
    as -gstabs -o movtest2.o movtest2.s
    ld -o movtest2 movtest2.o
    gdb -q movtest2
    ```
  
    执行结果图：
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104249153.png)
  

    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104302557.png)

    通过查看value标签的内存位置，发现初始值l被存储了。单步运行程序，直到EAX寄存器的
    值被传送到内存位置后，可以再次查看该值，为100，所以寄存器的值确实被存储到内存位置中。
  
  #### 程序八：movetest3.s
  
* 源代码：
  
  ```
    # movtest3.s -Another example of using indexed memory locations
  .section .data
    output:
  	.asciz "The value is %d\n"
    values:
  	.int 10,15,20,25,30,35,40,45,50,55,60
    .section .text
  .globl _start
    _start:
  	nop
    	movl $0,%edi
    loop:
    movl values(,%edi,4),%eax
    pushl %eax
    pushl $output
    call printf
    addl $8,%esp
    inc %edi
    cmpl $11,%edi
    jne loop
    movl $0,%ebx
    movl $1,%eax
    int $0x80
    
    
  ```
  
  * 汇编链接

    ```
  $as --32 -gstabs -o movetest3.o movetest3.s
    $ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -lc -o movetest3 movetest3.o
    ./movetest3
    ```
  
    执行结果图：

    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104316195.png)

  
  ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104327291.png)
  

  

  #### 程序九：movetest4.s

  * 源代码：

    ```
    #movtest4.s - An example of indirect addressing
    .section .data
    values:
    	.int 10,15,20,25,30,35,40,45,50,55,60
    .section .text
    .globl _start
    _start:
    	nop
    movl values,%eax
    movl $values,%edi
    movl $100,4(%edi)
    movl $1,%edi
    movl values(,%edi,4),%ebx
    movl $1,%eax
    int $0x80
    
    ```
  
  * 汇编链接
  
    ```
    as -gstabs -o movtest4.o movtest4.s
    ld -o movtest4 movtest4.o
    gdb -q movtest4
    ```
  
    执行结果图：

    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104347124.png)

  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104358508.png)
  
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104418969.png)

  
#### 程序九：cmovetest.s

* 源代码：
  
  ```
    #cvotest.s - An example of the CMOV instructions
  .section .data
    output:
  	.asciz"The largest value is %d\n"
    values:
  	.int 105,235,61,315,134,221,53,145,117,5
    .section .text
    .globl _start
    _start:
    	nop
    	movl values,%ebx
    	movl $1,%edi
    loop:
    	movl values(,%edi,4),%eax
    	cmp %ebx,%eax
    	cmova %eax,%ebx
    	inc %edi
    	cmp $10,%edi
  	jne loop
    	pushl %ebx
  	pushl $output
    	call printf
    	addl $8,%esp
    	pushl $0
    	call exit
    
  ```
  
* 汇编链接
  
  ```
    as --32 -gstabs -o cmovtest.o cmovtest.s
  ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -lc -o cmovtest cmovtest.o
    ./cmovtest
  ```
  
  执行结果图：
  
  ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104432746.png)
  

    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104448133.png)
  
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/2021031110445943.png)
  
  
  #### 程序十：swaptest.s
  
  * 源代码：
  
    ```
    # swaptest.s -An example of using the BSWAP instruction
    .section .text
    .globl _start
    _start:
    	nop
    	movl $0x12345678, %ebx
    	bswap %ebx
    	movl $1,%eax
    	int $0x80
    
    ```

  * 汇编链接调试

    ```
    as --gstabs -o swaptest.o swaptest.s
    ld -o swaptest swaptest.o
    gdb -q swaptest
    ```

    执行结果图：

    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104518735.png)

  
  ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104530518.png)
  

  

  #### 程序十一：cmpxchgtest.s

  * 源代码：

    ```
    #cmpxchg8btest.s -An example of the cmpxchg8b instruction
    
    .section .data
    	data:
    	.byte 0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88
    
    .section .text
    .globl _start
    _start:
    	nop
    	movl $0x44332211,%eax
    	movl $0x88776655,%edx
    	movl $0x11111111,%ebx
    	movl $0x22222222,%ecx
    	cmpxchg8b data
    	movl $0, %ebx
    	movl $1, %eax
    	int $0x80
    
    
    ```
  
  * 汇编链接调试
  
    ```
    as --gstabs -o cmpxchgtest.o cmpxchgtest.s
    ld -o cmpxchgtest cmpxchgtest.o
    gdb -q cmpxchgtest
    ```
  
    执行结果图：

    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104603657.png?)

  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104616359.png?)
  
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104627303.png)

  

  
#### 程序十二：bubble.s

* 源代码：
  
  ```
    # -An axample of the XCHG instruction
  .section .data
    values: 
    	.int 105,235,61,315,134,221,53,145,117,5
    .section .text
    .globl _start
    _start:
    	movl $values,%esi
    	movl $9,%ecx
    	movl $9,%ebx
    loop:
    	movl (%esi),%eax
    	cmp %eax,4(%esi)
    	jge skip
    	xchg %eax,4(%esi)
    	movl %eax,(%esi)
    skip:
    	add $4,%esi
    	dec %ebx
    	jnz loop
    	dec %ecx
    	jz end 
    	movl $values,%esi
    	movl %ecx,%ebx
    	jmp loop
    end:
    	movl $1,%eax
    	movl $0,%ebx
    	int $0x80
    
  ```
  
* 汇编链接调试
  
  ```
    as -gstabs -o bubble.o bubble.s
    ld -o bubble bubble.o
    gdb -q bubble
  ```
  
  执行结果图：
  
  ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104639888.png)
  

    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104656582.png)

  
#### 程序十三：pushpop.s

  * 源代码：
  
    ```
    #pushpop.s -An example of using the PUSH and POP instructions
    .section .data
    data:
    	.int 125
    .section .text
    .globl _start
    _start:
    	nop
    movl $24420,%ecx
    movw $350,%bx
    movl $100 ,%eax
    pushl %ecx
    pushw %bx
    pushl %eax
    pushl data
    pushl $data
    
    popl %eax
    popl %eax
    popl %eax
    
    popw %ax
    popl %eax
    movl $0,%ebx
    movl $1,%eax
    int $0x80
    
    
    ```
  
  * 汇编链接调试
  
    ```
    as --32 -gstabs -o pushpop.o pushpop.s
    ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -lc -o pushpop pushpop.o
    gdb -q pushpop
    ```
  
    执行结果图：
  
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104714362.png)
    
    ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104730345.png)

  ![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104730345.png)

​	![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104741661.png)



​	![](http://hurq5.gitee.io/os-labwork/LabWeek02/pictures/20210311104751918.png)