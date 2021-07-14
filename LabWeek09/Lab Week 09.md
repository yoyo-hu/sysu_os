

## 实验内容：进程间通信—管道和 socket 通信。

### 编译运行课件 Lecture11 例程代码：

* 程序相关知识点：

  1. **access()函数：**

     * 作用：判断是否具有存取文件的权限
     * 函数原型：int access(const char * pathname, int mode);
     * 包含在头文件：#include <unistd.h>中
     * F_OK 则是用来判断该文件是否存在
     * 所有计划查核的权限通过，表示成功返回0 值，只要有一权限被禁止则返回-1。

     

  2. **fcntl()函数：**

     * 作用：用来操作文件描述词的一些特性.
     * 函数原型：int fcntl(int fd, int cmd);
       int fcntl(int fd, int cmd, long arg);
       int fcntl(int fd, int cmd, struct flock * lock);
     * 包含在头文件\#include <unistd.h>   #include <fcntl.h>中
     * F_GETFL： 取得文件描述词状态标识符, 此标识符为open()的参数flags.
       F_SETFL ：设置文件描述词状态标识符, 参数arg 为新标识符。

  3. **write()函数：**

     * 作用：write()会把参数buf所指的内存写入count个字节到参数fd所指的文件内。
     * 函数原型：ssize_t write (int fd, const void * buf, size_t count); 

  4. **gethostbyname()函数：**

     * 作用：只要传递域名字符串，就会返回域名对应的 IP 地址

     * 函数原型：struct hostent *gethostbyname(const char *hostname);

     * 回的地址信息会装入 hostent 结构体，该结构体的定义如下：

       ```c
       struct hostent{
           char *h_name;  //official name
           char **h_aliases;  //alias list同一IP 地址可以绑定多个域名
           int  h_addrtype;  //host address type
           //IP地址的地址族信息，IPv4 对应 AF_INET，IPv6 对应 AF_INET6。
           int  h_length;  //address lenght
           char **h_addr_list;  //address list//以整数形式保存域名对应的 IP 地址,分配多个IP地址给同一域名，利用多个服务器进行均衡负载。
       }
       ```

  5. **inet_ntoa()函数**

     * 作用：将二进制数值转化为点分十进制的ip地址

     * 函数原型：

       ```c
       const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
       /*
       af: AF_INET或AF_INET6,IPv4 对应 AF_INET，IPv6 对应 AF_INET6。
       src:一个指向网络字节序的二进制值的指针；
       dst:一个指向转换后的点分十进制串的指针；
       cnt: 目标的大小，避免函数溢出缓冲区。
       */
       ```

     * 头文件：\#include <sys/types.h>，#include <sys/socket.h>，#include <arpa/inet.h>

     * 函数返回: 指向点分开的字符串地址(ip地址）的指针

  6. **socket()函数**

     * 作用：创建套接字

     * 函数原型：

       ```c
       int socket(int af, int type, int protocol);
       //af: AF_INET或AF_INET6,IPv4 对应 AF_INET，IPv6 对应 AF_INET6。
       // type :数据传输方式/套接字类型，常用的有 SOCK_STREAM（流格式套接字/面向连接的套接字） 和 SOCK_DGRAM（数据报套接字/无连接的套接字），
       //protocol :传输协议，常用的有 IPPROTO_TCP 和 IPPTOTO_UDP，分别表示 TCP 传输协议和 UDP 传输协议。protocol 的值设为 0，系统会自动推演出应该使用什么协议
       ```

     * 头文件：#include<sys/socket.h> 

     * 函数返回值：创建套接字成功，返回套接口的描述字，创建套接字失败，返回INVALID_SOCKET错误。

  7. **sockaddr_in结构体**

     * 头文件：在ws2def.h中
   * 结构体原型：

   ```c
     typedef struct sockaddr_in {
         #if(_WIN32_WINNT < 0x0600)
             short   sin_family;
         #else //(_WIN32_WINNT < 0x0600)
             ADDRESS_FAMILY sin_family;
         #endif //(_WIN32_WINNT < 0x0600)
         USHORT sin_port;
         IN_ADDR sin_addr;
         CHAR sin_zero[8];
     } SOCKADDR_IN, *PSOCKADDR_IN;
   ```

     * 成员变量介绍：
       sin_family：定义是哪种地址族

       sin_port：用来保存端口号

       sin_addr：用来保存IP地址信息

       sin_zero：没有特殊含义，只是为了让sockaddr与sockaddr_in两个数据结构保持大小相同而保留的空字节。

  8. **htons函数：**

     * 功能: 将整型变量从主机字节顺序转变成网络字节顺序

     * 函数原型：

       ```
       u_short htons(u_short hostshort);
       ```

     * 头文件：\#include<winsock2.h>

  9.  **bzero()函数：**

      * 功能：将内存（字符串）前n个字节清零；

      * 函数原型：

        ```
         void bzero(void *s, int n);
        ```

      * 头文件：\#include <string.h>

  10. **connect()函数：**

      * 功能：将参数sockfd 的socket 连至参数serv_addr 指定的网络地址

      * 函数原型：

        ```c
        int connect(int sockfd, struct sockaddr * serv_addr, int addrlen);
        ```

      * 头文件：\#include<sys/types.h>

  11. **getsockname()函数：**

      * 功能：用于获取与某个套接字关联的本地协议地址

      * 函数原型：

        ```c
        int getsockname( SOCKET s, struct sockaddr FAR* name,int FAR* namelen);
        //s: 一个已捆绑套接口的描述字
        //name：接收套接口的地址（名字）
        //namelen：名字缓冲区长度
        ```

      * 头文件：#include <sys/socket.h>

      * 返回值：成功返回0，失败返回-1

  12. **ntohs函数：**

      * 作用：将一个无符号短整型数从网络字节顺序转换为主机字节顺序。

      * 函数原型：

        ```c
        uint16_t ntohs(uint16_t netshort);
        //netshort：一个以网络字节顺序表达的16位数
        ```

      * 头文件：　#include <winsock.h>

  13. **read()函数：**

      * 作用：把参数fd 所指的文件传送count 个字节到buf 指针所指的内存中

      * 函数原型：

        ```
        ssize_t read(int fd, void * buf, size_t count);
        //fd：文件标识符
        //buf：目标存储区
        //count：读取的字节数
        ```

      * 头文件：\#include <unistd.h>

      * 返回值：当未读到指定的字节数的内容时，函数返回0，否则返回字节数。

  14. **send()函数：**

      * 作用：将数据由指定的socket 传给对方主机

      * 函数原型：

        ```c
        int send(int s, const void * msg, int len, unsigned int falgs);
        //s: 已建立好连接的套接字
        //msg: 指向欲连线的数据内容
        //len: 数据长度
        /*
           flags:一般设0, 其他数值定义如下：
           MSG_OOB 传送的数据以out-of-band 送出.
           MSG_DONTROUTE 取消路由表查询
           MSG_DONTWAIT 设置为不可阻断运作
           MSG_NOSIGNAL 此动作不愿被SIGPIPE 信号中断.
        */
        ```

      * 头文件：\#include <sys/types.h>  #include <sys/socket.h>

      * 返回值：成功返回传送出去的字符数, 失败返回-1. 查看errno的值对照errno值对应的错误原因表可以找到函数出错的原因。

  15. **kill()函数：**

      * 作用：可以用来传送参数sig 指定的信号给参数pid 指定的进程

      * 函数原型：

        ```c
        int kill(pid_t pid, int sig);
        ```

      * 头文件：\#include <sys/types.h>   #include <signal.h>

      * 返回值：执行成功返回0, 失败返回-1

      * SIGKILL是发送给一个进程来导致它立即终止的信号

  16. **recv()函数：**

      * 作用：接收远端主机经指定的socket 传来的数据, 并把数据存到由参数buf 指向的内存空间,

      * 函数原型：

        ```c
        int recv(int s, void *buf, int len, unsigned int flags);
        //s:表示套接字的标识符
        //buf：表示从套接字接收过来的数据存放的地方
        //len：可接收数据的最大长度
        /*
        flags一般设0. 其他数值定义如下:
        1、MSG_OOB 接收以out-of-band 送出的数据.
        2、MSG_PEEK 返回来的数据并不会在系统内删除, 如果再调用recv()会返回相同的数据内容.
        3、MSG_WAITALL 强迫接收到len大小的数据后才能返回,除非有错误或信号产生.
        4、MSG_NOSIGNAL 此操作不愿被SIGPIPE 信号中断
        */
        ```

      * 头文件：\#include <sys/types.h>  #include <sys/socket.h>

      * 返回值：成功返回接收到的字符数, 失败返回-1, 查看errno的值对照errno值对应的错误原因表可以找到函数出错的原因。

  17. **pipe()函数：**

      * 作用：创建管道

      * 函数原型：

        ```c
        int pipe(int filedes[2]);
        //两个文件描述符来操作管道。其中一个对管道进行写操作，另一个对管道进行读操作
        //fd[0]:读管道，fd[1]:写管道。
        ```

      * 头文件：\#include<unistd.h>

      * 返回值：成功返回0，失败返回-1

  18. **socket()函数：**

      * 作用：创建套接字

      * 函数原型：

        ```c
        int socket(int af, int type, int protocol);
        //af：Address Family，即IP地址类型，常用的有AF_INET和AF_INET6
        //type：数据传输方式/套接字类型，常用的有 SOCK_STREAM（流格式套接字/面向连接的套接字） 和 SOCK_DGRAM（数据报套接字/无连接的套接字
        // protocol 表示传输协议，常用的有 IPPROTO_TCP 和 IPPTOTO_UDP，分别表示 TCP 传输协议和 UDP 传输协议。可以将 protocol 的值设为 0，系统会自动推演出应该使用什么协议
        ```

        

      * 头文件：<sys/socket.h>

      * 返回值：创建成功返回套接字描述符，创建套接字失败，函数返回-1

  19. **atoi()函数：**

      * 作用：将字符串转换成int类型的整数，从头扫描字符串，跳过前面的无效字符，直到遇上数字或正负符号才开始做转换，直到遇到非数字或字符串结束时('\0')才结束转换，并将结果返回。

      * 函数原型：

        ```
        int atoi (const char * str);
        ```

      * 头文件：\#include <stdlib.h>

      * 返回值：成功返回转换后的整型数；失败返回 0。

  20. **setsockopt()函数：**

      * 作用：设置socket状态

      * 函数原型：

        ```c
        int setsockopt(int s, int level, int optname, const void * optval, socklen_* toptlen);
        //s：特定的套接字
        //level：欲设置的网络层, 一般设成SOL_SOCKET以存取socket层
        //optname:欲设置的选项
        /*
        可以设置为一下的值：
          SO_DEBUG 打开或关闭排错模式
          SO_REUSEADDR 允许在bind ()过程中本地地址可重复使用
          SO_TYPE 返回socket 形态.
          SO_ERROR 返回socket 已发生的错误原因
          SO_DONTROUTE 送出的数据包不要利用路由设备来传输.
          SO_BROADCAST 使用广播方式传送
          SO_SNDBUF 设置送出的暂存区大小
          SO_RCVBUF 设置接收的暂存区大小
          SO_KEEPALIVE 定期确定连线是否已终止.
          SO_OOBINLINE 当接收到OOB 数据时会马上送至标准输入设备
          SO_LINGER 确保数据安全且可靠的传送出去.
        */
        //optval:欲设置的值
        //toptlen:optval的长度
        ```

      * 头文件：#include <sys/types.h>   #include <sys/socket.h>

      * 返回值： 成功返回0,失败返回-1, 查看errno的值对照errno值对应的错误原因表可以找到函数出错的原因

  21. **bind()函数：**

      * 作用：
      * 函数原型：

      ````c
      int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
      //sockfd: socket()函数创建的文件描述符
      //my_addr: 指向一个结构为sockaddr参数的指针，sockaddr中包含了地址、端口和IP地址的信息
      //addrlen: my_addr结构的长度
      ````

      * 头文件：#include<sys/types.h>   #include<sys/socket.h>
      * 返回值：成功返回0，失败返回-1，查看errno的值对照errno值对应的错误原因表可以找到函数出错的原因。

  22. **listen()函数:**

      * 作用：让套接字进入被动监听状态

      * 函数原型：

        ```c
        int listen(int sock, int backlog);  
        //sock: 需要进入监听状态的套接字
        //backlog: 请求队列的最大长度
        ```

      * 头文件：#include<sys/socket.h>

      * 返回值：成功返回0，失败返回-1，查看errno的值对照errno值对应的错误原因表可以找到函数出错的原因。

  23. **accept()函数:**

      * 作用：接收客户端请求

      * 函数原型：

        ```c
        int accept(int sock, struct sockaddr *addr, socklen_t *addrlen);
        //sock: 服务器端套接字
        //addr: sockaddr_in 结构体变量
        //addrlen: 参数 addr 的长度
        ```

      * 头文件：#include<sys/socket.h>

      * 返回值：成功返回0，失败返回-1，查看errno的值对照errno值对应的错误原因表可以找到函数出错的原因。

  24. **fcntl()函数：**

      * 作用：用来操作文件描述词的一些特性

      * 函数原型：

        ```c
        int fcntl(int fd, int cmd);
        int fcntl(int fd, int cmd, long arg);
        int fcntl(int fd, int cmd, struct flock * lock);
        //fd: 欲设置的文件描述词
        //cmd: 欲操作的指令
        /*
          * F_DUPFD: 用来查找大于或等于参数arg 的最小且仍未使用的文件描述词, 并且复制参数fd 的文件描述词. 执行成功则返回新复制的文件描述词.
          *  F_GETFD：取得close-on-exec 标识符. 若此标识符的FD_CLOEXEC位为0, 代表在调用exec()相关函数时文件将不会关闭.
          * F_SETFD: 设置close-on-exec 标识符. 该标识符以参数arg 的FD_CLOEXEC 位决定.
          * F_GETFL: 取得文件描述词状态标识符, 此标识符为open()的参数flags.
          * F_SETFL: 设置文件描述词状态标识符, 参数arg 为新标识符, 但只允许O_APPEND、O_NONBLOCK 和O_ASYNC 位的改变, 其他位的改变将不受影响.
          * F_GETLK: 取得文件锁定的状态.
          * F_SETLK: 设置文件锁定的状态. 此时flcok 结构的l_type 值必须是F_RDLCK、F_WRLCK 或
          * F_UNLCK: 如果无法建立锁定, 则返回-1, 错误代码为EACCES 或EAGAIN.
          * F_SETLKW: 同F_SETLK 作用相同, 但是无法建立锁定时, 此调用会一直等到锁定动作成功为止. 
        */
        ```

      * 头文件：#include <unistd.h>     #include <fcntl.h>

      * 返回值：成功返回0，失败返回-1，查看errno的值对照errno值对应的错误原因表可以找到函数出错的原因。

  25. **inet_ntoa()函数：**

      * 作用：将网络二进制的数字转换成网络地址

      * 函数原型：

        ```c
        char * inet_ntoa(struct in_addr in);
        ```

      * 头文件：#include <sys/socket.h>   #include <netinet/in.h>   #include <arpa/inet.h>

      * 返回值：成功返回字符串指针, 失败返回NULL.

  26. **getifaddrs()函数：**

      * 作用：获取本地网络接口信息，将之存储于链表中

      * 函数原型：

        ```c
        int getifaddrs (struct ifaddrs **__ifap)
            
        ```

     /*
         __ifap是结果参数，指向链表第一个元素的指针
     struct ifaddrs结构描述如下：
                   struct ifaddrs {
                   struct ifaddrs  *ifa_next;    /* Next item in list *///指向链表中下一个struct ifaddr结构
                       char            *ifa_name;    /* Name of interface *///网络接口名
                       unsigned int     ifa_flags;   /* Flags from SIOCGIFFLAGS */
        //网络接口标志
                       struct sockaddr *ifa_addr;    /* Address of interface */
        //指向一个包含网络地址的sockaddr结构
                       struct sockaddr *ifa_netmask; /* Netmask of interface */
        // 指向一个包含网络掩码的结构
                       union {
                           struct sockaddr *ifu_broadaddr;
                                            /* Broadcast address of interface */
                           //如果(ifa_flags&IFF_BROADCAST)有效，ifu_broadaddr指向一个包含广播地址的结构。
                           struct sockaddr *ifu_dstaddr;
                                            /* Point-to-point destination address *///如果(ifa_flags&IFF_POINTOPOINT)有效，ifu_dstaddr指向一个包含p2p目的地址的结构。
                       } ifa_ifu;
                   #define              ifa_broadaddr ifa_ifu.ifu_broadaddr
                   #define              ifa_dstaddr   ifa_ifu.ifu_dstaddr
                       void            *ifa_data;    /* Address-specific data *///指向一个缓冲区，其中包含地址族私有数据。没有私有数据则为NULL。
                   };
         */
        ```
      

      * 头文件： \#include <sys/types.h>       #include <ifaddrs.h>
      
      * 返回值：成功返回0，失败返回-1，查看errno的值对照errno值对应的错误原因表可以找到函数出错的原因。

  27. **inet_ntop()函数:**

      * 作用：将二进制整数转换为点分十进制

      * 函数原型：

        ```c
        const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
        //af:地址族类型
        //src：一个指向网络字节序的二进制值的指针
        //dst:一个指向转换后的点分十进制串的指针；
        //cnt:目标的大小，以免函数溢出其调用者的缓冲区。
        ```

      * 头文件：\#include <sys/types.h>，#include <sys/socket.h>，#include <arpa/inet.h>

      * 返回值：成功返回字符串的首地址，错误返回NULL；

* 源代码以及详细注释：每条重要语句的功能都在注释中体现

  alg.11-3-socket-input-2.c

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  
  #define BUFFER_SIZE 1024
  
  /* input terminal, data packed without '\n' */
  /* write to pipe_data() through a named pipe */
  int main(int argc, char *argv[])
  {
      //fifoname用来存放管道文件名，write_buf表示读缓冲，暂时存放读入的消息数据
      char fifoname[80], write_buf[BUFFER_SIZE];
      int fdw, flags, ret, i;
  
      if(argc < 2) {
          //当命令行的参数个数小于2个时，提示输入管道文件名
          printf("Usage: ./a.out pathname\n");
          return EXIT_FAILURE;
      }
      //将命令行中的第二个参数写入fifoname中作为管道文件名使用
      strcpy(fifoname, argv[1]);
      //access()会检查是否可以读/写某一已存在的文件。
      //F_OK 则是用来判断该文件是否存在
      //这里是判断文件是否存在
      if(access(fifoname, F_OK) == -1) {
          //若文件不存在，则自己创立一个文件名为指定文件名fifoname，可读可写的具名管道
          if(mkfifo(fifoname, 0666) != 0) { /* make a named pipe */
              //如果创建具名管道失败则输出失败，并退出程序
              perror("mkfifo()");
              exit(EXIT_FAILURE);
          }
          else
          //如果建立新的管道成功则输出新的管道（名字为fifoname）已经建立
              printf("new fifo %s created ...\n", fifoname);
      }
      //以可读可写的方式打开文件，打开失败则返回-1
      fdw = open(fifoname, O_RDWR); /* blocking write and blocking read in default */
      
      if(fdw < 0) { 
          //文件打开失败，输出失败并退出程序
          perror("pipe open()");
          exit(EXIT_FAILURE);
      }
      else {
          //fcntl()用来操作文件描述词的一些特性.
          //F_GETFL 取得文件描述词状态标识符, 此标识符为open()的参数flags.
          flags = fcntl(fdw, F_GETFL, 0);
          //F_SETFL 设置文件描述词状态标识符, 参数flags 为新标识符
          //O_NONBLOCK 非阻塞I/O，表示程序没有读写数据时也不发生阻塞
          fcntl(fdw, F_SETFL, flags | O_NONBLOCK); /* set to non-blocking write named pipe */
          while (1) {
              //提示输入消息
              printf("Enter some text (#0-quit | #1-nickname): \n");
              //将指定大小的读缓冲区全部清零
              memset(write_buf, 0, BUFFER_SIZE);
              //从输入流中读入指定缓冲区大小的信息到读缓冲区中
              fgets(write_buf, BUFFER_SIZE, stdin);
              //设置缓冲区的最后一个字节为0，作为消息（字符串）结束的标记符
              write_buf[BUFFER_SIZE-1] = 0;
              for (i = 0; i < BUFFER_SIZE; i++) {
                  //设置当消息回车时，回车前的消息才是完整消息，在回车处设置消息结束的标记符
              	if(write_buf[i] == '\n') {
                      write_buf[i] = 0;
                  }
              }  /* '\n' filtered */
              //write()把写缓冲区write_buf的内容写入BUFFER_SIZE个字节到标识符为fdw的管道中
              //写入成功返回写入的字节数，写入失败放回-1
              ret = write(fdw, write_buf, BUFFER_SIZE); /* non-blocking write named pipe */
              if(ret <= 0) {
                  //写入失败，答应写入失败，管道阻塞，要求用户再次输入消息
                  perror("write()");
                  printf("Pipe blocked, try again ...\n");
                  //程序等待1秒
                  sleep(1);
              }
         	}
      }
  	//关闭管道文件
      close(fdw);
  
      exit(EXIT_SUCCESS);
  }
  
  ```

  alg.11-4-socket-connector-BBS-2.c

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <sys/signal.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #define bzero(a,b) memset(a,0,b)
  #define BUFFER_SIZE 1024
  #define NICKNAME_L 11
  #define MSG_SIZE BUFFER_SIZE+NICKNAME_L+4
  #define ERR_EXIT(m) \
      do { \
          perror(m); \
          exit(EXIT_FAILURE); \
      } while(0)
  
  /* asynchronous send-receive version; separated input terminal*/
  
  int main(int argc, char *argv[])
  {
      char fifoname[80], nickname[80];
      int fdr, connect_fd;
      char ip_name_str[INET_ADDRSTRLEN];
      uint16_t port_num;
      char stdin_buf[BUFFER_SIZE], msg_buf[MSG_SIZE];
      int sendbytes, recvbytes, ret;
      char clr;
      struct hostent *host;
      struct sockaddr_in server_addr, connect_addr;//实例化addr来存放地址信息
      //分别表示服务器绑定的地址，连接的对端地址
      socklen_t addr_len;
      pid_t childpid;
      
      if(argc < 2) {
          printf("Usage: ./a.out pathname\n");
          return EXIT_FAILURE;
      }
  
      //将命令行中的第二个参数写入fifoname中作为管道文件名使用
      strcpy(fifoname, argv[1]);
      //access()会检查是否可以读/写某一已存在的文件。
      //F_OK 则是用来判断该文件是否存在
      //这里是判断文件是否存在
      if(access(fifoname, F_OK) == -1) {
          //若文件不存在，则自己创立一个文件名为指定文件名fifoname，可读可写的具名管道
          if(mkfifo(fifoname, 0666) != 0) {
              //如果创建具名管道失败则输出失败，并退出程序
              perror("mkfifo()");
              exit(EXIT_FAILURE);
          }
          else
          //如果建立新的管道成功则输出新的管道（名字为fifoname）已经建立
              printf("new fifo %s named pipe created\n", fifoname);
      }
      //以可读可写的方式打开文件，打开失败则返回-1
      fdr = open(fifoname, O_RDWR);  /* blocking write and blocking read in default */
      if(fdr < 0) {
          //文件打开失败，输出失败并退出程序
          perror("pipe read open()");
          exit(EXIT_FAILURE);
      }
      //提示输入服务端的hostname或者ipv4
      printf("Input server's hostname/ipv4: "); /* www.baidu.com or an ipv4 address */
      scanf("%s", stdin_buf);
      //清除多余的输入
      while((clr = getchar()) != '\n' && clr != EOF); /* clear the stdin buffer */
      //提示输入端口号
      printf("Input server's port number: ");
      scanf("%hu", &port_num);
      //清除多余的输入
      while((clr = getchar()) != '\n' && clr != EOF);
      //gethostbyname()函数：通过域名获取IP地址
      if((host = gethostbyname(stdin_buf)) == NULL) {
          //如果返回的存放IP地址的hostent 结构体为空，则说明该hostname或者ipv4是无效的
          //打印hostname或者ipv4无效并退出程序
          printf("invalid name or ip-address\n");
          exit(EXIT_FAILURE);
      }
      //打印该hostname或者ipv4的官方域名
      printf("server's official name = %s\n", host->h_name);
      //ptr指向指定域名结果的ip地址表
      char** ptr = host->h_addr_list;
      for(; *ptr != NULL; ptr++) {
          //将prt指向的网络字节序的二进制值转换成ip_name_str指向的ip点分十进制串存放处，字节限制为ip_name_str结构的大小，即INET_ADDRSTRLEN
          //其中ip地址的类型为指定域名结构中的类型host->h_addrtype
          inet_ntop(host->h_addrtype, *ptr, ip_name_str, sizeof(ip_name_str));
          //打印转换后的ip点分十进制串
          printf("\tserver address = %s\n", ip_name_str);
      }
          /*creat connection socket*/
      //SOCK_STREAM:流格式套接字/面向连接的套接字
      //创建ip地址类型为AF_INET，类型为流格式套接字的套接字，创建套接字的第3个参数为0表示系统会自动推演出应该使用什么协议
      if((connect_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
          //创建套接字失败，函数放回-1
          //输出错误类型并退出程序
          ERR_EXIT("socket()");
      }
          /* set sockaddr_in of server-side */
      
      server_addr.sin_family = AF_INET;//指定IP地址类型为IPV4
      //htons(port_num):将整型变量的端口号从主机字节顺序转变成网络字节顺序
      server_addr.sin_port = htons(port_num);//存放端口信息
      server_addr.sin_addr = *((struct in_addr *)host->h_addr);//转换服务器ip地址
      //将sin_zero成员变量的前8个字节清零
      bzero(&(server_addr.sin_zero), 8);
  
      addr_len = sizeof(struct sockaddr);
      //将套接字的socket连至server_addr指定的网络地址,参数addr_len为server_addr的结构长度.
      ret = connect(connect_fd, (struct sockaddr *)&server_addr, addr_len); /* connect to server */
      if(ret == -1) {
          //连接失败，关闭套接字，输出失败原因并退出程序
          close(connect_fd);
          ERR_EXIT("connect()"); 
      }
      /* connect_fd is assigned a port_number after connecting */
      addr_len = sizeof(struct sockaddr);
      //获取由内核赋予该连接的本地IP地址和本地端口号
      ret = getsockname(connect_fd, (struct sockaddr *)&connect_addr, &addr_len);
      if(ret == -1) {
          //获取失败则关闭连接套接字，输出错误原因并退出程序
          close(connect_fd);
          ERR_EXIT("getsockname()");
      }
      //ntohs: 将一个无符号短整型数的端口号从网络字节顺序转换为主机字节顺序并存放在变量port_num中
      port_num = ntohs(connect_addr.sin_port);
      //将二进制数值的连接地址的端口号转化为点分十进制的ip地址赋给变量ip_name_str
      strcpy(ip_name_str, inet_ntoa(connect_addr.sin_addr));
      //打印按主机字节顺序的端口号以及点分十进制的ip地址
      printf("Local port: %hu, IP addr: %s\n", port_num, ip_name_str);
      //将二进制数值的服务端地址的端口号转化为点分十进制的ip地址赋给变量ip_name_str
      strcpy(ip_name_str, inet_ntoa(server_addr.sin_addr));
  
      childpid = fork();
      if(childpid < 0)
          ERR_EXIT("fork()");
      if(childpid > 0) { /* parent pro */
          while(1) { /* sending cycle */
              //从fdr的消息管道中读取BUFFER_SIZE个字节的消息存储在stdin_buf缓冲区中
              ret = read(fdr, stdin_buf, BUFFER_SIZE); /* blocking read named pipe; read data from input terminal */
              if(ret <= 0) {
                  //若读取失败，则打印失败并退出发送循环
                  perror("read()"); 
                  break;
              } 
              //将缓冲区的最后一个字节设置为消息的结束符
              stdin_buf[BUFFER_SIZE-1] = 0;
              //将stdin_buf中BUFFER_SIZE大小字节的数据由套接字connect_fd发送到连接到的主机中
              //发送成功返回发送的字节数，失败放回-1
              sendbytes = send(connect_fd, stdin_buf, BUFFER_SIZE, 0); /* blocking socket send */
              if(sendbytes <= 0) {
                  //发送失败，打印连接终止，退出发送循环
                  printf("sendbytes = %d. Connection terminated ...\n", sendbytes);
                  break;
              }
              //当发送的字符为“#0”,程序给连接主机发送退出消息，并退出发送循环
              if(strncmp(stdin_buf, "#0", 2) == 0) {
                  memset(stdin_buf, 0, BUFFER_SIZE);
                  strcpy(stdin_buf, "I quit ... ");
                  sendbytes = send(connect_fd, stdin_buf, BUFFER_SIZE, 0); /* blocking socket send */
                  break;
              }  
          } 
          //关闭消息管道
          close(fdr);
          //关闭连接的套接字
          close(connect_fd);
          //传送信号SIGKILL给childpid指定的进程，SIGKILL是发送给一个进程来导致它立即终止的信号
          kill(childpid, SIGKILL);
      }
      else { /* child pro */
          while(1) { /* receiving cycle */
          //从标识符为connect_fd的套接字传来的字节上限为MSG_SIZE大小的数据, 并把数据存到msg_buf中,
              recvbytes = recv(connect_fd, msg_buf, MSG_SIZE, 0); /* blocking socket recv */
              //成功返回接收到的字符数, 失败返回-1
              if(recvbytes <= 0) {
                  //接收失败，打印接收字节数，并显示连接终止，退出接收循环
                  printf("recvbytes = %d. Connection terminated ...\n", recvbytes);
                  break;
              }
              //将接收到的数据的最后一个单位改为0，作为字符串消息结束的标志，便于打印消息内容
              msg_buf[MSG_SIZE-1] = 0;
              printf("%s\n", msg_buf); 
              //比较接收到的数据是否等于"Console: #0"
              ret = strncmp(msg_buf, "Console: #0", 11); /* be kicked out */
              if(ret == 0) {
                  //当接收到的数据等于"Console: #0"时，表示接收结束，退出接收的循环
                  break;
              }
          }
          //关闭连接的套接字
          close(connect_fd);
          //传送信号SIGKILL给当前进程，SIGKILL是发送给一个进程来导致它立即终止的信号
          kill(getppid(), SIGKILL);
      }
  
      return EXIT_SUCCESS;
  }
  
  
  ```

  alg.11-5-socket-server-BBS-3.c

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <ifaddrs.h>
  #include <sys/shm.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <ctype.h>
  
  #define BUFFER_SIZE 1024 /* each pipe has at least 64 blocks for this sze */
  #define NICKNAME_L 11 /* 10 chars for nickname */
  #define MSG_SIZE BUFFER_SIZE+NICKNAME_L+4 /* msg exchanged between pipe_data() and recv_send_data() */
  #define MAX_QUE_CONN_NM 5 /* length of ESTABLISHED queue */
  #define MAX_CONN_NUM 10 /* cumulative number of connecting processes */
  #define STAT_EMPTY 0
  #define STAT_NORMAL 1
  #define STAT_ENDED -1
  
  #define ERR_EXIT(m) \
      do { \
          perror(m); \
          exit(EXIT_FAILURE); \
      } while(0)
  
  /* one server, m clients BBS, with private chatting */
  
  struct {
      int stat;
      char nickname[NICKNAME_L];
  } sn_attri[MAX_CONN_NUM+1];
  
  int connect_sn, max_sn; /* from 1 to MAX_CONN_NUM */
  int server_fd, connect_fd[MAX_CONN_NUM+1];
  int fd[MAX_CONN_NUM+1][2];
                  /* ordinary pipe: pipe_data() gets max_sn from main() by fd[0][0]
                     recv_send_data(sn) get send_buf from pipe_data() by fd[sn][0], 0<sn<MAX_CONN_NUM+1 */
  int fd_stat[2]; /* ordinary pipe: pipe_data() gets stat of connect_sn from main() */
  int fd_msg[2];  /* ordinary pipe: pipe_data() gets message of connect_sn from recv_send_data() */
  int fdr;   /* named pipe: pipe_data() gets stdin_buf from input terminal */
  struct sockaddr_in server_addr, connect_addr;
  
  int getipv4addr(char *ip_addr)
  {
      struct ifaddrs *ifaddrsptr = NULL;
      struct ifaddrs *ifa = NULL;
      void *tmpptr = NULL;
      int ret;
      //获取本地网络接口信息,将之存储于ifaddrsptr指向的链表中
      ret = getifaddrs(&ifaddrsptr);
      if(ret == -1)
          //获取失败，打印失败原因并退出程序
          ERR_EXIT("getifaddrs()");
  
      for(ifa = ifaddrsptr; ifa != NULL; ifa = ifa->ifa_next) {
          if(!ifa->ifa_addr) {
              //如果网络地址为空，则继续找下一个执行的网络地址
              continue;
          }
          //网络地址类型为IP4类型
          if(ifa->ifa_addr->sa_family == AF_INET) { /* IP4 */
              tmpptr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
              char addr_buf[INET_ADDRSTRLEN];
              //将网络字节序二进制值tmpptr转换成点分十进制串addr_buf
              //打印网络接口的名字以及对应的ip地址addr_buf
              inet_ntop(AF_INET, tmpptr, addr_buf, INET_ADDRSTRLEN);
              printf("%s IPv4 address %s\n", ifa->ifa_name, addr_buf);
              if(strcmp(ifa->ifa_name, "lo") != 0)
              //如果接口名不为“lo”则将该端口的ip地址作为结果返回
                  strcpy(ip_addr, addr_buf); /* return the ipv4 address */
          } else if(ifa->ifa_addr->sa_family == AF_INET6) { /* IP6 */
          //网络地址类型为IP6类型
              tmpptr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
              char addr_buf[INET6_ADDRSTRLEN];
              //将网络字节序二进制值tmpptr转换成点分十进制串addr_buf
              //打印网络接口的名字以及对应的ip地址addr_buf
              inet_ntop(AF_INET6, tmpptr, addr_buf, INET6_ADDRSTRLEN);
              printf("%s IPv6 address %s\n", ifa->ifa_name, addr_buf);
          }
      }
  
      if(ifaddrsptr != NULL) {
          freeifaddrs(ifaddrsptr);
      }
  
      return EXIT_SUCCESS;
  }
  
  void pipe_data(void)
  {
  /* get sidin_buf from input terminal
     update max_sn from main()
     update sn_stat from main() - STAT_EMPTY->STAT_NORMAL
     update sn_stat from recv_send_data() STAT_NORMAL->STAT_ENDED
     update sn_nickname from recv_send_data()
     select connect_sn by the descritor @**** in start of send_buf */
  
      char send_buf[BUFFER_SIZE], stat_buf[BUFFER_SIZE], stdin_buf[BUFFER_SIZE];
  	char msg_buf[MSG_SIZE]; /* sn(4)nickname(10)recv_buff(BUFFER_SIZE) */
      int flags, sn, ret, i, new_stat;
      char nickname[NICKNAME_L];
      //取得fd[0][0]的文件描述词状态标识符，赋值给flags
      flags = fcntl(fd[0][0], F_GETFL, 0);
      //F_SETFL: 设置fd[0][0]的文件描述词状态标识符为上一个函数获取的状态标识符flags，并设置管道为非阻塞状态
      fcntl(fd[0][0], F_SETFL, flags | O_NONBLOCK); /* set to non-blocking read ord-pipe */
      //取得fd_stat[0]的文件描述词状态标识符，赋值给flags
      flags = fcntl(fd_stat[0], F_GETFL, 0);
      //F_SETFL: 设置fd_stat[0]的文件描述词状态标识符为上一个函数获取的状态标识符flags，并设置管道为非阻塞状态
      fcntl(fd_stat[0], F_SETFL, flags | O_NONBLOCK); /* set to non-blocking read ord-pipe */
      //取得fd_msg[0]的文件描述词状态标识符，赋值给flags
      flags = fcntl(fd_msg[0], F_GETFL, 0);
      //F_SETFL: 设置fd_msg[0]的文件描述词状态标识符为上一个函数获取的状态标识符flags，并设置管道为非阻塞状态
      fcntl(fd_msg[0], F_SETFL, flags | O_NONBLOCK); /* set to non-blocking read ord-pipe */
      //取得fdr的文件描述词状态标识符，赋值给flags
      flags = fcntl(fdr, F_GETFL, 0);
      //F_SETFL: 设置fdr的文件描述词状态标识符为上一个函数获取的状态标识符flags，并设置管道为非阻塞状态
      fcntl(fdr, F_SETFL, flags | O_NONBLOCK); /* set to non-blocking read nam-pipe */
  
      while(1) { 
          while (1) { /* get the last current max_sn from main() */
              //从读标识符为fd[0][0]的客户端管道中读取sizeof(sn)个字节的信息发送到sn中
              ret = read(fd[0][0], &sn, sizeof(sn)); /* non-blocking read ord-pipe from main() */
              if(ret <= 0) { /* pipe empty */
                  //管道为空，读取失败，退出读取循环
                  break;
              } 
              //更新最大的客户端进程数量
              max_sn = sn;
              printf("max_sn changed to: %d\n", max_sn);
          }
  
          while (1) { /* update sn_stat from main() */
              //从参数fd_stat[0]所指的文件中（管道中）传送BUFFER_SIZE个字节到stat_buf缓冲区中
              ret = read(fd_stat[0], stat_buf, BUFFER_SIZE); /* non-blocking read ord-pipe from main() */
              if(ret <= 0) { /* pipe empty */
                  //从管道读取消息失败，退出循环
                  break;
              } 
              //将stat_buf的数据格式化输入整型变量sn和new_stat中
              sscanf(stat_buf, "%d,%d", &sn, &new_stat);
              //打印客户端进程的数目（是第几个连接上服务端的客户端，以及客户端的状态）
              printf("SN stat changed: sn = %d, stat: %d -> %d\n", sn, sn_attri[sn].stat, new_stat);
              //更新客户端状态
              sn_attri[sn].stat = new_stat;
          }  
  
          /*以下是服务端接收客户端发来的消息后，执行相关操作，如断开和客户端的连接，
          修改客户端的昵称，进行特定客户端之间的信息传输，进行客户端的广播通讯*/
  
          while (1) { /* update sn_stat and nickname from recv_send_data(), or brocast msg to all sn */
              //从消息管道中读取MSG_SIZE个字节的消息到缓冲区msg_buf中
              ret = read(fd_msg[0], msg_buf, MSG_SIZE); /* non-blocking read ord-pipe from recv_send_data() */
              if(ret <= 0) { /* pipe empty */
                  //消息管道读取失败退出读取循环
                  break;
              }
              //将msg_buf的数据格式化输入4位整型变量sn和字符串stat_buf中
              sscanf(msg_buf, "%4d%s", &sn, stat_buf);
              if(msg_buf[4] == '#') {
                  /*当读取的信息的前两个元素为“#0”时候，输出客户端的编号数以及当前的状态，
                  将要更改的状态（即是关闭状态），更改管道状态为关闭状态*/
                  if(msg_buf[5] == '0') { /* #0: terminating the connect_fd */
                      /*实现客户端的连接的退出*/
                      new_stat = STAT_ENDED;
                      printf("SN stat changed: sn = %d, stat: %d -> %d\n", sn, sn_attri[sn].stat, new_stat);
                      sn_attri[sn].stat = new_stat;
                  }
                  /*当读取的消息的前两个元素为“#1”时候，更改客户端的名字*/
                  if(msg_buf[5] == '1') { /* #1name: renaming the nickname */
                      /*实现修改昵称*/
                      strncpy(nickname, &msg_buf[6], NICKNAME_L);
                  /*消息前两个元素后面剩余元素组成的新的名字，当名字中出现空格将空格转换为
                  下划线，当消息中元素包含回车时，表示名字输入结束，将该字节修改为字符串的结束标志0*/
                      for (i = 0; i < NICKNAME_L-1; i++) {
                       	if(nickname[i] == ' ') {
                              nickname[i] = '_';
                          }
                       	if(nickname[i] == '\n') {
                              nickname[i] = 0;
                          }
                      }
                      nickname[i] = 0;
                      //输出第几个编号的客户端的昵称从旧的昵称变成新的昵称
                      printf("SN stat changed: sn = %d, nickname: %s -> %s\n", sn, sn_attri[sn].nickname, nickname);
                      for (i=0; i<=max_sn; i++) { /* sn_attri[0].nickname = "Console" */
                          //将客户端的昵称一个一个和新的昵称进行比较，找出是否有客户端已经占用该昵称
                          ret = strcmp(sn_attri[i].nickname, nickname);
                          //通过比较找到占用昵称的客户端编号，通过将消息写入特定的管道向该客户端发送昵称该客户端占用的消息
                      	if(ret == 0) {
                              memset(msg_buf, 0, MSG_SIZE);
                              sprintf(msg_buf, "Console: this nickname occupied: %s", nickname);
                              ret = write(fd[sn][1], msg_buf, MSG_SIZE); /* non-blocking write ord-pipe */
                              if(ret <= 0) {
                                  //如果消息写入管道失败，则输出失败，信息丢失
                                  printf("sn = %d write error, message missed ...\n", sn);
                              }    
                              break;
                          }
                      }
                      //当i>max_sn时，说明该昵称没有被任何客户端占用，修改指定编号的客户端的昵称为新的昵称
                      if(i > max_sn) {
                          //通过写入特定的关东向修改的客户端发送“你的昵称已经被修改成。。。（新昵称)”的消息
                          strncpy(sn_attri[sn].nickname, nickname, NICKNAME_L);
                          memset(msg_buf, 0, MSG_SIZE);
                          sprintf(msg_buf, "Console: your nickname changed to %s", sn_attri[sn].nickname);
                          ret = write(fd[sn][1], msg_buf, MSG_SIZE); /* non-blocking write ord-pipe */
                          if(ret <= 0) {
                              //写入管道失败，打印消息丢失
                              printf("sn = %d write error, message missed ...\n", sn);
                          }
                      }
                  }  
                  /* ignore the message from recv_send_data() otherwise */
              }
              else if(msg_buf[4] == '@') {
                  /*实现客户端向客户端发送消息*/
                  //将@后的字符串输入到nickname的数组中直到遇到字符串的结束符或者空格
                  for (i = 0; i < NICKNAME_L-1; i++) {
                      nickname[i] = msg_buf[5+i];
                      if(msg_buf[5+i] == 0 || msg_buf[5+i] == ' ') {
                      	break;
                      }
                  }
                  nickname[i] = 0;
                  //将昵称后面多余的空格删除
                  if(msg_buf[5+i] == ' ') {
                      i++;
                  }
                  //将昵称后面的信息输入到缓冲区stdin_buf中
                  strcpy(stdin_buf, &msg_buf[5+i]);
                  //对msg_buf缓冲区进行清零操作，以“对方客户端的昵称@：发送给对方的信息”的形式将字符串存储在msg_buf缓冲区中
                  memset(msg_buf, 0, MSG_SIZE);
                  sprintf(msg_buf, "%s@: %s", sn_attri[sn].nickname, stdin_buf);
                  for (sn = 1; sn <= max_sn; sn++) { /* message sent to all sn's by ord-pipes fd[sn][1] */
                      //遍历所有客户端，找到连接到服务端且名字为nickname的客户端
                      //向通向该客户端的管道中写入msg_buf的消息
                      if(sn_attri[sn].stat == STAT_NORMAL && strcmp(sn_attri[sn].nickname, nickname) == 0) {
                          flags = fcntl(fd[sn][1], F_GETFL, 0);
                          fcntl(fd[sn][1], F_SETFL, flags | O_NONBLOCK); /* set to non-blocking write ord-pipe */
                          ret = write(fd[sn][1], msg_buf, MSG_SIZE); /* non-blocking write ord-pipe */
                          if(ret <= 0) {
                              //写入失败时，打印写入失败，消息丢失
                              printf("sn = %d write error, message missed ...\n", sn);
                          }
                      }
                  }
              }
              else {
                  /*实现客户端以广播的形式向其他客户端发送消息*/
                  //将待发送信息输入到缓冲区stdin_buf中
                  strcpy(stdin_buf, &msg_buf[4]);
                  //对msg_buf缓冲区进行清零操作，以“发送消息的客户端的昵称：发送给对方的信息”的形式将字符串存储在msg_buf缓冲区中
                  memset(msg_buf, 0, MSG_SIZE);
                  sprintf(msg_buf, "%s: %s", sn_attri[sn].nickname, stdin_buf);
                  for (sn = 1; sn <= max_sn; sn++) { /* message sent to all sn's by ord-pipes fd[sn][1] */
                      //遍历客户端，找到处于连接到服务端状态的客户端，则通过写入特定管道的方式向该客户端发送消息
                      if(sn_attri[sn].stat == STAT_NORMAL) {
                          flags = fcntl(fd[sn][1], F_GETFL, 0);
                          fcntl(fd[sn][1], F_SETFL, flags | O_NONBLOCK); /* set to non-blocking write ord-pipe */
                          ret = write(fd[sn][1], msg_buf, MSG_SIZE); /* non-blocking write ord-pipe */
                          if(ret <= 0) {
                              //写入失败时，打印写入失败，消息丢失
                              printf("sn = %d write error, message missed ...\n", sn);
                          }
                      }
                  }
              }
          }
  
  
          while (1) { /* read from input terminal and brocast to all sn */
  			ret = read(fdr, stdin_buf, BUFFER_SIZE); /* non-blocking read nam-pipe from input terminal */
              if(ret <= 0) {
                  break;
              } 
              if(stdin_buf[0] == '@') {
                  //私聊程序，向特定的客户端发送消息
                  sn = atoi(&stdin_buf[1]);
                  if(sn > 0 && sn <= max_sn && sn_attri[sn].stat == STAT_NORMAL) {
                      for (i = 1; isdigit(stdin_buf[i]); i++) ;
                      if(stdin_buf[i] == '#' && stdin_buf[i+1] == '0') { /* #0: terminating the connect_fd */
                          new_stat = STAT_ENDED;
                          printf("SN stat changed: sn = %d, stat: %d -> %d\n", sn, sn_attri[sn].stat, new_stat);
                          sn_attri[sn].stat = new_stat;
                          memset(msg_buf, 0, MSG_SIZE);
                          sprintf(msg_buf, "%s: %s", sn_attri[0].nickname, "#0 your connection terminated!");
                          ret = write(fd[sn][1], msg_buf, MSG_SIZE); /* non-blocking write ord-pipe */
                          if(ret <= 0) {
                              printf("sn = %d write error, message missed ...\n", sn);
                          }
                          ;
                      }
                      else {
                          flags = fcntl(fd[sn][1], F_GETFL, 0);
                          fcntl(fd[sn][1], F_SETFL, flags | O_NONBLOCK); /* set to non-blocking write ord-pipe */
                          memset(msg_buf, 0, MSG_SIZE);
                          sprintf(msg_buf, "%s: %s", sn_attri[0].nickname, &stdin_buf[i]);
                          ret = write(fd[sn][1], msg_buf, MSG_SIZE); /* non-blocking write ord-pipe */
                          if(ret <= 0) {
                              printf("sn = %d write error, message missed ...\n", sn);
                          }
                      }
                  }
                  ; /* invalid connect_sn ignored */
              } 
              else {
                  //广播，向所有的客户端包括自己发送消息
                  memset(msg_buf, 0, MSG_SIZE);
                  sprintf(msg_buf, "%s: %s", sn_attri[0].nickname, stdin_buf);
                  for (sn = 1; sn <= max_sn; sn++) { /* message sent to all sn's by ord-pipes fd[sn][1] */
                      if(sn_attri[sn].stat == STAT_NORMAL) {
                          flags = fcntl(fd[sn][1], F_GETFL, 0);
                          fcntl(fd[sn][1], F_SETFL, flags | O_NONBLOCK); /* set to non-blocking write ord-pipe */
                          ret = write(fd[sn][1], msg_buf, MSG_SIZE); /* non-blocking write ord-pipe */
                          if(ret <= 0) {
                              printf("sn = %d write error, message missed ...\n", sn);
                          }
                      }
                  }
              }
          }
      } 
      return;
  }
  
  void recv_send_data(int sn)
  {
      char recv_buf[BUFFER_SIZE], send_buf[BUFFER_SIZE];
      char msg_buf[MSG_SIZE]; /* sn(4)nickname(10)recv_buff(BUFFER_SIZE) */
      int recvbytes, sendbytes, ret, flags;
      int stat;
  
      // 取得connect_fd[sn]的文件描述词状态标识符flags
      flags = fcntl(connect_fd[sn], F_GETFL, 0);
      //设置connect_fd[sn]的文件描述词状态标识符, flags为新标识符。
      fcntl(connect_fd[sn], F_SETFL, flags | O_NONBLOCK); /* set to non-blocking mode to socket recv */
      // 取得fd[sn][0]的文件描述词状态标识符flags
      flags = fcntl(fd[sn][0], F_GETFL, 0);
      //设置fd[sn][0]的文件描述词状态标识符, flags为新标识符。
      fcntl(fd[sn][0], F_SETFL, flags | O_NONBLOCK); /* set to non-blocking mode to ord-pipe read */
  
      while(1) { /* receiving and sending cycle */
          //连接管道中读取最大长度为BUFFER_SIZE的消息存储到recv_buf中，设置读取不到消息不需要阻塞等待
          recvbytes = recv(connect_fd[sn], recv_buf, BUFFER_SIZE, MSG_DONTWAIT); /* non-blocking socket recv */
          if(recvbytes > 0) {
              //读取成功则返回消息的字节数
              //打印客户端编号以及其发过来的消息
              printf("===>>> SN-%d: %s\n", sn, recv_buf);
              memset(msg_buf, 0, MSG_SIZE);
              //将sn作为前4个元素，以及recv_buf一起输入到msg_buf缓冲区
              sprintf(msg_buf, "%4d%s", sn, recv_buf);
              //将消息写入消息管道中
              ret = write(fd_msg[1], msg_buf, MSG_SIZE); /* blocking write ord-pipe to pipe_data() */
              if(ret <= 0) {
                  //写入失败，打印失败原因并退出读操作循环
                  perror("fd_stat write() to pipe_data()");
                  break;
              }
          }
          //从客户端管道中读取消息，将消息以固定的格式打印在服务端处，标明了消息来自哪个编号的客户端
          //将消息发动到连接管道中
          ret = read(fd[sn][0], msg_buf, MSG_SIZE); /* non-blocking read ord-pipe from pipe_data() */
          if(ret > 0) {
              printf("sn = %d send_buf ready: %s\n", sn, msg_buf);
              sendbytes = send(connect_fd[sn], msg_buf, MSG_SIZE, 0); /* blocking socket send */
              if(sendbytes <= 0) {
                  //发送失败则退出循环
                  break;
              }
          } 
          sleep(1); /* heart beating */
      }
      return;
  }
  
  
  int main(int argc, char *argv[])
  {
      socklen_t addr_len;
      pid_t pipe_pid, recv_pid, send_pid;
      char stdin_buf[BUFFER_SIZE], ip4_addr[INET_ADDRSTRLEN];
      uint16_t port_num;
      int ret;
      char fifoname[80], clr;
      int stat;
    
      if(argc < 2) {
          //命令行参数小于2个，提示输入第二个命令行参数（文件名），程序执行失败退出程序
          printf("Usage: ./a.out pathname\n");
          return EXIT_FAILURE;
      }
  
      //将命令行中的第二个参数写入fifoname中作为管道文件名使用
      strcpy(fifoname, argv[1]);
      //access()会检查是否可以读/写某一已存在的文件。
      //F_OK 则是用来判断该文件是否存在
      //这里是判断文件是否存在
      if(access(fifoname, F_OK) == -1) {
          //若文件不存在，则自己创立一个文件名为指定文件名fifoname，可读可写的具名管道
          if(mkfifo(fifoname, 0666) != 0) {
              //如果创建具名管道失败则输出失败，并退出程序
              perror("mkfifo()");
              exit(EXIT_FAILURE);
          }
          else {
              //如果建立新的管道成功则输出新的管道（名字为fifoname）已经建立
              printf("new fifo %s named pipe created\n", fifoname);
          }
      }
      //以可读可写的方式打开文件，打开失败则返回-1
      fdr = open(fifoname, O_RDWR);  /* blocking write and blocking read in default */
      if(fdr < 0) {
          //命名管道文件打开失败，输出失败并退出程序
          perror("named pipe read open()");
          exit(EXIT_FAILURE);
      }
      //创立MAX_CONN_NUM个管道，fd为管道标识符存放的数组，当有一个管道建立失败时打印出错原因
      for (int i = 0; i <= MAX_CONN_NUM; i++) {
          ret = pipe(fd[i]);
          if(ret == -1) {
              perror("fd pipe()");
          }
      }
     //建立fd_stat管道
      ret = pipe(fd_stat);
      if(ret == -1) {
          //管道建立失败，打印出错原因
          perror("fd_stat pipe()");
      }
      //建立fd_msg管道
      ret = pipe(fd_msg);
      if(ret == -1) {
          //管道建立失败，打印出错原因
          perror("fd_msg pipe()");
      }
      //初始化客户端的状态为空，命名为 "Anonymous"，表示现在未连接
      for (int i = 0; i <= MAX_CONN_NUM; i++) {
          sn_attri[i].stat = STAT_EMPTY;
          strcpy(sn_attri[i].nickname, "Anonymous");
      }
      //将第一个客户端位命名为"Console"
      strcpy(sn_attri[0].nickname, "Console");
      //建立服务端的套接字
      //SOCK_STREAM:流格式套接字/面向连接的套接字
      //创建ip地址类型为AF_INET，类型为流格式套接字的套接字，创建套接字的第3个参数为0表示系统会自动推演出应该使用什么协议   
      server_fd = socket(AF_INET, SOCK_STREAM, 0);
      if(server_fd == -1) {
          //创建套接字失败，函数放回-1
          //输出错误类型并退出程序
          ERR_EXIT("socket()");
      }
      //输出套接字标识符
      printf("server_fd = %d\n", server_fd);
      //getIPv4addr():获取ip4_addr的IPv4
      getipv4addr(ip4_addr);
  
      /*输入服务器端口号并且转换为整型存放在变量port_num中*/
      //提示输入服务器端口号
      printf("input server port number: ");
      //将输入的缓冲区的前BUFFER_SIZE个字节清零
      memset(stdin_buf, 0, BUFFER_SIZE);
      //从输入流中读取6个字节进入输入的缓冲区
      fgets(stdin_buf, 6, stdin);
      //将缓冲区的最后一个元素清零
      stdin_buf[5] = 0;
      //将字符串stdin_buf转换为int整数型port_num
      port_num = atoi(stdin_buf);
  
      /* set sockaddr_in */
      server_addr.sin_family = AF_INET;//指定IP地址类型为IPV4
      //htons(port_num):将整型变量的端口号从主机字节顺序转变成网络字节顺序
      server_addr.sin_port = htons(port_num);//存放端口信息
      //server_addr.sin_addr.s_addr = INADDR_ANY; 
      server_addr.sin_addr.s_addr = inet_addr(ip4_addr);//转换服务器ip地址
      //将sin_zero成员变量的前8个字节清零
      bzero(&(server_addr.sin_zero), 8); /* padding with 0's */
  
      int opt_val = 1;
      //SO_REUSEADDR:允许在bind ()过程中本地地址可重复使用
      //设置服务端的套接字允许在bind ()过程中本地地址可重复使用
      setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)); /* many options */
      
  
      addr_len = sizeof(struct sockaddr);
      //将套接字绑定到服务端的地址和端口
      ret = bind(server_fd, (struct sockaddr *)&server_addr, addr_len);
      if(ret == -1) {
          //绑定失败，关闭套接字，打印失败原因并退出程序
          close(server_fd);
          ERR_EXIT("bind()");    
      }
      //绑定成功，打印绑定成功
      printf("Bind success!\n");
      //让服务端的套接字进入被动监听状态，等待连接的队伍的最大长度为MAX_QUE_CONN_NM
      ret = listen(server_fd, MAX_QUE_CONN_NM);
      if(ret == -1) {
          //建立监听失败，关闭套接字，打印失败原因并退出程序
          close(server_fd);
          ERR_EXIT("listen()");
      }
      //建立监听成功，打印正在监听
      printf("Listening ...\n");
      //建立子进程，目的是为了建立进程同步
      pipe_pid = fork();
      if(pipe_pid < 0) {
          //建立进程失败，关闭服务端套接字，打印失败原因并退出程序
          close(server_fd);
          ERR_EXIT("fork()");
      }
      if(pipe_pid == 0) {
          //进入子进程：利用管道传输数据/消息
          pipe_data();
          exit(EXIT_SUCCESS); /* ignoring all the next statements */
      }
      
      max_sn = 0;
      connect_sn = 1;
      while (1) {
          //如果连接的客户端进程数大于最大的可连接进程数
          if(connect_sn > MAX_CONN_NUM) {
              //打印连接的客户端超过规定的范围，并打印现有的客户端
             printf("connect_sn = %d out of range\n", connect_sn);
             break;
          }
          addr_len = sizeof(struct sockaddr); /* should be assigned each time accept() called */
          //服务端接收第connect_sn个客户端请求
          //connect_addr:结果参数,接收放回客户端的地址，客户端地址的长度
          connect_fd[connect_sn] = accept(server_fd, (struct sockaddr *)&connect_addr, &addr_len);
          if(connect_fd[connect_sn] == -1) {
              //接收失败，打印接收失败，继 续接听下一个客户端
              perror("accept()");
              continue;
          }
          //客户端地址的端口号从网络字节顺序转换为主机字节顺序。
          port_num = ntohs(connect_addr.sin_port);
          //将网络二进制的数字的端口号转换成网络地址赋值给ip4_addr
          strcpy(ip4_addr, inet_ntoa(connect_addr.sin_addr));
          //打印新连接的客户端是第几个连接上的客户端，以及客户端的标识符，地址以及端口号
          printf("New connection sn = %d, fd = %d, IP_addr = %s, port = %hu\n", connect_sn, connect_fd[connect_sn], ip4_addr, port_num);
          //将服务端接收状态设置为已经有接收到的客户端，还可以继续接收客户端
          stat = STAT_NORMAL;
          //将客户端的个数（是第几个连接上的客户端），以及客户端个数的状态按照整型的形式写入到缓冲区stdin_buf
          sprintf(stdin_buf, "%d,%d", connect_sn, stat);
          //将缓冲区的内容写到客户端的的状态位中
          ret = write(fd_stat[1], stdin_buf, sizeof(stdin_buf)); /* blocking write ordinary pipe to pipe_data() */
          if(ret <= 0) {
              //写入失败，打印在写入过程中发生错误
              perror("fd_stat write() from recv_send_data() to pipe_data()");
          }
  		//申请子进程       
          recv_pid = fork();
          if(recv_pid < 0) {//子进程建立失败，打印在建立子进程的过程中发生错误，退出循环
              perror("fork()");
              break;
          }
          if(recv_pid == 0) {//进入子程序
              //同步调用服务端与特定连接上的客户端发送和接收信息的程序
              recv_send_data(connect_sn);
              exit(EXIT_SUCCESS); /* ignoring all the next statements */
          }
          ret = max_sn = connect_sn;
          //write()会把参数max_sn所指的内存写入sizeof(max_sn)个字节到参数fd[0][1]所指的文件内
          write(fd[0][1], &max_sn, sizeof(max_sn)); /* blocking write ordinary pipe to pipe_data() */
          if(ret <= 0) {
              //写入失败，打印在写入过程中发生错误
              perror("fd_stat write() from recv_send_data() to pipe_data()");
          }
          connect_sn++;//连接到的客户端个数+1
          /* parent pro continue to listen to a new client forever */
      }
  
      wait(0);//等待所有子进程结束
      for (int sn = 1; sn <= max_sn; sn++) {
          //关闭所有的客户端管道
          close(connect_fd[sn]);
      }
      //关闭服务端管道
      close(server_fd);
      exit(EXIT_SUCCESS);
  }
  
  
  ```


* 整体逻辑解释：

  ![image-20210422111158384](http://hurq5.gitee.io/os-pictures/image-20210422111158384.png)

  * 服务端处：

    程序服务端建立套接字，进行与客户端的链接，调用socket()创建了的服务端套接字server_fd，服务端连接到本地IP地址，以及输入的端口号，调用bind()让服务端的套接字与地址绑定，调用listen()函数监听客户端的连接请求，建立监听成功后调用accept()接收客户端的连接请求并建立连接，每连接到一个客户端，调用recv_send_data()函数通过socket的机制接收来自服务器的消息，从套接字connect_fd[sn]中读消息，写到消息管道fd_msg[1]中，pipe_data()函数读取消息管道fd_msg[1]中的消息，写到指定sn的客户端的管道fd[sn]中，传输给指定的客户端，程序还建立fd_stat管道用来传输客户端的状态，当客户端未被建立连接时，标志位为0，当客户端被建立连接时，标志位为1，当客户端断开连接时，标志位为-1；

    ![image-20210422112115687](http://hurq5.gitee.io/os-pictures/image-20210422112115687.png)

  


  * 客户端：

    客户端调用socket()创建了的客户端套接字connect_fd，调用connect()函数连接到服务端端后，recv_send_data()函数通过套接字接收客户端的消息，input的程序通过管道fd[sn]输入指定sn的客户端的消息，每个input程序对应一个connect程序，他们共用一个管道进行消息的传输。

* 运行结果过程说明：

  1. 使用ifconfig指令查看网卡的ip地址如图所示：

     ![image-20210421145803930](http://hurq5.gitee.io/os-pictures/image-20210421145803930.png)

     分析：可以观察到本机的ip地址为10.0.2.15

  2. 运行alg.11-5-socket-server-BBS-3.c 主程序

     服务器端一直监听本机的8000号端口，如果收到连接请求，将接收请求并接收客户端发来的消息。

     * 使用以下指令编译运行程序：

       ```
       gcc -std=c99 -D _SVID_SOURCE alg.11-5-socket-server-BBS-3.c
       ```

       因为版本的不同可能string.h库中可能没有bzero函数，因此编译不过，这个时候可以使用memset(a,0,b)函数来替代bzero(a,b)的功能，因此我们可以在头文件中加入替代宏定义：

       ```
       #define bzero(a,b) memset(a,0,b)
       ```

     * 使用以下命令运行程序：

       ```
       ./a.out testone
       ```

     * 输入不易被占用的8000端口，程序处于监听状态

       ![image-20210421152859526](http://hurq5.gitee.io/os-pictures/image-20210421152859526.png)

       可见：服务端的ip地址为10.0.2.15，套接字成功绑定该ip地址以及8000的端口号

  3. 分别建立三个客户端，并连接到服务端：

     * 使用以下命令编译连接程序：

       ```
       gcc alg.11-4-socket-connector-BBS-2.c -o connect.o
       ```

     * 开三个终端分别使用以下命令运行程序：

       ```
       ./connect.o test_frist_client
       ./connect.o test_secend_client
       ./connect.o test_third_client
       ```

       三个客户端处都输入服务端的ip地址10.0.2.15，以及正在被监听的端口号8000

       第一个客户端：

       ![image-20210422080910549](http://hurq5.gitee.io/os-pictures/image-20210422080910549.png)

       分析：可以观察到客户端一成功的连接到服务端，客户端的ip地址为10.0.2.15，端口号被分配为49900.

       第二个客户端：

       ![image-20210422080939824](http://hurq5.gitee.io/os-pictures/image-20210422080939824.png)

       分析：可以观察到客户端一成功的连接到服务端，客户端的ip地址为10.0.2.15，端口号被分配为49901.

       第三个客户端：

       ![image-20210422081116927](http://hurq5.gitee.io/os-pictures/image-20210422081116927.png)

       分析：可以观察到客户端一成功的连接到服务端，客户端的ip地址为10.0.2.15，端口号被分配为49902.

       在服务端这边执行的终端也显示了成功接收到3个客户端，结果如下：

       ![image-20210422081237296](http://hurq5.gitee.io/os-pictures/image-20210422081237296.png)

       分析：

       如图所示：

       1. 连接到第一个客户端时，编号为1，该客户端和服务端通信的管道标识符为31，客户端的ip地址为10.0.2.15，端口号为49900，客户端连接上服务端，状态从0改变为1，服务端已经连接上的最大客户端数量变为1
       2. 连接到第二个客户端时，编号为2，该客户端和服务端通信的管道标识符为32，客户端的ip地址为10.0.2.15，端口号为49901，客户端连接上服务端，状态从0改变为1，服务端已经连接上的最大客户端数量变为2
       3. 连接到第一个客户端时，编号为3，该客户端和服务端通信的管道标识符为33，客户端的ip地址为10.0.2.15，端口号为49902，客户端连接上服务端，状态从0改变为1，服务端已经连接上的最大客户端数量变为3

       

  4. 开3个终端分别运行3个客户端的input程序：

     * 使用以下命令编译程序：

       ```
       gcc alg.11-3-socket-input-2.c -o input.o
       ```

     * 开三个终端分别使用以下命令执行程序：

       ```
       ./input.o test_frist_client
       ./input.o test_secend_client
       ./input.o test_third_client
       ```

     * 得到三个客户端输入方式：

       ![image-20210422081953612](http://hurq5.gitee.io/os-pictures/image-20210422081953612.png)

       ![image-20210422082003503](http://hurq5.gitee.io/os-pictures/image-20210422082003503.png)

       ![image-20210422082014283](http://hurq5.gitee.io/os-pictures/image-20210422082014283.png)

  5. 客户端使用广播的形式向其他的客户端发送信息：

     * 使用编号为1的客户端以广播的形式向其他客户端（包括自己）发送消息：

       ![image-20210422083104370](http://hurq5.gitee.io/os-pictures/image-20210422083104370.png)

     * 先存在的三个客户端都接收到了hello的消息，因为编号为1的客户端尚未改名，名字被初始化为”Anonymous“，表示客户端匿名的意思：

       ![image-20210422083221474](http://hurq5.gitee.io/os-pictures/image-20210422083221474.png)

       ![image-20210422083250896](http://hurq5.gitee.io/os-pictures/image-20210422083250896.png)

       ![image-20210422083258331](http://hurq5.gitee.io/os-pictures/image-20210422083258331.png)

     * 观察服务端界面的情况，如下图所示：

       ![image-20210422083339266](http://hurq5.gitee.io/os-pictures/image-20210422083339266.png)

       分析：可以观察到服务端从sn=1的客户端接收到消息”hello“，并向所有的客户端（sn=1，2，3）发送该消息成功。

  6. 分别修改三个客户端的名字，sn=1的客户端改成client1，sn=2的客户端的昵称改成client2，sn=3的客户端的昵称改为client3

     在这里具体以第二个客户端为例子：

     * 在sn=2的客户端的输入终端处输入”#1client2“

       ![image-20210422083926121](http://hurq5.gitee.io/os-pictures/image-20210422083926121.png)

     * 观察服务端界面的情况，如下图所示：

       ![image-20210422083958878](http://hurq5.gitee.io/os-pictures/image-20210422083958878.png)

       分析：可以观察到服务端从sn=2的客户端接收到消息”#1client2“，并将客户端2的昵称从原来的”Anonymous“改成新的昵称“client2”

     * 观察连接程序界面，如下所示：

       ![image-20210422084155403](http://hurq5.gitee.io/os-pictures/image-20210422084155403.png)

       分析：控制台说明昵称已经被修改成client2

     * 同理修改sn=1的客户端的昵称为client1，sn=3的客户端的昵称为client3：根据以下的服务端界面可以得出修改成功：

       ![image-20210422084457367](http://hurq5.gitee.io/os-pictures/image-20210422084457367.png)

  7. 使用@私聊特定昵称的客户端，这里使用client1私聊client3

     * 在第一个客户端的输入端口输入“@client3 it‘s1"

       ![image-20210422085131199](http://hurq5.gitee.io/os-pictures/image-20210422085131199.png)

     * 观察服务端界面的情况，如下图所示：

       ![image-20210422085356767](http://hurq5.gitee.io/os-pictures/image-20210422085356767.png)

       分析：可以观察到服务端接收到客户端“@client3 it‘s1"的消息，并向@的对象（sn=3的客户端）发送消息“client1@： it‘s1”，表示消息是由client1私聊发送的，消息的内容为“it’s1”

     * 观察连接程序界面，如下图所示，可以观察到只用客户端3接收到了该私聊消息：

       客户端1：

       ![image-20210422085633510](http://hurq5.gitee.io/os-pictures/image-20210422085633510.png)

       客户端2：

       ![image-20210422085643697](http://hurq5.gitee.io/os-pictures/image-20210422085643697.png)

       客户端3：![image-20210422085656409](http://hurq5.gitee.io/os-pictures/image-20210422085656409.png)

  8. 程序的昵称有防重的作用，当我们把sn=1的客户端的昵称修改为client3（和sn=3的客户端的昵称相同），昵称修改失败。

     * 在sn=1的客户端的输入界面输入“#1client3”，表示把自己的昵称修改成client3：

       ![image-20210422090206566](http://hurq5.gitee.io/os-pictures/image-20210422090206566.png)

     * 观察服务端界面的情况，如下图所示：

       ![image-20210422090248177](http://hurq5.gitee.io/os-pictures/image-20210422090248177.png)

       分析：可以观察到服务端接收到客户端“#1client3 "的消息，尝试修改昵称为client3，遍历所有的客户端，发现有同名情况，服务端不再修改昵称，并通过管道向客户端1发送“this nickname occupied：client3“的消息，表示该昵称已经被占用了。

     * 观察sn=1的客户端的连接程序界面，如下图所示：

       ![image-20210422090553396](http://hurq5.gitee.io/os-pictures/image-20210422090553396.png)

       分析：客户端接收到服务端发送过来的消息，打印该昵称已经被占用的信息

  9. 退出客户端3的程序，服务端接收到客户端欲退出的消息后，改变客户端的状态位，断开与其的连接，并向其他客户端发送该客户端退出聊天室的消息：

     * 在客户端3的输入界面输入“#0”，表示自己要退出聊天室：

       ![image-20210422092214834](http://hurq5.gitee.io/os-pictures/image-20210422092214834.png)

     * 观察服务端界面的情况，如下图所示：

       ![image-20210422092240265](http://hurq5.gitee.io/os-pictures/image-20210422092240265.png)

       分析：

       1. 服务端收到客户端3“#0”的消息后，知道客户端3要断开与其的连接，修改标志位为-1，表示客户端已经退出聊天室。
       2. 当客户端收到输入程序通过管道发来的消息"#0"消息时候，不仅会把“#0”的消息转发给服务端，还会向服务端发送“I quit”的信息。
       3. 当服务端接收到客户端3“I quit”的消息后，将该消息以广播的方式发给所有的客户端，消息的头部添加退出聊天室的客户端名称client3，这样能够更加明确的告诉其他用户，哪一个用户退出了聊天室。

     * 观察各个客户端的连接程序界面，如下图所示：

       客户端1：

       ![image-20210422093411295](http://hurq5.gitee.io/os-pictures/image-20210422093411295.png)

       分析：客户端1成功接收到昵称为client3（即客户端3）退出聊天室的通告。

       客户端2：

       ![image-20210422093537207](http://hurq5.gitee.io/os-pictures/image-20210422093537207.png)

       分析：客户端2成功接收到昵称为client3（即客户端3）退出聊天室的通告。

       客户端3：

       ![image-20210422093704509](http://hurq5.gitee.io/os-pictures/image-20210422093704509.png)

       分析：客户端3断开和服务端的连接，退出程序。

