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
#include <fcntl.h>

#define MAXCLIENTS 10

int main()
{
    int nFileSenders;
    int servSock;
    struct sockaddr_in servaddr;
    int ret;

    struct pollfd fdarray[100];
    int nfds = 1, current_size = 0, i, j;

    // socket() function return a socket descriptor
    servSock = socket(AF_INET, SOCK_STREAM, 0);
    if (servSock < 0)
    {
        perror("socket() failed");
        exit(1);
    }

    // after socket desciptor is created, bind() gets a unique name for the socket.
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9000);

    if (bind(servSock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind() failed");
        exit(1);
    }

    // listen() allows server accept incoming client connections.
    if (listen(servSock, MAXCLIENTS) < 0)
    {
        perror("listen() failed");
        exit(1);
    }

    // init pollfds
    memset(fdarray, 0, sizeof(fdarray));

    fdarray[0].fd = servSock;
    fdarray[0].events = POLLIN;

    do
    {
        printf("Waiting on poll()...\n");
        ret = poll(fdarray, nfds, 10 * 1000);

        if (ret < 0)
        {
            perror("poll() fail");
            return 1;
        }

        current_size = nfds;

        for (i = 0; i < current_size; ++i)
        {
            if (fdarray[i].revents == 0)
                continue;

            if (fdarray[i].fd == servSock)
            {
                // new connection
                int fd = accept(servSock, NULL, NULL);
                if (fd < 0)
                {
                    perror("accept() fail!");
                    return 1;
                }
                fdarray[nfds].fd = fd;
                fdarray[nfds].events = POLLIN;
                nfds++;
            }
            else
            {
                // exist connection
                char buffer[1024];
                ret = read(fdarray[i].fd, &buffer, sizeof(buffer));
                if (ret < 0)
                {
                    perror("read() failed");
                    return 1;
                }
                buffer[ret] = 0;
                printf("%s", buffer);
                write(fdarray[i].fd, buffer, sizeof(buffer));
            }
        }
    } while (true);
}
