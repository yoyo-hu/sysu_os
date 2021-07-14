# Lab Week 03.
##  实验要求

验证实验 Blum’s Book: Sample programs in Chapter 06, 07
(Controlling Flow and Using Numbers)

## 实验报告

相关内容的技术日志、遇到问题和解决方案。

## 实验内容

###  程序一：jumptest.s

* 知识点：(无条件跳转）跳转指令使用单一指令码

  ```
  jmp location
  ```

  

* 源代码：

  ```
  # jumptest.s - An example of the jmp instruction
  .section .text
  .globl _start
  _start:
  	nop
  	movl $1,%eax #用于系统调用exit
  	jmp overhere
  	movl $10,%ebx
  	int $0x80
  overhere:
  	movl $20,%ebx
  	int $0x80
  
  ```

* 验证程序是否发生跳转

  1. 汇编链接代码，运行结果显示EBX寄存器的值为20，说明发生跳转：

     具体命令行如下：

     ```
     $as -o jumptest.o jumptest.s
     $ld -o jumptest jumptest.o
     $./jumptest
     $echo $?
     ```

     运行截图如下：

     ![image-20210305221730744](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210305221730744.png)

     

  2. 使用objdump程序反汇编代码找到跳转位置的机器指令位置，命令行如下：
  
     ```
   objdump -D jumptest
     ```

     运行截图如下：

     ![image-20210306094033894](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306094033894.png)

     断点设置的位置为0x804805a

     overhere跳转标签的位置为0x8048063

  3. 调试，在调试过程中观察第一个内存位置（显示在EIP寄存器中），具体命令行如下
  
     ```
     $ as --32 --gstabs -o jumptest.o jumptest.s
     $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o jumptest jumptest.o
   $gdb -q jumptest
     ```

     运行截图：

     ![image-20210306095046676](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306095046676.png)
  
     可以观察到断点处$eip的值等于0x8048055，和反汇编机器码的结果相同，两部单步执行后，\$eip的值等于跳转位置的值（即0x8048063），说明程序发生了跳转

### 程序二：calltest.s

* 知识点：程序调用以及返回，返回的地址常存在%esp中，我们可以通过在调用函数前后将\%esp的值保存在%ebp，以用于恢复内存地址：

  具体模板如下：

  ```
  function_label:
  	pushl %ebp
  	movl %esp,%ebp
  	....
  	movl %ebp,%esp
  	popl %ebp
  	ret
  ```

* 源代码：

  ```
  #calltest.s - An example of using the CALL instruction
  .section .data
  .globl _start
  _start:
  	pushl $1
  	pushl $output
  	call printf
  	add $8,%esp
  	call overhere
  	pushl $3
  	pushl $output
  	call printf
  	add $8,%esp
  	pushl $0
  	call exit
  overhere:
  	pushl %ebp
  	movl %esp,%ebp
  	pushl $2
  	pushl $output
  	call printf
  	add $8,%esp
  	movl %ebp,%esp
  	popl %ebp
  	ret
  ```

* 直接运行代码，通过print的输出观察调用跳转的情况：具体命令行如下：

  ```
  as --32 -o calltest.o calltest.s
  ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o calltest -lc calltest.o
  ./calltest
  ```

  ![image-20210306103504216](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306103504216.png)

### 程序三：cmptest.s

* 知识点：

  1. 比较指令：（条件跳转）CMP的指令格式为

     ```
     cmp operand1,operand2
     ```

  2. JGE表示如果大于或等于就跳转

* 源代码：

  ```
  # jumptest.s - An example of the jmp instruction
  .section .text
  .globl _start
  _start:
  	nop
  	movl $1,%eax
  	jmp overhere
  	movl $10,%ebx
  	int $0x80
  overhere:
  	movl $20,%ebx
  	int $0x80
  
  ```

* 使用以下命令行运行程序,观察寄存器中的值，检验是否发生跳转：

  ```
  $as --32 -o cmptest.o cmptest.s
  $ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o cmptest -lc cmptest.o
  $./cmptest
  $echo $?
  ```

  ​	运行结果图：

  ![image-20210306105036241](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306105036241.png)

  因为EBX寄存器中的值小于EAX的值，所以没有执行条件跳转指令，指令指针指向下一条指令，因此我们查看到的寄存器中的值为10

### 程序四：paritytest.s

* 知识点：JP ：如果奇偶校验则跳转（PF=1)

* 源代码：

  ```
  # paritytest.s - An example of testing the parity flag
  .section .text
  .globl _start
  _start:
  	movl $1,%eax
  	movl $4,%ebx
  	subl $3,%ebx
  	jp overhere
  	int $0x80
  overhere:
  	movl $100,%ebx
  	int $0x80
  ```

* 使用以下命令行运行程序,观察寄存器中的值，检验是否发生跳转：

  ```
  $as --32 -o paritytest.o paritytest.s
  $ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o paritytest -lc paritytest.o
  $./paritytest
  $echo $?
  ```

  运行截图：

  ![image-20210306105756919](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306105756919.png)

  分析：程序中，减法的结果为1，二进制表示为00000001.其中1的位数为奇数，因此不设置奇偶校验位，JP指令不会发生跳转，结果代码为1

### 程序五：signtest.s

* 知识点：JNS 如果无符号则跳转->SF=0

* 源代码：

  ```
  # signtest.s - An example of using the sign flag
  .section .data
  value:
  	.int 21,15,34,11,6,50,32,80,10,2
  output:
  	.asciz "The value is: %d\n"
  .section .text
  .globl _start
  _start:
  	movl $9,%edi
  loop:
  	pushl value(,%edi,4)
  	pushl $output
  	call printf
  	add $8,$esp
  	dec %edi
  	jns loop
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  
  ```

* 使用以下命令行运行程序,观察输出结果，检验是否发生跳转：

  ```
  $as --32 -o signtest.o signtest.s
  $ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o signtest -lc signtest.o
  $./signtest
  ```

  运行截图：

  ![image-20210306111624000](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306111624000.png)

  分析：程序反向遍历数据数组，使用EDI寄存器作为变址，JNS指令检查EDI寄存器什么时候变成负值，如果不是负值，则返回到循环的开头。

### 程序六：loop.s

* 知识点：循环指令使用ECX寄存器作为计数器并且随着循环指令的执行自动递减它的值

  | 指令          | 描述                                     |
  | ------------- | ---------------------------------------- |
  | LOOP          | 循环直到ECX寄存器为0                     |
  | LOOPE/LOOPZ   | 循环直到ECX寄存器为0，或者没有设置ZF标志 |
  | LOOPNE/LOOPNZ | 循环直到ECX寄存器为0，或者设置了ZP标志   |

* 源代码：

  ```
  #loop.s - An example of the loop instruction
  .section .text
  .globl _start
  _start:
  	mpvl $100,%ecx
  	movl $0,%eax
  loop1:
  	addl %ecx,%eax
  	loop loop1
  	pushl %eax
  	pushl $output
  	call printf
  	add $8,%esp
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  ```

* 使用以下命令行运行程序,观察输出结果，检验是否发生跳转：

  ```
  $as --32 -o loop.o loop.s
  $ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o loop -lc loop.o
  $./loop
  ```

  运行截图：

  ![image-20210306144436566](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306144436566.png)

### 程序七：betterloop.s

* 知识点：为了纠正“ECX本身初始值为负数，无法递减到0，而使得LOOP指令继续执行下去，最终导致寄存器溢出而退出”的错误，

  我们采用JCXZ指令进行纠正（其作用是如果寄存器CX为0时，则跳转）这样可以有效的避免程序进入LOOP循环

* 源代码：

  ```
  #betterloop.s - An example fo the loop and jcxz instructions
  .section .text
  .globl _start
  _start:
  	movl $0,%ecx
  	movl $0,%eax
  	jcxz done
  loop1:
  	addl %ecx,%eax
  	loop loop1
  done:
  	pushl %eax
  	pushl $output
  	call printf
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  
  ```

* 使用以下命令行运行程序,观察输出结果，检验是否发生跳转：

  ```
  $as --32 -o betterloop.o betterloop.s
  $ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o betterloop -lc betterloop.o
  $./betterloop
  ```

  运行截图：

  ![image-20210306145600314](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306145600314.png)

### 程序八：ifthen.c

* 知识点：

  将高级语言转换为编程语言，具体命令行如下：

  ```
  gcc -S xxxx.c
  ```

* 源代码：

  ```c
  // ifthen.c - A sample C if-then program
  #include <stdio.h>
  
  int main(){
  	int a=100;
  	int b=25;
  	if(a>b){
  		printf("The higher value is %d\n",a);
  	}else
  		printf("The higher value is %d\n",b);
  	return 0;
  }
  
  ```

* 使用GUU编译器的-S参数查看生成的汇编语言代码，命令行如下

  ```
  gcc -S ifthen.c
  cat ifthen.s
  ```

  运行结果图：

  ![image-20210306150208055](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306150208055.png)

  ![image-20210306150330694](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306150330694.png)

![image-20210306150348817](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306150348817.png)

### 程序九：for.c

* 源代码：

  ```c
  /*for.c - A sample C for program*/
  #include <stdio.h>
  int main(){
  	int i=0;
  	int j;
  	for(int i=0;i<1000;i++){
  		j=i+5;
  		printf("The answer is %d\n",j			);
  	}
  	return 0;
  }
  
  ```

* 使用GUU编译器的-S参数查看生成的汇编语言代码，命令行如下

  ```
  gcc -S for.c
  cat for.s
  ```

  运行结果图：

  ![image-20210306150722199](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306150722199.png)

  ​	![image-20210306150817242](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306150817242.png)

### 程序十：inttest.s

* 目的：把带符号整数存储在寄存器中的3种不同方法

* 源代码：

  ```
  #inttest.s - An example of using signed integers
  .section .data
  data:
  	.int -45
  .section .text
  .globl _start
  _start:
  	nop
  	movl $-345,%ecx
  	movw $0xffb1,%dx
  	movl data,%ebx
  	movl $1,%eax
  	int $0x80
  
  ```

* 分析：前两个mov指令是使用立即数把带符号负整数存放在寄存器中，第三个mov指令是使用标签把他存进寄存器中

  使用单步运行指令直到所有的数据都被加载到寄存器中。使用info命令显示寄存器：调试代码命令行如下：

  ```
  $ as --32 --gstabs -o inttest.o inttest.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o inttest inttest.o
  $gdb -q inttest
  ```

  运行结果图：

  ![image-20210306153913092](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306153913092.png)

  ![image-20210306153945688](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306153945688.png)

### 程序十一：movzxtest.s 

* 知识点：MOVZX指令可以把长度小的无符号整数值传送给长度大的无符号整数值

* 源代码：

  ```
  #movzxtext.s - A example of the MOVZX instruction
  .section .text
  .globl _start
  _start:
  	nop
  	movl $279,%ecx
  	movzx %cl,%ebx
  	movl $1,eax
  	int $0x80
  
  ```

* 通过调试监视程序的运行，并且查看寄存器的值是怎么变化的，命令行如下：

  ```
  $ as --32 --gstabs -o movzxtext.o movzxtext.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o movzxtext movzxtext.o
  $gdb -q movzxtext
  ```

  ![image-20210306223007798](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306223007798.png)

  ![image-20210306223114033](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306223114033.png)

  分析：观察到无符号整数的初始值为279，长度小的无符号整数值传送给长度大的无符号整数值后该数变成23，通过以十六进制的形式输出我们可以观察到，MOVZX指令只传送了ECX寄存器的低位字节，而用0填充了EBX中的剩余字节

### 程序十二： movsxtest.s

* 知识点：MOVSX指令，允许扩展符号整数并且保留符号

* 源代码：

  ```
  #movsxtest.s - An example of the MOVSX instruction
  .setion .text
  .globl _start
  _start:
  	nop
  	movw $-79,%cx
  	movl $0,%ebx
  	movw %cx,%bx
  	movsx %cx,%eax
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  
  ```

* 通过调试监视程序的运行，并且查看寄存器的值是怎么变化的，命令行如下：

  ```
  $ as --32 --gstabs -o movsxtest.o movsxtest.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o movsxtest movsxtest.o
  $gdb -q movsxtest
  ```

  执行结果图

  ![image-20210306224749747](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306224749747.png)

  ![image-20210306224825192](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306224825192.png)

  分析：MOVSX指令把CX寄存器传送给EAX寄存器之后，EAX寄存器包含的值为0xFFFFFFB1，可见MOVSX指令正确的为这个值添加了高位部分的1.

### 程序十三： movsxtest2.s

* 源代码：

  ```
  #movsxtest2.s - Another example using the MOVSX instruction
  .setion .text
  .globl _start
  _start:
  	nop
  	movw $79,%cx
  	xor %ebx,%ebx
  	movw %cx,$bx
  	movsx %cx,%eax
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  ```

* 通过调试监视程序的运行，并且查看寄存器的值是怎么变化的，命令行如下：

  ![image-20210306233916684](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306233916684.png)

  ![image-20210306233945887](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306233945887.png)

  ![image-20210306234020585](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210306234020585.png)

  分析：MOVSX指令正确地使用零填充了EAX寄存器，生成了正确的32位带符号数整数值。

### 程序十四： quadtest.s

* 知识点：.quad命令可以定义一个或者多个带符号整数值，但是为每一个值分配8个字节。

* 源代码：

  ```
  #quadtest.s - An example fo quad integers
  .setion .data
  data1:
  	.int 1,-1,463345,-333252322,0
  data2:
  	.quad 1,-1,463345,-333252322,0
  .section .text
  .globl _start
  _start:
  	nop
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  ```

  

* 通过调试监视程序的运行，并且查看data1和data2数组的十进制值，命令行如下：

  ```
  $ as --32 --gstabs -o quadtest.o quadtest.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o quadtest quadtest.o
  $gdb -q quadtest
  ```

  运行结果图：

  ![image-20210307015600177](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307015600177.png)

  ![image-20210307015618866](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307015618866.png)
  
  分析：运行结果正如我们预料的那样，data1每个数组元素使用4个字节，并且按照小尾数格式存放。data2的数据值是使用四字编码的，所以每个值使用8个字节。

### 程序十五：mmxtest.s

* 知识点：MMX寄存器是64位的寄存器，可以使用MOVQ指令把数据传送到MMX寄存器中。MOVQ的指令格式如下;

  ```
  movq source,destination
  ```

  其中source和destination可以是MMX寄存器，SSE寄存器或者64位的内存位置。

* 源代码：

  ```
  #mmxtest.s - An example if using the MMX data types
  .section .data
  values1:
  	.int 1,-1
  values2:
  	.byte 0x10,0x05,0xff,0x32,0x47,0xe4,0x00,0x01
  .setion .text
  .globl _start
  _start:
  	nop
  	movq values1,%mm0
  	movq values2,%mm1
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  
  ```

* 通过调试监视程序的运行，单步运行到MOVQ指令后，可以显示MM0和MM1寄存器中的值，命令行如下：

  ```
  $ as --32 --gstabs -o mmxtest.o mmxtest.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o mmxtest mmxtest.o
  $gdb -q mmxtest
  ```

  运行结果图如下：

  ![image-20210307105048013](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307105048013.png)

  ![image-20210307105115201](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307105115201.png)

  分析：可以观察到MOVQ指令将定义的两个数值的值加载到前2个MMX寄存器中。

### 程序十六：ssetest.s

* SSE寄存器是128位的寄存器，MOVDQA/MOVDQU指令用于把128位数据传送到XMM寄存器中，或者在XMM寄存器之前传输数据，MOVDQA/MOVDQU指令的指令格式如下：

  ```
  movdqa/movdqu source destination
  ```

  source/destination可以是SSE128位寄存器或者128位的内存位置，不能用于两个内存位置之间传输数据

* 源代码：

  ```
  #ssetest.s - An example of using 128-bit SSE registers
  .section .data
  values1:
  	.int 1,-1,0,135246
  values2:
  	.quad 1,-1
  .section .text
  .globl _start
  _start:
  	nop
  	movdqu values1,%xmm0
  	movdqu values2,%xmm1
  	
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  ```

* 通过调试监视程序的运行，使用print命令显示SSE寄存器的值，命令行如下：

  ```
  $ as --32 --gstabs -o ssetest.o ssetest.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o ssetest ssetest.o
  $gdb -q ssetest
  ```

  运行结果图：

  ![image-20210307112927780](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307112927780.png)

  ![image-20210307113012263](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307113012263.png)

  分析：数据段中的数据成功传输到了xmm0和xmm1寄存器中

### 程序十七 bcdtest.s

* 知识点：FBLD指令用于演示把BCD值加载到FPU寄存器堆栈中，具体格式为

  ```
  fbld source
  ```

  FPU寄存器有别于通用寄存器，8个FDU寄存器的行为类似于内存中的堆栈区域。ST0引用位于堆栈顶部的寄存器。当值被压入FPU寄存器堆栈时，它被存放在ST0寄存器中，ST0中原来的值被加载到ST1中。

* 源代码：

  ```
  #bcdtest.s - AN example of using BCD integer values
  .section .data
  data1:
  	.byte 0x34,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  data2:
  	.int 2
  .section .text
  .globl _start
  _start:
  	nop
  	fbld data1
  	fimul daa2
  	fbstp data1
  
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  
  ```

  在这里FBLD指令的作用就是把data1中的值加载到FPU寄存器堆栈的顶部，使用FIMUL指令把ST0寄存器和data2所在位置中的整数值相乘。最后使用FBSTP指令把堆栈中新的值传送回data1所在的内存位置中。

* 调试监视程序的执行，命令行如下：

  ```
  $ as --32 --gstabs -o bcdtest.o bcdtest.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o bcdtest bcdtest.o
  $gdb -q bcdtest
  ```

  运行截图：

  ![image-20210307161250470](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307161250470.png)

  在执行命令之前先查看内存处data1的值，

  ![image-20210307161337757](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307161337757.png)

  单步执行FBLD后查看ST0寄存器中的值，

  ![image-20210307161410741](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307161410741.png)

  ![image-20210307161433045](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307161433045.png)

  单步执行FLMUL后，再次查看寄存器，寄存器中的值与data2中的2相乘后变为2468

  ![image-20210307161557767](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307161557767.png)

  ![image-20210307161617351](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307161617351.png)

  程序执行结束，查看data1中的值，可见data1的值为新的结果值（翻倍）：

  ![image-20210307161838033](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307161838033.png)

### 程序十八：floattest.s

* 知识点：

  1. fld指令用来把浮点值传送入和传送出FPU寄存器，FLD指令格式是

     ```
     fld source
     ```

     

     其中source可以是32位，64位或者80位内存位置

     flds用于加载或者获取单精度数字，fldl用于加载或者获取双精度数字

  2. fst用于获取FPU寄存器堆栈中顶部的值，并将该值存入内存位置，对于单精度数字，指令为fsts,对于双精度数字，指令为fstl

* 源代码：

  ```
  #floattest.s - An example of using floating piint numbers
  .section .data
  
  value1:
  	.float 12.34
  value2:
  	.double 2343.631
  .section .bss
  	.lcomm data,8
  .section .text
  .globl _start
  _start:
  	nop
  	flds value1
  	fldl value2
  	fstl data
  
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  
  ```

* 通过调试，在执行指令时监视内存位置和寄存器值，命令行如下：

  ```
  $ as --32 --gstabs -o floattest.o floattest.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o floattest floattest.o
  $gdb -q floattest
  ```

  查看浮点数在内存中是怎么存储的：

  ![image-20210308084740204](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308084740204.png)

  ![image-20210308084827079](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308084827079.png)

  单步运行第一条FLDS指令后，使用info reg或者print指令查看ST0寄存器的值：然后再单步运行下一条指令并查看ST0和ST1寄存器的值

  ![image-20210308085122613](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308085122613.png)

  可见：ST0的值下移到了ST1寄存器中

  单步执行FSTL指令后查看data中的值

  ![image-20210308085336744](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308085336744.png)

  可见：FSTL指令把ST0寄存器中的值加载到了data标签指向的内存位置。

### 程序十九：fpuvals.s

* IA-32指令集包括一些预置的浮点数，

  | 指令   | 描述                               |
  | ------ | ---------------------------------- |
  | FLD1   | 把+1.0压入FPU堆栈中                |
  | FLDL2T | 把10的对数（底数为2）压入FPU堆栈中 |
  | FLDL2E | 把e的对数（底数为2）压入FPU堆栈中  |
  | FLDPI  | 把pi压入FPU堆栈中                  |
  | FLDLG2 | 把2的对数（底数为10）压入FPU堆栈中 |
  | FLDLN2 | 把2的对数（底数为e）压入FPU堆栈中  |
  | FLDZ   | 把+0.0压入FPU堆栈中                |

* 源代码：

  ```
  #fpuvals.s -An example pf pushing floating point constants
  .section .text
  .globl _start
  _start:
  	nop
  	fld1
  	fld12t
  	fld12e
  	fldpi
  	fldlg2
  	fldln2
  	fldz
  	movl $1, %eax
  	movl $0, %ebx
  	int $0x80
  ```

  

* 对程序进行汇编，再调试器中运行它。命令行如下

  ```
  $ as --32 --gstabs -o floattest.o floattest.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o floattest floattest.o
  $gdb -q floattest
  ```

  ![image-20210308091120721](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308091120721.png)

  从而在执行指令时监视FPU寄存器堆栈，单步运行到所有预置指令执行后，使用info all 查看堆栈中的值，调试结果图如下：

  ![image-20210308091144599](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308091144599.png)

  ![image-20210308091223378](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308091223378.png)

  观察可见:值的顺序和它们被存入堆栈的顺序是相反的，符合堆栈的特点

### 程序二十： ssefloat.s

* 知识点：SSE技术引进了8个128位XMM寄存器，可以使用这些寄存器打包浮点数（4位单精度浮点数，2位62位的双精度的浮点数）

  movups 用来把4个不对准的打包单精度值传送给XMM寄存器或者内存

* 源代码：

  ```
  #ssefloat.s - An example of moving SSE FP data types
  .section .data
  value1:
  	.float 12.34,2345.543,-3493.2,0.44901value2:
  	.float -5439.234,32121.4,1.0094,0.000003
  .section .bss
  	.lcomm data,16
  .section .text
  .globl _start
  _start:
  	nop
  	movups value1,%xmm0
  	movups value2,%xmm1
  	movups %xmm0,%xmm2
  	movups %xmm0,data
  
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  
  ```

* 通过调试监视程序的运行，单步运行前三个指令后，使用print命令显示XMM寄存器的值，命令行如下：

  ```
  $ as --32 --gstabs -o ssefloat.o ssefloat.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o ssefloat ssefloat.o
  $gdb -q ssefloat
  ```

  运行结果图：

  ![image-20210307153825208](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307153825208.png)

  ![image-20210307154105277](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307154105277.png)

  ![image-20210307154124339](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307154124339.png)

  分析：观察图片可以看到movups指令成功的将value1中 4位浮点数传送给%xmm0寄存器，成功将value2中4位浮点数复制给%xmm1，将%xmm0中的 4位浮点数传送给%xmm2寄存器.

  

### 程序二十一： sse2float.s

* 知识点：

  movupd 用来把2个不对准的打包双精度值传送给XMM寄存器或者内存

* 源代码：

  ```
  # sse2float.s - An sample of moving SSE2 FP data types
  .section .data
  value1:
  	.double 12.34,2345.543
  value2:
  	.double -5439.234,32121.4
  .section .bss
  	.lcomm data,16
  .section .text
  .globl _start
  _start:
  	nop
  	movupd value1,%xmm0
  	movupd value2,%xmm1
  	movupd %xmm0,%xmm2
  	movupd %xmm0,data
  
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  
  ```

* 通过调试监视程序的运行，单步运行前三个指令后，使用print命令显示XMM寄存器的值，命令行如下：

  ```
  $ as --32 --gstabs -o sse2float.o sse2float.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o sse2float sse2float.o
  $gdb -q sse2float
  ```

  运行结果图：

  ![image-20210307154727205](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307154727205.png)

  ![image-20210307154737670](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307154737670.png)

  ![image-20210307154752158](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210307154752158.png)

  分析：观察图片可以看到movupd指令成功的将value1中 2位双精度值传送给%xmm0寄存器，成功将value2中2位双精度浮点数复制给%xmm1，将%xmm0中的 2位双精度浮点数传送给%xmm2寄存器.

  

### 程序二十二：convtest.s

* 知识点：

  cvtps2dq -> 打包单精度FP到打包双字整数（XMM）

  cvttps2dq -> 打包单精度FP到打包双字整数（XMM，截断）

  cvtdq2ps -> 打包双字整数到打包单精度FP（XMM）

* 源代码：

  ```
  #convtest - An example of data conversion
  .section .data
  value1:
  	.float 1.25,124.79,200.0,-312.5
  value2:
  	.int 1,-435,0,-25
  .section .bss
  data:
  	.lcomm data,16
  .section .text
  .globl _start
  _start:
  	nop
  	cvtps2dq value1,%xmm0
  	cvtps2dq value1,%xmm1
  	cvtdq2ps value2,%xmm2
  	movdqu %xmm0,data
  
  	movl $1,%eax
  	movl $0,%ebx
  	int $0x80
  ```

* 进行通常的汇编和调试以后，查看XMM寄存器和data中内存的值，来观察这些指令是如何执行的：

  ![image-20210308094138989](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308094138989.png)

  ![image-20210308094203367](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308094203367.png)

  ![image-20210308094226785](http://hurq5.gitee.io/os-labwork/LabWeek03/pictures/image-20210308094226785.png)

  

  分析：可以观察到

  1. value1中的单精度值被成功打包成双字整数存储在寄存器%xmm0中（浮点数进行四舍五入转化成整数）

     即，v4_int32={1,125,200,-312}

  2. value1中的单精度值被成功打包成双字整数存储在寄存器%xmm1中（截断,向0舍入）

     即，v4_int32={1,124,200,-312}

  3. value2中的双精度值被成功打包成双字整数存储在寄存器%xmm1中，即v4_float={1,-435,0,-25}

  4. data的值被转换成打包双字整数。