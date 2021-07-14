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
    server_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);//转换服务器ip地址
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

