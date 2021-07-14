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

