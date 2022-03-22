#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h> //define the sockaddr_un structure

int num_reverse(int num)
{
    int S=0,sum=0;
    
    while(num)
    {
        S=num%10;
        sum=10*sum+S;
        num = num / 10;
    }
    
    return sum;
}

#define SOCKET_PATH "xxxxx_socket"


int main()
{
    /* 断开之前的socket文件 */
    unlink("server_socket");
    
    /* 创建一个socket */
    int server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    printf("server_sockfd=%d:\n", server_sockfd);

    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);
    
    /* 与本地文件进行绑定 */
    bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    /* 监听 */
    if(listen(server_sockfd, 5)<0)
    {
	    perror("Listen failed");
    }
    
    int client_sockfd;
    struct sockaddr_un client_addr;
    socklen_t len = sizeof(client_addr);
    int num = 0;
    while(1)
    {
        printf("server waiting:\n");
        /* 接受一个客户端的链接 */
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &len);
        
        /*数据交换 */
        read(client_sockfd, &num, 4);
        printf("get an integer from client: %d\n", num);
        num=num_reverse(num);
        write(client_sockfd, &num, 4);
        
        /* 关闭socket */
        close(client_sockfd);
    }
    
    return 0;
}

