#include <sys/socket.h>
#include <sys/poll.h>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024
#define TIME_OUT    5000

const char *verifyLogin(char *data);

int main(int argc, char **argv)
{
    int listenPort = SERVER_PORT;
    int servSock, cliSock, ret, on = 1;
    struct sockaddr_in servSockAddr, cliAddr;
    int servSockLen, cliAddrLen;
    char recvBuf[BUFFER_SIZE];
    int n;

    struct pollfd fds;
    nfds_t nfds = 1;
    int timeout;

    if (argc > 1)
    {
        listenPort = strtol(argv[1], NULL, 10);
    }

    // socket() function return a socket descriptor
    servSock = socket(AF_INET, SOCK_STREAM, 0);
    if (servSock < 0)
    {
        perror("socket() failed");
        exit(1);
    }

    // setsockopt allow the local address to be reused when the server is restarted before the required wait time expires.
    ret = setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

    if (ret < 0)
    {
        perror("setsockopt(SOL_REUSEADDR) failed");
        exit(1);
    }

    // after socket desciptor is created, bind() gets a unique name for the socket.
    bzero(&servSockAddr, sizeof(servSockAddr));
    servSockAddr.sin_family = AF_INET;
    servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servSockAddr.sin_port = htons(listenPort);
    servSockLen = sizeof(servSockAddr);

    if (bind(servSock, (struct sockaddr *)&servSockAddr, servSockLen) < 0)
    {
        perror("bind() failed");
        exit(1);
    }

    // listen() allows server accept incoming client connections.
    if (listen(servSock, 10) < 0)
    {
        perror("listen() failed");
        exit(1);
    }

    while (true)
    {
        printf("Ready for a client connect\n");
        // accept() accept an incoming connection request
        cliSock = accept(servSock, (struct sockaddr *)&cliAddr, (socklen_t *)&cliAddrLen);
        
        printf("Received a connection from a client %s\n", inet_ntoa(cliAddr.sin_addr));

        // the poll() allows the process to wait for an event to occur and to wake up the process when the event orcurs.
        memset(&fds, 0, sizeof(fds));
        fds.fd = cliSock;
        fds.events = POLLIN;
        fds.revents = 0;
        ret = poll(&fds, nfds, TIME_OUT);
        if (ret < 0)
        {
            perror("poll() failed");
            exit(1);
        }
        if (ret == 0)
        {
            printf("poll() time out");
            return 0;
        }
        int length = BUFFER_SIZE;
        ret = setsockopt(cliSock, SOL_SOCKET, SO_RCVLOWAT,
                      (char *)&length, sizeof(length));
        
        
        // login
        ret = recv(cliSock, recvBuf, sizeof(recvBuf), 0);
        recvBuf[ret] = '\0';
        const char *sentBuf = verifyLogin(recvBuf);
        printf("%s\n", sentBuf);
        if (strcmp(sentBuf, "Login sucess!") == 0)
        {
            write(cliSock, sentBuf, strlen(sentBuf));
            while (true)
            {
                n = read(cliSock, recvBuf, BUFFER_SIZE - 1);
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
    close(servSock);
}

const char *verifyLogin(char *data)
{
    const int LENGTH = 256;
    char userId[LENGTH];
    char password[LENGTH];

    int i = 0, offset;
    while (data[i] != ',')
    {
        userId[i] = data[i];
        i++;
    }
    userId[i] = '\0';
    i++;
    offset = i;
    while (data[i] != '\0')
    {
        password[i - offset] = data[i];
        i++;
    }
    password[i - offset] = '\0';

    if (strcmp(userId, "admin") == 0 && strcmp(password, "admin") == 0)
    {
        return "Login sucess!";
    }

    return "UserId or password incorrect!";
}