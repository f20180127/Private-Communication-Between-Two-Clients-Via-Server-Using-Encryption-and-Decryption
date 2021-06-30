#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX 1000000

void *communicate(void *param)
{
    char buff[MAX];
    int connfd[2];
    connfd[0] = *((int *)param);
    connfd[1] = *(((int *)param) + 1);
    int cnt;
    bzero(buff, MAX);
    while ((cnt=read(connfd[0], buff, MAX)) > 0)
    {
		write(connfd[1], buff, cnt);
        bzero(buff, MAX);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Command line arguments are not correct.\n");
        return 0;
    }

    int port = atoi(argv[1]);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed...\n");
        exit(1);
    }
    printf("Socket successfully created..\n");

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if ((bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
    {
        printf("Socket bind failed...\n");
        exit(1);
    }
    printf("Socket successfully binded..\n");

    int len;
    struct sockaddr_in cli;
    while (1)
    {
        if ((listen(sockfd, 10)) != 0)
        {
            printf("Listen failed...\n");
            exit(1);
        }
        printf("Server listening..\n");
        int connfd[2];
        connfd[0] = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (connfd[0] < 0)
        {
            printf("Server acccept failed...\n");
            exit(0);
        }
        printf("Connection established with first client. Waiting for another client to join....\n");

        connfd[1] = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (connfd[1] < 0)
        {
            printf("Server acccept failed...\n");
            exit(0);
        }
        printf("Connection established with second client. Input in the clients to begin communication....\n");

        pthread_t tid1,tid2;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tid1, &attr, communicate, connfd);

        int swapped_connfd[2] = {connfd[1], connfd[0]};
        pthread_create(&tid2, &attr, communicate, swapped_connfd);

        pthread_join(tid1, NULL);
        pthread_join(tid2, NULL);
        printf("The two clients have exited. Waiting for new connections...\n");
        close(connfd[0]);
        close(connfd[1]);
    }
    return 0;
}
