/*mimic端*/

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/select.h>
#define BACKLOG 5        //完成三次握手但没有accept的队列的长度
#define CONCURRENT_MAX 8 //应用层同时可以处理的连接
#define SERVER_PORT_MASTER 11333
#define SERVER_PORT_CALCULATOR 11334
#define SERVER_PORT_CLIENT 11335
#define BUFFER_SIZE 1024
#define QUIT_CMD ".quit"
int client_fds[CONCURRENT_MAX];
int calculator_fds[CONCURRENT_MAX];
char input_msg[BUFFER_SIZE];
char recv_msg[BUFFER_SIZE];

enum{DEFAULT,OK,CLOSE,ERROR};
struct mimic_elements{
    int fd;
    long receive_len;
    char recv_msg[BUFFER_SIZE];
    int statue;
};
struct mimic_elements *send_to_calculator[CONCURRENT_MAX];

void *send_and_receive(void *x){
    struct mimic_elements *tmp = x;
    int new_fd=tmp->fd;
    send(new_fd, tmp->recv_msg, sizeof(tmp->recv_msg) , 0);
    tmp->receive_len = recv(new_fd, tmp->recv_msg, sizeof(tmp->recv_msg) , 0);
    if(tmp->receive_len>0)tmp->statue=OK;
    else if(tmp->receive_len<0)tmp->statue=ERROR;
    else tmp->statue=CLOSE;
    exit(0);
}
int main(int argc, const char *argv[])
{

    //本地地址
    struct sockaddr_in server_addr_client;
    server_addr_client.sin_family = AF_INET;
    server_addr_client.sin_port = htons(SERVER_PORT_CLIENT);
    server_addr_client.sin_addr.s_addr = inet_addr("0.0.0.0");
    bzero(&(server_addr_client.sin_zero), 8);
    struct sockaddr_in server_addr_calculator;
    server_addr_calculator.sin_family = AF_INET;
    server_addr_calculator.sin_port = htons(SERVER_PORT_CALCULATOR);
    server_addr_calculator.sin_addr.s_addr = inet_addr("0.0.0.0");
    bzero(&(server_addr_calculator.sin_zero), 8);
    struct sockaddr_in server_addr_master;
    server_addr_master.sin_family = AF_INET;  
    server_addr_master.sin_port = htons(SERVER_PORT_MASTER);  
    server_addr_master.sin_addr.s_addr = inet_addr("0.0.0.0");
    bzero(&(server_addr_master.sin_zero), 8); 
    //创建socket
    int server_client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_client_sock_fd == -1)
    {
        perror("client socket error");
        return 1;
    }
    //创建socket
    int server_calculator_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_calculator_sock_fd == -1)
    {
        perror("calculator socket error");
        return 1;
    }
    //建立socket连接，AF_INET : TCP/IP-IPV4，SOCK_STREAM : TCP类型
    int server_master_sock_fd = socket(AF_INET, SOCK_STREAM, 0);  
    if(server_master_sock_fd == -1)  
    {  
        perror("build master socket error");
        return 1;
    }  
    //绑定socket
    int bind_client_result = bind(server_client_sock_fd, (struct sockaddr *)&server_addr_client, sizeof(server_addr_client));
    if (bind_client_result == -1)
    {
        perror("client bind error");
        return 1;
    }
    //绑定socket
    int bind_calculator_result = bind(server_calculator_sock_fd, (struct sockaddr *)&server_addr_calculator, sizeof(server_addr_calculator));
    if (bind_calculator_result == -1)
    {
        perror("calculator bind error");
        return 1;
    }
    
    //listen
    if (listen(server_client_sock_fd, BACKLOG) == -1)
    {
        perror("client listen error");
        return 1;
    }
    //listen
    if (listen(server_calculator_sock_fd, BACKLOG) == -1)
    {
        perror("calculator listen error");
        return 1;
    }
    //fd_set
    fd_set server_fd_set;
    int max_fd = -1;
    struct timeval tv; //超时时间设置
    while (1)
    {
        //TCP,客户端主动连接服务器
        if(connect(server_master_sock_fd, (struct sockaddr *)&server_addr_master, sizeof(struct sockaddr_in)) < 0)  
        { 
            printf("Master服务器端连接失败!\n");
            break;
        }  
        tv.tv_sec = 20;
        tv.tv_usec = 0;
        FD_ZERO(&server_fd_set);
        //将STDIN_FILENO添加入set中
        FD_SET(STDIN_FILENO, &server_fd_set);
        if (max_fd < STDIN_FILENO)
        {
            max_fd = STDIN_FILENO;
        }
        //将STDIN_FILENO添加入set中
        FD_SET(server_master_sock_fd, &server_fd_set);
        if (max_fd < server_master_sock_fd)
        {
            max_fd = server_master_sock_fd;
        }
        //printf("STDIN_FILENO=%d\n", STDIN_FILENO);
        //服务器端socket，添加到set中
        FD_SET(server_client_sock_fd, &server_fd_set);
        // printf("server_client_sock_fd=%d\n", server_client_sock_fd);
        if (max_fd < server_client_sock_fd)
        {
            max_fd = server_client_sock_fd;//select需要max_fd+1
        }
        //服务器端socket，添加到set中
        FD_SET(server_calculator_sock_fd, &server_fd_set);
        // printf("server_client_sock_fd=%d\n", server_client_sock_fd);
        if (max_fd < server_calculator_sock_fd)
        {
            max_fd = server_calculator_sock_fd;//select需要max_fd+1
        }
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
        //calculator连接
        for (int i = 0; i < CONCURRENT_MAX; i++)
        {
            //printf("client_fds[%d]=%d\n", i, client_fds[i]);
            if (calculator_fds[i] != 0)
            {
                FD_SET(calculator_fds[i], &server_fd_set);
                if (max_fd < calculator_fds[i])
                {
                    max_fd = calculator_fds[i];//select需要max_fd+1
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
            }
            //master有消息
            if (FD_ISSET(server_master_sock_fd, &server_fd_set)){
                bzero(recv_msg, BUFFER_SIZE);  
                long byte_num = recv(server_master_sock_fd, recv_msg, BUFFER_SIZE, 0);  
                if(byte_num > 0)  
                {  
                    if(byte_num > BUFFER_SIZE)  
                    {  
                        byte_num = BUFFER_SIZE;  
                    }  
                    recv_msg[byte_num] = '\0';
                    
                    /*此处可以添加控制信息*/
                    /*转发数据给其他的客户端*/
                    /*总*/
                    /*
                    - 主线程 mimic
                    -- 总转发任务A
                    --- 分转发任务A[i]  （时限）
                    --- 一致性判断
                    --（）
                    */
                    pthread_t ntid[CONCURRENT_MAX];
                    for (int i = 0; i < CONCURRENT_MAX; i++)
                    {
                        if (calculator_fds[i] != 0)
                        {
                            if(send_to_calculator[i]==NULL){
                                send_to_calculator[i] = malloc(sizeof(send_to_calculator[i]));
                                send_to_calculator[i]->statue = DEFAULT;
                            }
                            strcpy(send_to_calculator[i]->recv_msg,recv_msg);
                            pthread_create(&ntid[i],NULL,send_and_receive,calculator_fds[i]);
                            // send(calculator_fds[i], recv_msg, sizeof(recv_msg), 0);
                        }
                        if (send_to_calculator[i]->statue = CLOSE){
                            FD_CLR(calculator_fds[i], &server_fd_set);
                            calculator_fds[i] = 0;
                            printf("CAL端(%d)退出了\n", i);
                        }
                    }
                    printf("服务器:%s\n", recv_msg); 
                    long byte_num_mimic = 0;
                    for (int i = 0; i < CONCURRENT_MAX; i++)
                    {
                        /* code */
                        if (calculator_fds[i] != NULL && send_to_calculator[i]->receive_len > 0&&byte_num_mimic!=0){
                            //谁id小，谁正确
                            printf("calculator(%d):%s", i, send_to_calculator[i]->recv_msg);
                            bzero(recv_msg, BUFFER_SIZE);
                            strcpy(recv_msg,send_to_calculator[i]->recv_msg);
                            byte_num_mimic = send_to_calculator[i]->receive_len;
                        }
                        //清空
                        if (calculator_fds[i] != NULL){
                            send_to_calculator[i]->statue = DEFAULT;
                            send_to_calculator[i]->receive_len = 0;
                            bzero(send_to_calculator[i]->recv_msg, BUFFER_SIZE);
                        }
                    }
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
            if (FD_ISSET(server_calculator_sock_fd, &server_fd_set))
            {
                //有新的连接请求
                struct sockaddr_in calculator_address;
                socklen_t address_len;
                int calculator_sock_fd = accept(server_calculator_sock_fd, (struct sockaddr *)&calculator_address, &address_len);
                printf("new connection calculator_sock_fd = %d\n", calculator_sock_fd);
                if (calculator_sock_fd > 0)
                {
                    int index = -1;
                    for (int i = 0; i < CONCURRENT_MAX; i++)
                    {
                        if (calculator_fds[i] == 0)
                        {
                            index = i;
                            calculator_fds[i] = calculator_sock_fd;
                            break;
                        }
                    }
                    if (index >= 0)
                    {
                        printf("新cal端(%d)加入成功 %s:%d\n", index, inet_ntoa(calculator_address.sin_addr), ntohs(calculator_address.sin_port));
                    }
                    else
                    {
                        bzero(input_msg, BUFFER_SIZE);
                        strcpy(input_msg, "服务器加入的cal端数达到最大值,无法加入!\n");
                        send(calculator_sock_fd, input_msg, BUFFER_SIZE, 0);
                        printf("cal端连接数达到最大值，新cal端加入失败 %s:%d\n", inet_ntoa(calculator_address.sin_addr), ntohs(calculator_address.sin_port));
                    }
                }
            }
        }
    }
    return 0;
}