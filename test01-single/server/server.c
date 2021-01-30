#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define MAX_READ_LINE 1024

int main(void) {
    int recv_len = -1;
    int conn_fd = -1;
    int ret = -1;
    int server_ip_port = 10004;

    // sockaddr_in在头文件#include<netinet/in.h>或#include <arpa/inet.h>中定义，该结构体解决了sockaddr的缺陷，
    // 把port和addr 分开储存在两个变量中，如下：
    struct sockaddr_in t_sockaddr;
    memset(&t_sockaddr, 0, sizeof(t_sockaddr));
    t_sockaddr.sin_family = AF_INET;//AF_INET（TCP/IP – IPv4）
    t_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);//ip
    t_sockaddr.sin_port = htons(server_ip_port);//port

    //SOCK_STREAM TCP类型
    //AF_INET（TCP/IP – IPv4）
    //初始化创建socket对象
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        fprintf(stderr, "socket error %s errno: %d\n", strerror(errno), errno);
    }
    printf("创建SOCKET成功!\n");
    //将初始化的socket对象绑定特定ip、端口上
    ret = bind(listen_fd,(struct sockaddr *) &t_sockaddr,sizeof(t_sockaddr));
    if (ret < 0) {
        fprintf(stderr, "bind socket error %s errno: %d\n", strerror(errno), errno);
    }
    printf("绑定成功!\n");
    //启动监听服务
    ret = listen(listen_fd, 1024);
    if (ret < 0) {
        fprintf(stderr, "listen error %s errno: %d\n", strerror(errno), errno);
    }
    printf("监听成功!\n");
    bool flag = true;
    while(flag) {
        struct sockaddr_in client_addr;
        socklen_t client_addrlength = sizeof(client_addr);
 
        //TCP
        //从已经完成连接队列返回下一个简历成功的连接，如果队列为空进入阻塞态
        conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addrlength);
        if(conn_fd < 0) {
            fprintf(stderr, "accpet socket error: %s errno :%d\n", strerror(errno), errno);
            continue;
        }
        printf("有客户端连接进来了.IP:%s, port:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        //TCP类型的数据接收，返回数据大小
        char buff[MAX_READ_LINE];
        while(1){
            memset(buff, 0, sizeof(buff));
            recv_len = recv(conn_fd, buff, MAX_READ_LINE, 0);
            if (recv_len < 0) {
                fprintf(stderr, "recv error %s errno: %d\n", strerror(errno), errno);
                continue;
            }
            //添加char[]结束标志
            buff[recv_len] = '\0';
            fprintf(stdout, "recv message from client: %s\n", buff);
            if(strcmp(buff,"quit")==0){
                printf("Conversation will be closed.\n");
                break;
            }

            if(strcmp(buff,"allquit")==0){
                printf("Server will be closed.\n");
                flag = false;
                break;
            }
            printf(">> %s\n",buff);
        }
        close(conn_fd);
        conn_fd = -1;
    }

    close(listen_fd);
    listen_fd = -1;

    return 0;
}

