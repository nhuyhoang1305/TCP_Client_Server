#include <sys/socket.h>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024    

int main(int argc, char **argv)
{
    int     listenPort = SERVER_PORT;
    int     servSock, cliSock;
    struct  sockaddr_in servSockAddr, cliAddr;
    int     servSockLen, cliAddrLen;
    char    recvBuf[BUFFER_SIZE];
    int     n;


    if (argc > 1)
    {
        listenPort = strtol(argv[1], NULL, 10);
    }

    servSock = socket(AF_INET,  SOCK_STREAM, 0);
    bzero(&servSockAddr, sizeof(servSockAddr));
    servSockAddr.sin_family = AF_INET;
    servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servSockAddr.sin_port = htons(listenPort);

    servSockLen = sizeof(servSockAddr);
    if (bind(servSock, (struct sockaddr *)&servSockAddr, servSockLen) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(servSock, 3) < 0)
    {
        perror("listen");
        exit(1);
    }

    while (true)
    {
        printf("Waiting for a client....\n");
        cliSock = accept(servSock, (struct sockaddr *) &cliAddr, (socklen_t *)&cliAddrLen);
        printf("Received a connection from a client %s\n", inet_ntoa(cliAddr.sin_addr));
        while (true)
        {
            n = read(cliSock, recvBuf, BUFFER_SIZE-1);
            recvBuf[n] = '\0';
            if (n == 0 || strcmp(recvBuf, "@close") == 0)
            {
                close(cliSock);
                printf("Close the connection\n");
                break;
            }
            printf("Received a message from the client: %s\n", recvBuf);
            for (int i = 0; i < n; ++i)
            {
                if (recvBuf[i] >= 'a' && recvBuf[i] <= 'z')
                {
                    recvBuf[i] -= 32;
                }
            }
            write(cliSock, recvBuf, n);
            printf("Sent a message to the client: %s\n", recvBuf);
        }
    }

}