#include <sys/socket.h>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include "filesender.h"

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    char serverAddress[100] = "127.0.0.1";
    int serverPort = SERVER_PORT;
    int connSock;
    struct sockaddr_in servAddr;
    char sentBuf[BUFFER_SIZE], recvBuf[BUFFER_SIZE];
    int sentDataLen, recvDataLen;

    if (argc == 3)
    {
        strcpy(serverAddress, argv[1]);
        serverPort = strtol(argv[2], NULL, 10);
    }

    connSock = socket(AF_INET, SOCK_STREAM, 0);
    if (connSock < 0)
    {
        printf("Error when creating socket\n");
        exit(1);
    }

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(serverAddress);
    servAddr.sin_port = htons(serverPort);

    if (connect(connSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("Error when connecting!");
        exit(1);
    }

    printf("Connected to server...\n");

    FileSender fileSender;
    fileSender.sendFile("test.txt", connSock);
    while (true)
    {
        fileSender.handle_io();
        recvDataLen = read(connSock, recvBuf, sizeof(recvBuf));
        if (recvDataLen < 0)
        {
            printf("Error when receive data\n");
            break;
        }
        recvBuf[recvDataLen] = '\0';
        printf("Message received from server: %s\n", recvBuf);
        fileSender.setState("test.txt");
    }

    close(connSock);
    return 1;
}