/*服务端*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAXBUF 1024
char recv_buf[MAXBUF+1];
char link_state=1;
int aa;
void *fun(int x)
{
   int new_fd=x;//(int)(*((int*)x));
   while(1)
   {
     bzero(recv_buf, MAXBUF + 1);
     scanf("%s",recv_buf);
     int len = send(new_fd, recv_buf, sizeof(recv_buf) , 0);
     printf("server:send ok\n");
     if(len<=0)
        break;
   }
    exit(0);
}


int main(int argc, char *argv[])
{
    int server_sockfd;//服务器端套接字
    int client_sockfd;//客户端套接字
    int len;
    struct sockaddr_in my_addr;   //服务器网络地址结构体
    char buf[BUFSIZ];  //数据传送的缓冲区
    memset(&my_addr,0,sizeof(my_addr)); //数据初始化--清零
    my_addr.sin_family=AF_INET; //设置为IP通信
    my_addr.sin_addr.s_addr=INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上
    my_addr.sin_port=htons(8000); //服务器端口号

    /*创建服务器端套接字--IPv4协议，面向连接通信，TCP协议*/
    if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {  
        perror("socket error");
        return 1;
    }

        /*将套接字绑定到服务器的网络地址上*/
    if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)
    {
        perror("bind error");
        return 1;
    }

    /*监听连接请求--监听队列长度为5*/
    listen(server_sockfd,5);
    bool flag = true;
    while(flag) {
        struct sockaddr_in remote_addr; //客户端网络地址结构体
        socklen_t sin_size = sizeof(remote_addr);
        /*等待客户端连接请求到达*/
        if((client_sockfd=accept(server_sockfd,(struct sockaddr *)&remote_addr,&sin_size))<0)
        {
            perror("accept");
            return 1;
        }
        printf("accept client %s\n",inet_ntoa(remote_addr.sin_addr));

        aa=client_sockfd;//可以直接传递client_sockfd.
        //创建线程为了给客户端发送数据
        pthread_t ntid;
        int a = pthread_create(&ntid,NULL,fun,aa);
        if(a=0)
            printf("ok\n"); 
        while(1){
            memset(buf,0,sizeof buf);
            /*接收客户端的数据并将其发送给客户端--recv返回接收到的字节数，send返回发送的字节数*/
            int recv_len  = recv(client_sockfd,buf,BUFSIZ,0);
            if(recv_len<0){
                perror("recv error %s errno: %d\n");
                continue;
            }
            //添加char[]结束标志
            buf[recv_len] = '\0';
            printf("server:recvived:%s\n",buf);
            if(strcmp(buf,"quit")==0){
                printf("Conversation will be closed.\n");
                break;
            }

            if(strcmp(buf,"allquit")==0){
                printf("Server will be closed.\n");
                flag = false;
                break;
            }
        }
        close(client_sockfd);
        client_sockfd = -1;
    }
    close(server_sockfd);
    server_sockfd=-1;
    return 0;
}