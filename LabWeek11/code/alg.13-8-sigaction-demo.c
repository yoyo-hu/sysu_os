#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
 
void my_handler(int signo)  /* user signal handler */
{
    printf("\nhere is my_handler");
    printf("\nsignal catched: signo = %d", signo);
    printf("\nCtrl+\\ is masked");
    printf("\nsleeping for 10 seconds ... \n");
    sleep(10); 
    printf("my_handler finished\n");
    printf("after returned to the main(), Ctrl+\\ is unmasked\n");
    return;
}

int main(void)
{
    int ret;
    struct sigaction newact;
 
    newact.sa_handler = my_handler; /* set the user handler */
    //初始化信号集newact.sa_mask,使其中所有信号的对应的bit清零，表示该信号集不包含任何有效信号.
    sigemptyset(&newact.sa_mask); /* clear the mask */
    //在该信号集中添加SIGQUIT信号
    //用户在终端按下ctrl-\时，终端驱动程序会发送信号SIGQUIT信号给前台进程
    sigaddset(&newact.sa_mask, SIGQUIT); /* sa_mask, set signo=3 (SIGQUIT:Ctrl+\) */
    newact.sa_flags = 0; /* default */

    printf("now start catching Ctrl+c\n");
    //sigaction()会依SIGINT指定的信号编号来设置该信号的处理函数
    ret = sigaction(SIGINT, &newact, NULL); /* register signo=2 (SIGINT:Ctrl+C) */
    if(ret == -1) {
        perror("sigaction()");
        exit(1);
    }
 
    while (1);
 
    return 0;
}
