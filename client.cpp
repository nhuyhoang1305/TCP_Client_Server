#include <sys/socket.h>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    char    serverAddress[100] = "127.0.0.1";
    int     serverPort = SERVER_PORT;
    int     connSock;
    struct  sockaddr_in servAddr;
    char    sentBuf[BUFFER_SIZE], recvBuf[BUFFER_SIZE];
    int     sentDataLen, recvDataLen;

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
    
    if (connect(connSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    {
        printf("Error when connecting!");
        exit(1);
    }

    printf("Connected to server...\n");
    while (true)
    {
        printf("Input a message to server (@ for exit): ");
        fgets(sentBuf, BUFFER_SIZE, stdin);
        if (strcmp(sentBuf, "@\n") == 0)
        {
            strcpy(sentBuf, "@close");
            write(connSock, sentBuf, strlen(sentBuf));
            break;
        }
        sentDataLen = write(connSock, sentBuf, strlen(sentBuf)-1);
        if (sentDataLen < 0)
        {
            printf("Error when send data");
            break;
        }
        recvDataLen = read(connSock, recvBuf, sizeof(recvBuf));
        if (recvDataLen < 0)
        {
            printf("Error when receive data\n");
            break;
        }
        recvBuf[recvDataLen]='\0';
        printf("Message received from server: %s\n", recvBuf);
    }
    close(connSock);
    return 1;

}