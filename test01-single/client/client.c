#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAX_READ_LINE 1024

int main(void) {
    char *server_ip_addr = "127.0.0.1";
    int server_ip_port = 10004;
    char send_message[MAX_READ_LINE];

    //建立socket连接，AF_INET : TCP/IP-IPV4，SOCK_STREAM : TCP类型
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        fprintf(stderr, "socket error %s errno: %d\n", strerror(errno), errno);
    }
    printf("创建SOCKET成功!\n");

    struct sockaddr_in t_sockaddr;
    memset(&t_sockaddr, 0, sizeof(struct sockaddr_in));
    t_sockaddr.sin_family = AF_INET;
    t_sockaddr.sin_port = htons(server_ip_port);
    //inet_pton是一个IP地址转换函数，可以在将点分文本的IP地址转换为二进制网络字节序”的IP地址，
    //而且inet_pton和inet_ntop这2个函数能够处理ipv4和ipv6
    inet_pton(AF_INET, server_ip_addr, &t_sockaddr.sin_addr);

    //TCP,客户端主动连接服务器
    if((connect(socket_fd, (struct sockaddr*)&t_sockaddr, sizeof(struct sockaddr))) < 0 ) {
        fprintf(stderr, "connect error %s errno: %d\n", strerror(errno), errno);
        return 0;
    }
    printf("连接成功!\n");
    while(1){
        //TCP发送数据
        memset(send_message, 0, 255);
        printf("发送:");
        //get input from keyboard using fileinput
        fgets(send_message,sizeof(send_message), stdin);
        strtok(send_message, "\n");
        if(strcmp(send_message, " ") == 0 || strcmp(send_message, "") == 0)
        {
        printf("Your input is empty.");
        continue;
        }
        if((send(socket_fd, send_message, strlen(send_message), 0)) < 0) {
            fprintf(stderr, "send message error: %s errno : %d", strerror(errno), errno);
            return 0;
        }
        if(strcmp(send_message, "quit") == 0||strcmp(send_message, "allquit") == 0)
        {
            printf("End conversation.\n");
            break;
        }
    }
    close(socket_fd);
    socket_fd = -1;

    return 0;
}

