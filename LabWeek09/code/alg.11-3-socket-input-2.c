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
