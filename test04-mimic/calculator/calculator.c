/*calculator端*/
#include<stdio.h>  
#include<stdlib.h>  
#include<netinet/in.h>  
#include<sys/socket.h>  
#include<arpa/inet.h>  
#include<string.h>

#include<time.h>

#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>

#define BUFFER_SIZE 1024  
#define SERVER_PORT_MIMIC 11334
#define QUIT_CMD ".quit"

int main(int argc, const char * argv[])  
{  
    srand((unsigned)time(NULL));
    struct sockaddr_in server_addr_clint;  
    server_addr_clint.sin_family = AF_INET;  
    server_addr_clint.sin_port = htons(SERVER_PORT_MIMIC);  
    server_addr_clint.sin_addr.s_addr = inet_addr("0.0.0.0");  
    bzero(&(server_addr_clint.sin_zero), 8);  
    //建立socket连接，AF_INET : TCP/IP-IPV4，SOCK_STREAM : TCP类型
    int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);  
    if(server_sock_fd == -1)  
    {  
        perror("socket error");
        return 1;  
    }  
    char recv_msg[BUFFER_SIZE];
    char input_msg[BUFFER_SIZE];
    //TCP,客户端主动连接服务器
    if(connect(server_sock_fd, (struct sockaddr *)&server_addr_clint, sizeof(struct sockaddr_in)) == 0)  
    {  
        fd_set client_fd_set;  //一个集合，这个集合中存放的是文件描述符(filedescriptor)，即文件句柄
        struct timeval tv;  //它指明我们要等待的时间：
        // open_dev();//打开驱动的设备节点.无则删除
        while(1)  
        {
            tv.tv_sec = 20;//秒
            tv.tv_usec = 0;//毫秒
            FD_ZERO(&client_fd_set); //将指定的文件描述符集清空，在对文件描述符集合进行设置前，必须对其进行初始化
            // FD_SET(STDIN_FILENO, &client_fd_set);//用于在文件描述符集合中增加一个新的文件描述符。
            /*
            STDIN_FILENO：接收键盘的输入
            STDOUT_FILENO：向屏幕输出
            */
            FD_SET(server_sock_fd, &client_fd_set);  
            //参数maxfd是需要监视的最大的文件描述符值+1；
            //int select(int maxfdp,fd_set *readfds,fd_set *writefds,fd_set *errorfds,struct timeval *timeout);
            /*
            int maxfdp是一个整数值，是指集合中所有文件描述符的范围，即所有文件描述符的最大值加1，不能错！在Windows中这个参数的值无所谓，可以设置不正确。
            fd_set*readfds是指向fd_set结构的 指针，这个集合中应该包括 文件描述符，我们是要监视这些文件描述符的读变化的，即我们关心是否可以从这些文件中读取数据了，如果这个集合中有一个文件可读，select就会返回一个大于0的值，表示有文件可读，如果没有可读的文件，则根据timeout参数再判断是否超时，若超出timeout的时间，select返回0，若发生错误返回负值。可以传入NULL值，表示不关心任何文件的读变化。
            fd_set*writefds是指向fd_set结构的指针，这个集合中应该包括文件描述符，我们是要监视这些文件描述符的写变化的，即我们关心是否可以向这些文件中写入数据了，如果这个集合中有一个文件可写，select就会返回一个大于0的值，表示有文件可写，如果没有可写的文件，则根据timeout参数再判断是否超时，若超出timeout的时间，select返回0，若发生错误返回负值。可以传入NULL值，表示不关心任何文件的写变化。
            fd_set *errorfds同上面两个参数的意图，用来监视文件错误异常。
            struct timeval *timeout是select的超时时间，这个参数至关重要，它可以使select处于三种状态，第一，若将NULL以 形参传入，即不传入时间结构，就是将select置于 阻塞状态，一定等到监视 文件描述符集合中某个文件描述符发生变化为止；第二，若将时间值设为0秒0毫秒，就变成一个纯粹的非 阻塞函数，不管文件描述符是否有变化，都立刻返回继续执行，文件无变化返回0，有变化返回一个正值；第三，timeout的值大于0，这就是等待的超时时间，即select在timeout时间内阻塞，超时时间之内有事件到来就返回了，否则在超时后不管怎样一定返回，返回值同上述。
            */
            select(server_sock_fd + 1, &client_fd_set, NULL, NULL, &tv);  
            // if(FD_ISSET(STDIN_FILENO, &client_fd_set))  //用于测试指定的文件描述符是否在该集合中。
            // {  
            //     bzero(input_msg, BUFFER_SIZE);  //置字节字符串s的前n个字节为零且包括‘\0’
            //     fgets(input_msg, BUFFER_SIZE, stdin);  
            //     if(send(server_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)  
            //     {  
            //         perror("发送消息出错!\n");  
            //     }  
            // }  
            if(FD_ISSET(server_sock_fd, &client_fd_set))  
            {  
                bzero(recv_msg, BUFFER_SIZE);  
                long byte_num = recv(server_sock_fd, recv_msg, BUFFER_SIZE, 0);  
                if(byte_num > 0)  
                {  
                    if(byte_num > BUFFER_SIZE)  
                    {  
                        byte_num = BUFFER_SIZE;  
                    }  
                    recv_msg[byte_num] = '\0';

                    /*此处可以添加控制信息*/

                    printf("中转端发来的消息: %s\n", recv_msg);  
                    int tmp = rand()%byte_num;
                    recv_msg[tmp] = recv_msg[tmp]+1;
                    send(server_sock_fd, recv_msg, BUFFER_SIZE, 0);
                }
                else if(byte_num < 0)  
                {  
                    printf("接受消息出错!\n");  
                }  
                else  
                {  
                    printf("服务器端退出!\n");  
                    exit(0);  
                }
                if(strcmp(recv_msg, QUIT_CMD) == 0){
                    printf("End conversation.\n");
                    break;
                }
            }  
        }    
    }  
    return 0;  
} 