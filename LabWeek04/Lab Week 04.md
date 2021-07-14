# ladweek04 - Assembly Languang

## 实验内容

验证实验 Blum’s Book: Sample programs in Chapter 08, 10 (Basic
Math Functions and Using Strings)
## 实验报告
相关内容的技术日志、遇到问题和解决方案。

## 实验验证及知识点

### 实验一：addtest1.s

* 知识点：

  add指令用于把两个整数相加：

  ```
  add source,destination
  ```

  1. 其中source可以是立即数，内存位置或者寄存器，destination可以是寄存器或着内存位置中存储的值
  2. 用于不同位数的相加，需要添加助记符，b(用于字节)，w(用于字)，l(用于双字)

* 使用以下代码调试程序，并单步执行指令直到所有的程序指令执行结束

  ```
  $ as --32 --gstabs -o addtest1.o addtest1.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o addtest1 addtest1.o
   $gdb -q addtest1
  ```

  ![image-20210312104709347](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312104709347.png)

  ![image-20210312104838642](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312104838642.png)

  查看寄存器中的值：

   ![image-20210312104910104](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312104910104.png)

  观察结果和程序运行的预计结果相同

  * %eax中的值等于30（al)+40(data)=70
  * data内存位置的值等于70（%eax)+40(data)=110
  * %ecx中的值等于100（100->cx)
  * %edx中的值等于200（该寄存器被赋值为100后，与自身相加，使自身翻倍）

  

### 实验二：addtest2.s

* 使用以下代码调试程序，并单步执行指令直到所有的程序指令执行结束

  ```
  $ as --32 --gstabs -o addtest2.o addtest2.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o addtest2 addtest2.o
   $gdb -q addtest2
  ```

  ![image-20210312110814281](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312110814281.png)

  ![image-20210312110919315](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312110919315.png)

  查看相关寄存器中的值

  ![image-20210312110956783](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312110956783.png)

  观察结果和程序运行的预计结果相同

  ```
  	movl $-10,%eax 
  	#%eax=-10
  	movl $-200,%ebx
  	#%ebx=-200
  	movl $80,%ecx
  	#%ecx=80
  	addl data,%eax
  	#%eax=(-10)+(-40)=-50
  	addl %ecx,%eax
  	#%eax=80+(-50)=30
  	addl %ebx,%eax
  	#%eax=-200+30=-170
  	addl %eax,data
  	#data=-170+(-40)=-210
  	addl $210,data
  	#data=210+(-210)=0
  ```

  ​		由执行结果图得到

  * %eax中的值等于-170
  * data内存位置的值等于0
  * %ecx中的值等于80
  * %ebx中的值等于-200

  这与我们进行代码分析的结果相同

  

### 实验三：addtest3.s

* 知识点：

  当整数相加时，应该注意EFLAGS寄存器

  * 对于无符号数，当二进制加法照成进位情况时，进位标志位会被设置为1
  * 对于带符号数，当出现溢出情况时，溢出标志位会被设置为1.

* 运行代码 ，用echo命令查看结果代码：

  ```
  $ as --gstabs -o addtest3.o addtest3.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o addtest3 addtest3.o
   $./addtest3
   $echo $?
  ```

  ![image-20210312125027655](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312125027655.png)

  

  分析：程序对存储在AL和BL寄存器中的2字节无符号整数值执行简单的加法。190+100=290超过了255位，因此产生了进位，JC指令将跳转到标签over，影刺结果为0

  现在改动寄存器使其不产生进位

  ```
  将代码中的100改为10
  ```

  运行程序得到结果如下：

  

  ![image-20210312125821729](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312125821729.png)

  可见加法没有产生进位，没有发生跳转，并且加法的结果被设置为结果代码

### 实验四：addtest4.s

* 知识点：带符号数相加，当结果溢出正值或负值界限时，溢出标志位会被设置为1

* 执行程序，观察输出的结果：

  ```
  $ as --32 -o addtest4.o addtest4.s
  $ ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o addtest4 -lc addtest4.o
   $./addtest4
   $echo $?
  ```

  ![image-20210312132330783](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312132330783.png)

  分析：两个大的负数相加，这造成了溢出情况。JO指令用于检查溢出并且把控制传递给标签over。因此结果输出0

  将两个负数的值改成如下，使得两个值的 相加不产生溢出情况：

  ```
  movl $-190876934，%ebx
  movl $-159239143,%eax
  ```

  执行结果如下，可以看到输出的结果为加法的结果：

  ![image-20210312132918801](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312132918801.png)

### 实验五：adctest.s

* 知识点：

  adc指令执行两个无符号或者带符号整数值的加法，并且把前一个ADD指令产生的进位标志的值包含在其中。

  adc指令的格式如下：

  ```
  adc source destination
  ```

  和add一样adc针对不同位数的操作数有其固定的助记符。（b,w,l)

* 汇编链接文件后，运行调试器来监视处理过程种的各个步骤。

  ```
  $ as --32 --gstabs -o adctest.o adctest.s
  $ ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2 -o adctest -lc adctest.o
   $gdb -q adctest
  ```

  ![image-20210312144027004](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312144027004.png)

  单步执行指令，当加载指令执行完毕后查看操作数是否加载到相应的寄存器中：

  ![image-20210312144158895](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312144158895.png)

  可以观察到data1的值的低32位被成功加载到EBX寄存器中，高32位被成功加载到EAX寄存器中，相同的方式，data2被成功加载到了ECX：EDX寄存器中

  单步执行完加法指令后，再次查看寄存器中的值

  ![image-20210312144813337](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312144813337.png)

  可以观察到执行加法操作后64位的结果被成功的保存在了ECX：EDX寄存器中。

* 直接运行指令观察结果值是否正确，如下图，可见64位的加法结果是正确的：

  ![image-20210312145649544](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312145649544.png)

### 实验六： subtest1.s

* 知识点：sub可以用急无符号整数或者带符号整数，具体指令格式如下：

  ```
  sub source destination
  ```

  和add一样adc针对不同位数的操作数有其固定的助记符。（b,w,l)

* 使用以下代码调试程序，并单步执行指令直到最后一条减法指令未被执行，重点观察最后一条指令执行的操作数值和结果值

  ```
  $ as --32 --gstabs -o subtest1.o subtest1.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o subtest1 subtest1.o
   $gdb -q subtest1
  ```

  ![image-20210312151135553](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312151135553.png)

  ![image-20210312151222513](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312151222513.png)
  
  分析：可见最后一条指令SUB，它把内存位置data1的值（40）减去EAX寄存器中的值（-30）;处理器从40中减去-30，并且得到正确的结果70.

### 实验七： subtest2.s

* 对程序进行汇编后，运行并查看结果寄存器中的值

  ![image-20210312230636828](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312230636828.png)

  分析：程序简单的把5存放到EAX寄存器中，把值2存放在EBX寄存器中，然后从EBX寄存器的值中减去EAX寄存器的值，结果小于0，这在带符号整数中是非法的，进位标志被设置为1.程序发生跳转，程序的结果代码为0

* 使用以下代码调试程序，并单步执行指令直到减法指令被执行，观察寄存器中ebx的值：

  ```
  $ as --32 --gstabs -o subtest2.o subtest2.s
  $ ld -m elf_i386 -dynamic-linker/lib/ld-linux.so.2 -o subtest2 subtest2.o
   $gdb -q subtest2
  ```

  ![image-20210312231514133](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312231514133.png)

  ![image-20210312231557956](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210312231557956.png)

  分析: 尽管运行的结果超过了无符号值的范围，但是处理器并不能够识别，因此结果常为负数。

### 实验八：subtest3.s

* 对程序进行汇编后，运行并查看执行的结果

  ![image-20210313113024386](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210313113024386.png)

  分析：存储在EBX寄存器中的负值减去存储在EAX寄存器中的正值，生成一个超过32位EBX寄存器范围的值。JO指令用于检测溢出标志，并且把程序转到over，把输出设置为0

* 将%eax寄存器中的值该为负值，运算结果不发生溢出，指令不发生跳转，输出运算结果

  ![image-20210313114023601](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210313114023601.png)

### 实验九: sbbtest.s

* 知识点：sbb使用进位情况帮助执行大的无符号整数值的减法操作，指令格式如下：

  ```
  sbb source，destination
  ```

  和add一样sbb针对不同位数的操作数有其固定的助记符。（b,w,l)

* 对程序进行汇编后，运行并查看执行的结果

  ![image-20210313115002126](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210313115002126.png)

  分析：和adctest.s的执行过程类似，通过使用四个寄存器以及sbb指令成功的实现了大无符号数整数值的减法操作，结果 正确的输出了data2减去data1的结果。

* 对递增递减知识点的补充：

  使用递增命令inc和递减指令dec对操作数进行加一和减一操作，执行格式如下：

  ```
  dec destination
  inc destination
  ```

  其中destination可以是寄存器或着内存位置中存储的值

### 实验十：multest.s

* 知识点：和加法减法不同，乘法指令对于无符号数和有符号数有着不同的指令。

  MUL指令用于两个无符号整数的相乘

  * 对于8位的源操作数，目标操作数为AL，使用AX寄存器储存16位的结果
  * 对于16位的源操作数，目标操作数为AX，使用DX：AX寄存器保存32位的结果
  * 对于32位的源操作数，目标操作数为EAX，采用64位的EDX：EAX寄存器。

* 对程序进行汇编，并且把它和C库相互连接后，直接运行它查看输出，在调试过程中查看执行过程中寄存器的情况，MUL指令执行后，调制器的输出如下：

  ![image-20210313142610736](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210313142610736.png)

  ![image-20210313142643342](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210313142643342.png)

  分析：程序将data1的值加载到EAX寄存器中，然后使用MUL指令把另一个值和EAX寄存器中的值相乘。并且将EDX；EAX寄存器对的值加载到一个64位的内存位置result中。

### 实验十一：imultest.s

* 知识点：imul指令可以用于带符号和无符号整数，但不使用目标的最高有效位，指令有三种格式

  * 第一种格式

    ```
    imul source
    ```

    source操作数可以是8位，16位，32位寄存器或者内存中的值，它与位于AL，AX或者EAX寄存器的隐含操作数相乘。然后，结果放在AX寄存器，DX：AX寄存器对或者EDX：EAX寄存器对中。

  * 第二种格式

    ```
    imul source,destination
    ```

    其中source可以是16位或者32位寄存器或者内存中的值，destination必须是16位或者32位通用寄存器。

  * 第三种格式

    ```
    imul multiplier，source，destination
    ```

    其中multiplier是一个立即值，source是16位或者32位寄存器或内存中的值，destination必须是16位或者32位通用寄存器。

  和add指令一样imul指令也需要添加助记符

* 汇编和连接程序之后，可以使用调试器监视程序执行过程中寄存器的值，单步执行IMUL指令后，寄存器如下：

  ![image-20210313145321452](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210313145321452.png)

  ![image-20210313145345775](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210313145345775.png)

  分析：EAX寄存器包含EDX寄存器的值（400）和立即值2相乘得到的结果，ECX寄存器包含EBX寄存器的值（10）和最初加载到ECX寄存器中的值（-35）相乘的结果。

### 实验十二：imultest2.s

* 使用JO指令检查溢出标志。

* 汇编执行程序，查看结果寄存器中的值：

  ![image-20210313150345806](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210313150345806.png)

  

  分析：程序把680传送到AX寄存器中，把100传送到CX寄存器中，使用16位的IMUL指令将两者相乘，结果为68000大于65535，导致寄存器的溢出，因此JO指令发生跳转，跳转到over标签处，因此结果代码为1.

* 修改立即数100为10，使得结果不大于65535，

  ![image-20210313150919292](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210313150919292.png)

  分析：可见JO指令没有发生跳转，结果代码为0

### 实验十三. divtest.s

* 知识点：

  DIV指令用于无符号整数的除法操作，指令格式如下：v

  ```
  div divisor
  ```

  约束条件同mul

  商和余数的存储状况如下：

  ![image-20210314010536583](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314010536583.png)

* 汇编和连接程序之后，可以使用调试器监视程序执行过程中寄存器的值，单步执行DIV指令后，寄存器如下：

  ![image-20210314010233253](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314010233253.png)
  
  分析：程序将以为64位的四字整数8335加载到EDX：EAX寄存器对中，然后使用一个存储在内存中的32位双字整数25除以这个值，得到的商333存储在eax寄存器中，余数10存储在EDX寄存器中

### 实验十四：saltest.s

* 知识点：SAL用于逻辑左移，SHL用于逻辑右移

  它们的3个格式如下：

  ```
  sal destination
  sal %cl,destination
  sal shifter,destination
  ```

  第一个版本destination值左移1位

  第二个版本destination值左移cl寄存器中指定的位数

  第三个版本destination值左移shifter值指定的位数。

  （说明，移位造成的空位用零填充，移位造成的超出数据长度的首位，先是被存放在进位标志中，然后再下一次移位操作中被丢弃。）

* 汇编和连接程序之后，可以使用调试器监视程序执行过程中寄存器的值，单步执行所有指令后，查看寄存器的值，如下：

  ![image-20210314012443070](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314012443070.png)

  ![image-20210314012648450](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314012648450.png)

  ![image-20210314012710718](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314012710718.png)

  分析:十进制10被加载到EBX寄存器中，第一条sal指令把它移动1位，使之乘以2，结果为20。第二条sal指令把它移动2位，使之乘以4，结果为80。第三条sal指令把它再移动2位，结果乘以4变成80。value1位置中的值（25）被移动1位（50），然后再移动2位，使之为200.

### 实验十五：dastest.s

* 知识点：

  处理打包BCD值时，可用的指令只有2条：

  * DAA：调整ADD或者ADC指令的结果
  * DAS：调整SUB或者SBB指令的结果

  

* 汇编和连接程序之后，可以在调试器中运行它，并且随着通过SBB指令计算减法值，然后通过DAS指令把它转变为打包BCD格式，监视EAX的值，第一个减法指令操作后，EAX寄存器的值如下：

  ![image-20210314143846048](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314143846048.png)

  执行第一条减法操作后，EAX寄存器的值如下

  ![image-20210314143906860](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314143906860.png)

  施行王DAS后该值调整为如下：

  ![image-20210314143922726](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314143922726.png)

  它表示的是结果的第一个十进制位

### 实验十六：cpuidtest.s

* 知识点：

  TEST指令在8位，16位或者32位值之间执行按位逻辑AND操作，并且相应地设置符号，零和奇偶校验标志，而且不修改目标值。

* 执行结果如下：

  ![image-20210314151933601](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314151933601.png)

  分析：程序首先使用PUSHFL指令把EFLAGS客存器的值保存到堆栈项部。然后,.使用POPL指令把EFLAGS值提取到EAX寄存器中。下-个步骤演示如何使用XOR指令设置寄存器的一位。使用MOVL指令把EFLAGS值的拷贝保存到EDX客存器中,然后使用XOR指令设置ID位(仍然代EAX寄存器中)为值L。XOR指令使用一个设置个ID位的立即值。EAX寄存器经过亦或操作之后,就确保ID位被设置为1了。下一个步骠把新的EAX寄存器值压人到堆栈中,然后使用POPFL指令把它存停在EFLAGS寄存器中。现在必须确定是否成功地设置了ID标志、再一次使用PUSHFL指令把EFLAGS寄存器压人堆栏,然后使用POPL指令把它弹出到EAX寄存器中。这个值和原始的EFLAGS值(先前存储在EDX寄存器中)逃行XOR撷作,查看值改变成了什么。

  最后,使用TEST指令查看ID标志位是否改变了。如果是,那么EAX中的值就不为零,然后使用INZ指令迸行眼转,输出适当的消息。

### 实验十七：movstest1.s

* 知识点：

  * movs指令的作用是位程序员提供把字符串从一个内存位置传送到另一个内存位置的简单途径。

    指令有以下三种格式：

    ```
    * MOVSB:传送单一字节
    
    * MOVSW:传送-个字(2字节}
    
    * MOVSL;传送一个双字(4字市)
    ```

    Intel文格使用MOVSD传送双字。GNU汇编器决定使用MOVSL。

    MOVS指令使用隐含的源和目标操作数。隐含的源操作数是ESI寄存器。它指向源字符串的内存位置。隐含的目标操作数是EDI寄存器。它指向守符串要被复制到的目标内存位置。记住操作数顺序的好方法是ESI中的代表源(source),而EDI中的D代表目标(destination)。

  * 内存位置的地址被加载到了ESI或者EDI官存器中有两种方法，

    1. 一种是使用movl指令：

       ```
       mov1 $output,%edi
       ```

       这条指令把output标签的32位内存位置传送给EDI寄存器。

    2. 另一种方式是LEA指令。LEA指后加载一个对象的有效地址。指令

       ```
       leal outpat,%edi
       ```

       该指令把output标签的32位内存位置加载到EDI寄存器中。

  * 汇编和连接程序之后，可以在调试器中运行它，在每条movs指令执行过后监视内存位置output的值，如下：![image-20210314154154517](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314154154517.png)

    ![image-20210314154214099](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314154214099.png)

    分析：程序把内存位置 value l的位置加载到ESI寄存器中,把 output内存位置的位置加载到ED寄存器中。

    1. 当执行MOⅤSB指令时,它把1字节的数据从 value位置传送到 output位置。即把“T”从value1位置传送到output位置。
    2. 当执行MOⅤSW指令时,它把接下来的2字节的数据传送到 output位置。即把“hi”从value1位置传送到output位置。
    3. 当执行MOⅤSW指令时,它把接下来的4字节的数据传送到 output位置。即把“s is”从value1位置传送到output位置。

### 实验十八：movstest2.s

* 知识点：

  如果DF标志被清零,那么毎条MOVS指令执行之后ESI和EDI寄存器就会递增。如果DF标志被设置,那么每条MOVS指令执行之后ESI和EDI寄存器就会递减。为了确保DF标志被设置为正确的方向,可以使用下面的命令

  ```
  CLD用于将DF标志清零
  STD用于设置DF标志
  ```

  使用STD指令时,ESI和ED寄存器在每条MOⅤS指令执行之后递减,所以它们应该指向字符串的末尾,而不是开头。

* 调试代码，在3条movs指令执行过后，可以使用调试器查看内存位置output的值：

  ![image-20210314155447134](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314155447134.png)

### 实验十九：movstest3.s

* 调试代码，循环执行movsb指令，直到整个字符串传送完毕，查看内存位置output的字符串值可以看到以下的结果：

  ![image-20210314160032734](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314160032734.png)

  。。。。。

  ![image-20210314160052254](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314160052254.png)

### 实验二十：reptest1.s

* 知识点：rep指令的特殊之处在于它自己不执行什么操作，而用于按照特定次数重复执行字符串指令。

* 调试代码，单步执行程序直到rep指令恰好被执行，查看内存位置output的字符串值可以看到以下的结果：

  ![image-20210314160916784](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314160916784.png)

  分析：可以看到rep循环传输单字节23次，将源字符串中的字节都传送导论目标字符串的位置。

  

### 实验二十一：reptest2.s

* 使用MOVSW指令和MOVSL指令遍历字符串时，小心不要超过字符串的边界

* 调试代码，单步执行程序直到rep指令恰好被执行，查看内存位置output的字符串值可以看到以下的结果：

  ![image-20210314161318054](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314161318054.png)

  可以观察到：最后一次执行MOVSL指令时，它不仅获得了value1字符串的末尾，而且会错误地获得定义的下一个字符串的一个字节的数据。

### 实验二十二:  reptest3.s

* 当我们直到字符串的长度时，可以执行整数除法，以确定字符串中包含多少个双字，然后余数部分可以使用MOVSB指令进行传送。

* 在调试器中监视程序的执行情况，首先在执行REP MOVSL指令组合之后停止程序显示内存位置buffer的内容：

  ![image-20210314171246452](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314171246452.png)

  ![image-20210314171314855](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314171314855.png)

### 实验二十三：reptest4.s

* 和 mostest2s程序类似, reptest4.s程序把源和目标字符串的末尾的位置加载到ESl和EDI寄存器中,然后使用STD指令设置DF标志。这使目标字符串按照相反的顺序被存储(尽管无法通过查看调试器验证这一点,因为REP指令仍然在一个步骤中传送所有的字节)

  ![image-20210314171846933](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210314171846933.png)

### 实验二十四：stostest1.s

* 知识点:

  **LODS指令用于把内存中的字符串值传送到EAX寄存器中。**

  1. LODS指令有3种不同格式：
     * LODSB：把一个字节加载到AL寄存器中
     * LODSW：把一个字（2字节）加载到AX寄存器中
     * LODSL：把一个双字（4字节）加载到EAX寄存器中（Intel文档使用LODSD加载双字。GNU汇编器使用LODSL）
  2. LODS指令使用ESI寄存器作为隐含的源操作数。ESI寄存器必须包含要加载的字符串所在的内存地址。
  3. 数据传送完成之后， LODS指令按照加载的数据的数量递增或者递减（取决于DF标志状态） ESI寄存器。

  **STOS指令把EAX寄存器存放到一个内存位置中。**

  1. STOS指令有3种格式：
     * STOSB：存储AL寄存器中一个字节的数据
     * STOSw：存储AX寄存器中一个字（2字节）的数据
     * STOSL：存储EAX寄存器中一个双字（4个字节）的数据
  2. STOS指令使用EDI寄存器作为隐含的目标操作数。
  3. 执行STOS指令时，它按照使用的数据长度递增或者递减EDI寄存器的值

* 使用调试器查看执行rep stosb前后内存位置buffer的值：

  ![image-20210315082936787](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210315082936787.png)

  分析：从输出可以观察到LODSB把空格字符成功加载到了AL寄存器

  ![image-20210315083004166](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210315083004166.png)

  分析：输出结果和预期相同，在rep stosb指令执行前，内存位置buffer包含零（.bss段中定义的变量初始化为0），在rep stosb指令执行后，内存位置buffer包含空格。

### 实验二十五：convert.s

* 对程序进行汇编并且把它和C库连接起来，运行查看结果是否正确：

  ![image-20210315084228500](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210315084228500.png)

  分析：程序把内存位置string1加载到ESI和EDI寄存器中，把字符串长度加载到ECX寄存器中。然后程序使用LOOP指令对字符串中的每个字符执行字符检查。

  程序进行字符检查的方法是，把每个字符加载到AL寄存器中，并且判断它是否小于字母a的ASCI值（0x61），或者大于字母2的ASCI值（Ox7a），如果字符在这个范围之内，那么它必然是小写字母，必须通过减去0x20把它转换为大写字母。

  从而使得输出为均为大写字母的语句

### 实验二十六：cmpstest1.s

* 知识点：

  CMPS指令系列用于比较字符串值，和其他字符串指令一样，

  1.  CMPS指令有3种格式
     * CMPSB:比较字节值
     * CMPSW:比较字(2字节)值
     * CMPSL:比较双字(4字节)值
  2. 隐含的源和目标操作数的位置同样存储在ESI和EDI寄存器中。
  3. 每次执行CMPS指令时，根据DF标志的设置， ESI和EDI寄存器按照被比较的数据的长度递增或者递减CMPS指令从源字符串中减去目标字符串，并且适当地设置EFLAGS寄存器的进位、符号、溢出、零、奇偶校验和辅助进位标志.

* 汇编连接程序之后，通过运行它并且查看结果代码来检测程序的成功执行：

  ![image-20210315090151789](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210315090151789.png)

  分析：

  程序首先把exit系统调用值加载到EAX寄存器中。把要测试的两个字符串的位置加载到ESI和EDI寄存器中之后， cmpstestl.s程序使用CMPSL指令比较字符串的前4个字节。如果字符串相等，就使用E指令跳转到标签equal，这里把程序结果代码设置为0并且退出。如果字符串不相等，则不会跳转到分支，程序顺序执行，设置结果代码为1并且退出。

  我们观察到结果代码为0，表示字符串相互匹配，符合我们对结果的期待

### 实验二十七：cmpstest2.s

* rep指令在检查字符串匹配过程中，只会关心ecx寄存器中的计数值，而不会在不匹配时停止匹配过程

  为了优化程序，我们使用repe，repne，repz，repnz。这些指令在每次重复过程中会检查零标志位，如果零标志被设置，就停止重复。

  这样我们就方便设置程序在遇到不匹配的字符对的时候，rep指令停止重复。

* 汇编连接程序之后，通过运行它并且查看结果代码来检测程序的成功执行：

  ![image-20210315091024770](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210315091024770.png)

  分析：程序匹配到"insruction"的‘i'和“Insruction"的’I'时字符串出现不匹配，将EXC寄存器设置为0，循环结束，结果代码等于计数寄存器的结果11.

### 实验二十八：strcomp.s

* 知识点：

  **词典式顺序的基本规则**

  1. 按字母表顺序,较低的字母小于较高的字母
  2. 大写字母小于小写字母


  **比较长度不同的两个字符串**

  按照长度短一些的字符串中的字符数量进行比较。如果短字符串大于长字符串中相同数量的字符,那么短字符串就大于长字符串。如果短字符串小于长字符串中相同数量的字符,那么短字符串就小于长字符串。如果短字符串等于长字符串中相同数量的字符,那么长字符串就大于短字符串

* 程序分析：

  短的长度值被加载到ECX寄存器中,使用REPE和 CMPSB指令,按照短字符串的长度逐字节地比较两个字符串。

  1. 如果第个字符串大于第二个字符串,程序就跳转到标签 greater位置的分支,设置结果代码为1并且退出
  2. 如果第一个字符串小于第二个字符串,程序就顺序执行,设置结果代码为255并且退出
  3. 如果两个字符串相等,就要做更多的工作程序还必须确定两个字符串中哪个更长。
     * 如果第一个字符串更长,则它更大,程序就跳转到标签 greater置的分支,设置结果代码为1并且
       退出。
     * 如果第二个字符串更长,那么第一个字符串就较小,程序跳转到标签1ess的分支,设置结果代码为255并且退出。
     * 如果两个长度不大于也不小于,那么不仅两个字符串匹配,而且它们的长度也相等,所以字符串相等

* 汇编连接程序之后，通过运行它并且查看结果代码来检测程序的成功执行：

  ![image-20210315104154539](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210315104154539.png)

  分析：我们可以观察到第一个字符串“test”小于第二个字符串“test1”，所以此时程序跳转到标签1ess的分支,设置结果代码为255并且退出

### 实验二十九：scastest1.s

* 知识点：

  SCAS指令系列用于扫描字符串搜索一个或者多个字符。

  1. SCAS指令有3个版本：
     * SCASB：比较内存中的个字节和AL寄存器的
     * SCASW.比较内存中的一个字和AX寄存器的值
     * SCASL:比较内存中的一个双字和EAX寄存器的值
  2. SCAS指令使用EDI寄存器作为隐含的目标操作数．EDI寄存器必须包含要扫描的字符串的内存地址。
  3. 当执行SCAS指令时，EDI寄存器的值按照搜索字符的数据
     长度递增或者递减（这取决于DF标志的值).进行比较时，会相应地设置EFLAGS的辅助进位、进位．奇偶校验、溢出、符号和零标志。
  4. 可以使用标准的条件分支指令检查扫描的结果。
  5. 把SCAS与REPE和REPNE前缀一起使用时，
     * REPE:扫描字符串的字符，查找不匹配搜索字符的字符
     * REPNE:扫描字符串的字符，查找匹配搜索字符的字符

* 程序分析：

  程序把要扫描的字符串的内存位置加载到EDX寄存器中,使用 LODSB指令把要搜索的字符加载到AL寄存器中,然后把字符串的长度存放到ECX寄存器中。所有这些工作完成之后,使用 REPNE SCASB指令扫描字符串,获得搜索字符的位置。如果没有找到这个字符,JNE指令就会跳转到标签 notfound的分支。如果找到了这个字符,那么现在它距离字符串末尾的位置就存放在ECX寄存器中。从ECX的值中减去字符串的长度,然后使用NEG指令改变结果值的符号,以便生成找到的字符在字符串中的位置。这个位置被加载到EBX寄存器中,使它成为程序终止之后的结果代码

* 汇编连接程序之后，通过运行它并且查看结果代码来检测程序的成功执行：

  ![image-20210315112350354](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210315112350354.png)

  分析：输出结果显示在字符串的第16个位置找到“-”字符，和预计的结果相同。

### 实验三十：scastest2.s

* SCASW和SCASL指令可以用于搜索2个或者4个字符的序列
  SCASW和SCASL指令扫描字符串，在每次比较之后，EDI寄存器要么递增2（对于SCASW），要么递增4（对
  于SCASL)，而不是递增1。

  

* 汇编连接程序之后，通过运行它并且查看结果代码来检测程序的成功执行：

  !![image-20210315130621491](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210315130621491.png)

  分析：

  程序试图在字符串中查找字符序列“test”。它把整个搜索字符串加载到EAX寄存器中，然后使用SCASL指令一次检查字符串的4个字节·注意ECX寄存器没有被设置为字符串的长度，而是被设置为REPNE指令遍历整个字符串所需的迭代次数·因为每次迭代检查4个字节，所以ECX寄存器的值是整个字符串长度44的四分之一

  但是我们可以观察到结果并不是上面预料的那样。这是因为string1中的test并没有位于同一个四字节段中，参与比较识别，因此被测试的每一组4个字节都不和搜索字符序列匹配，因此结果代码为0；

### 实验三十一：srisize.s

* 知识点：scas指令一个非常常用的功能就是确定零结尾的字符串长度

* 程序分析：

  分析：s程序把要试的字符串的内存位置加载到EDI寄存器中，把假设的字符串长度加载到ECX寄存器中·字符串长度值0xffff表明这个程序只能用于长度最大为65535的字符串，ECX寄存器将跟踪在字符串中找到表示结尾的零经过了多少次迭代，如果SCASB指令找到了零，就必须通过ECX寄存器的值计算它的位置，从ECX寄存器的值中减去它的初始值，并且改变结果
  的符号，就完成了这个工作．因为这个长度包含表示结尾的零，所以最终值必须减1才能显示字符串真正的长度．计算的结果存放在EBX寄存器中。

* 汇编连接程序之后，通过运行它并且查看结果代码来检测程序的成功执行：

  ![image-20210315135000192](http://hurq5.gitee.io/os-labwork/LabWeek04/pictures/image-20210315135000192.png)

  分析：可以看到程序成功输出字符串string1的字符串长度35.