#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>

#define SOCKET_PATH "xxxxx_socket"


int main()
{
    /* 创建一个socket */
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_PATH);
    
    /*从键盘读取需要转置的整数*/
    int num;
    printf("Please enter the num to reverse:\n");
	scanf("%d",&num);

    /* 链接至服务端 */
    int result = connect(sockfd, (struct sockaddr *)&address, sizeof(address));
    if(result == -1)
    {
        perror("connect failed: ");
        exit(1);
    }
    
    /* 数据处理 */
    write(sockfd, &num, 4);//一个int 4个字节
    read(sockfd, &num, 4);
    printf("get an integer from server: %d\n", num);
    
    /* 关闭socket */
    close(sockfd);
    
    return 0;
}  


