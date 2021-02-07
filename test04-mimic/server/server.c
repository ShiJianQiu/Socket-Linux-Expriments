/*server端*/

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#define BACKLOG 5        //完成三次握手但没有accept的队列的长度
#define CONCURRENT_MAX 8 //应用层同时可以处理的连接
#define SERVER_PORT_CLIENT 11332
#define SERVER_PORT_MIMIC 11333
#define BUFFER_SIZE 1024
#define QUIT_CMD ".quit"
#define WAITE_TIME_MAX 100
int client_fds[CONCURRENT_MAX];
int mimic_fd;
struct sockaddr_in server_addr_clint,server_addr_mimic;
char input_msg[BUFFER_SIZE];
char recv_msg[BUFFER_SIZE];

int main(int argc, char *argv[])
{
    //本地地址
    server_addr_clint.sin_family = AF_INET;
    server_addr_mimic.sin_family = AF_INET;
    server_addr_clint.sin_port = htons(SERVER_PORT_CLIENT);
    server_addr_mimic.sin_port = htons(SERVER_PORT_MIMIC);
    server_addr_clint.sin_addr.s_addr = inet_addr("0.0.0.0");
    server_addr_mimic.sin_addr.s_addr = inet_addr("0.0.0.0");
    bzero(&(server_addr_clint.sin_zero), 8);
    bzero(&(server_addr_mimic.sin_zero), 8);
    //创建socket
    int server_client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_client_sock_fd == -1)
    {
        perror("build client socket error");
        return 1;
    }
    int server_mimic_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_mimic_sock_fd == -1)
    {
        perror("build mimic socket error");
        return 1;
    }
    //绑定socket
    int client_bind_result = bind(server_client_sock_fd, (struct sockaddr *)&server_addr_clint, sizeof(server_addr_clint));
    if (client_bind_result == -1)
    {
        perror("client socket bind error");
        return 1;
    }
    int mimic_bind_result = bind(server_mimic_sock_fd, (struct sockaddr *)&server_addr_mimic, sizeof(server_addr_mimic));
    if (mimic_bind_result == -1)
    {
        perror("mimic socket bind error");
        return 1;
    }
    //listen
    if (listen(server_client_sock_fd, BACKLOG) == -1)
    {
        perror("server client listen error");
        return 1;
    }
    //listen
    if (listen(server_mimic_sock_fd, BACKLOG) == -1)
    {
        perror("server mimic listen error");
        return 1;
    }
    //fd_set
    fd_set server_fd_set;
    int max_fd = -1;
    struct timeval tv; //超时时间设置
    while (1)
    {
        tv.tv_sec = 20;
        tv.tv_usec = 0;
        FD_ZERO(&server_fd_set);
        //将STDIN_FILENO添加入set中
        FD_SET(STDIN_FILENO, &server_fd_set);
        if (max_fd < STDIN_FILENO)
        {
            max_fd = STDIN_FILENO;
        }
        printf("set STDIN_FILENO into server_fd_set, STDIN_FILENO=%d\n", STDIN_FILENO);
        //服务器端socket，添加到set中
        FD_SET(server_client_sock_fd, &server_fd_set);
        // printf("server_client_sock_fd=%d\n", server_client_sock_fd);
        if (max_fd < server_client_sock_fd)
        {
            max_fd = server_client_sock_fd;//select需要max_fd+1
        }
        printf("set server_client_sock_fd into server_fd_set, server_client_sock_fd=%d\n", server_client_sock_fd);
        FD_SET(server_mimic_sock_fd, &server_fd_set);
        if (max_fd < server_mimic_sock_fd)
        {
            max_fd = server_mimic_sock_fd;//select需要max_fd+1
        }
        printf("set server_mimic_sock_fd into server_fd_set, server_mimic_sock_fd=%d\n", server_mimic_sock_fd );

        //客户端连接
        for (int i = 0; i < CONCURRENT_MAX; i++)
        {
            //printf("client_fds[%d]=%d\n", i, client_fds[i]);
            if (client_fds[i] != 0)
            {
                FD_SET(client_fds[i], &server_fd_set);
                if (max_fd < client_fds[i])
                {
                    max_fd = client_fds[i];//select需要max_fd+1
                }
            }
        }
        int ret = select(max_fd + 1, &server_fd_set, NULL, NULL, &tv);
        if (ret < 0)
        {
            perror("select 出错\n");
            continue;
        }
        else if (ret == 0)
        {
            printf("select 超时\n");
            continue;
        }
        else
        {
            //ret 为未状态发生变化的文件描述符的个数
            if (FD_ISSET(STDIN_FILENO, &server_fd_set))
            {
                printf("发送消息：\n");
                bzero(input_msg, BUFFER_SIZE);
                fgets(input_msg, BUFFER_SIZE, stdin);
                //输入“.quit"则退出服务器
                if (strcmp(input_msg, QUIT_CMD) == 0)
                {
                    exit(0);
                }
                for (int i = 0; i < CONCURRENT_MAX; i++)
                {
                    if (client_fds[i] != 0)
                    {
                        printf("client_fds[%d]=%d\n", i, client_fds[i]);
                        send(client_fds[i], input_msg, BUFFER_SIZE, 0);
                    }
                }
            }
            if(FD_ISSET(server_mimic_sock_fd, &server_fd_set)){
                struct sockaddr_in mimic_address;
                socklen_t address_len;
                int mimic_sock_fd = accept(server_mimic_sock_fd, (struct sockaddr *)&mimic_address, &address_len);
                printf("new connection mimic_sock_fd = %d\n", mimic_sock_fd);
                if(mimic_sock_fd > 0){
                    if(mimic_sock_fd > 0){
                        mimic_fd = mimic_sock_fd;
                        printf("新中转端(mimic_fd = %d)加入成功 %s:%d\n", mimic_fd, inet_ntoa(mimic_address.sin_addr), ntohs(mimic_address.sin_port));
                    }else{
                        //将input_msg清零
                        bzero(input_msg, BUFFER_SIZE);
                        strcpy(input_msg, "服务器已有中转节点，无法加入!\n");
                        send(mimic_sock_fd, input_msg, BUFFER_SIZE, 0);
                        printf("中转端连接数达到最大值，新中转端加入失败 %s:%d\n", inet_ntoa(mimic_address.sin_addr), ntohs(mimic_address.sin_port));
                    }
                }

            }
            if (FD_ISSET(server_client_sock_fd, &server_fd_set))
            {
                //有新的连接请求
                struct sockaddr_in client_address;
                socklen_t address_len;
                int client_sock_fd = accept(server_client_sock_fd, (struct sockaddr *)&client_address, &address_len);
                printf("new connection client_sock_fd = %d\n", client_sock_fd);
                if (client_sock_fd > 0)
                {
                    int index = -1;
                    for (int i = 0; i < CONCURRENT_MAX; i++)
                    {
                        if (client_fds[i] == 0)
                        {
                            index = i;
                            client_fds[i] = client_sock_fd;
                            break;
                        }
                    }
                    if (index >= 0)
                    {
                        printf("新客户端(%d)加入成功 %s:%d\n", index, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                    }
                    else
                    {
                        bzero(input_msg, BUFFER_SIZE);
                        strcpy(input_msg, "服务器加入的客户端数达到最大值,无法加入!\n");
                        send(client_sock_fd, input_msg, BUFFER_SIZE, 0);
                        printf("客户端连接数达到最大值，新客户端加入失败 %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                    }
                }
            }
            for (int i = 0; i < CONCURRENT_MAX; i++)
            {
                if (client_fds[i] != 0)
                {
                    if (FD_ISSET(client_fds[i], &server_fd_set))
                    {
                        //处理某个客户端过来的消息
                        bzero(recv_msg, BUFFER_SIZE);
                        long byte_num = recv(client_fds[i], recv_msg, BUFFER_SIZE, 0);
                        if (byte_num > 0)
                        {
                            if (byte_num > BUFFER_SIZE)
                            {
                                byte_num = BUFFER_SIZE;
                            }
                            //recv_msg[byte_num] = '\0';
                            printf("接受客户端(%d):%s", i, recv_msg);
                            /*转发数据给中转端*/
                            // for (int i = 0; i < CONCURRENT_MAX; i++)
                            // {
                            //     if (client_fds[i] != 0)
                            //     {
                            //         send(client_fds[i], recv_msg, sizeof(recv_msg), 0);
                            //     }
                            // }
                            if(mimic_fd != 0){
                                send(mimic_fd, recv_msg, sizeof(recv_msg), 0);
                                printf("转发给中转端(%d):%s", i, recv_msg);
                            }
                            /*结束转发内容*/
                            bzero(recv_msg, BUFFER_SIZE);
                            // int recv_len = recv(mimic_fd, recv_msg, BUFFER_SIZE, 0);
                        }
                        else if (byte_num < 0)
                        {
                            printf("从客户端(%d)接受消息出错.\n", i);
                        }
                        else
                        {
                            FD_CLR(client_fds[i], &server_fd_set);
                            client_fds[i] = 0;
                            printf("客户端(%d)退出了\n", i);
                        }
                    }
                }
            }
        }
    }
    return 0;
}