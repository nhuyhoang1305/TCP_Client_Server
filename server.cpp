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
#include <thread>

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024
#define TIME_OUT 30000
#define MAX_CLIENT 10

int clientCount = 0;
std::thread thread[MAX_CLIENT];

struct Client
{
    int index;
    int sockID;
    struct sockaddr_in cliAddr;
    int len;
};

Client client[MAX_CLIENT];

void *createClientThread(void *clientDetail);
const char *verifyLogin(char *data);

int main(int argc, char **argv)
{
    int listenPort = SERVER_PORT;
    int servSock, ret, on = 1;
    struct sockaddr_in servSockAddr;
    int servSockLen;

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
    if (listen(servSock, MAX_CLIENT) < 0)
    {
        perror("listen() failed");
        exit(1);
    }

    while (clientCount < MAX_CLIENT)
    {
        printf("Ready for a client connect\n");
        client[clientCount].sockID = accept(servSock, (struct sockaddr *)&client[clientCount].cliAddr, (socklen_t *)&client[clientCount].len);
        thread[clientCount] = std::thread(createClientThread, (void *)&client[clientCount]);
        ++clientCount;
    }
    for (int i = 0; i < clientCount; ++i)
    {
        thread[clientCount].join();
    }
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

void *createClientThread(void *_clientDetail)
{
    struct pollfd fds;
    nfds_t nfds = 1;
    struct Client *clientDetail = (struct Client *)_clientDetail;
    int index = clientDetail->index;
    int cliSocket = clientDetail->sockID;
    int ret;
    char recvBuf[BUFFER_SIZE];
    printf("Received a connection from a client %s\n", inet_ntoa(clientDetail->cliAddr.sin_addr));

    memset(&fds, 0, sizeof(fds));
    fds.fd = clientDetail->sockID;
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
    ret = setsockopt(clientDetail->sockID, SOL_SOCKET, SO_RCVLOWAT,
                     (char *)&length, sizeof(length));

    // login
    ret = recv(clientDetail->sockID, recvBuf, sizeof(recvBuf), 0);
    recvBuf[ret] = '\0';
    const char *sentBuf = verifyLogin(recvBuf);
    printf("%s\n", sentBuf);
    if (strcmp(sentBuf, "Login sucess!") == 0)
    {
        write(clientDetail->sockID, sentBuf, strlen(sentBuf));
        while (true)
        {
            ret = read(clientDetail->sockID, recvBuf, BUFFER_SIZE - 1);
            recvBuf[ret] = '\0';
            if (ret == 0 || strcmp(recvBuf, "@close") == 0)
            {
                close(clientDetail->sockID);
                printf("Close the connection\n");
                break;
            }
            printf("Received a message from the client: %s\n", recvBuf);
            for (int i = 0; i < ret; ++i)
            {
                if (recvBuf[i] >= 'a' && recvBuf[i] <= 'z')
                {
                    recvBuf[i] -= 32;
                }
            }
            write(clientDetail->sockID, recvBuf, ret);
            printf("Sent a message to the client: %s\n", recvBuf);
        }
    }
    return NULL;
}