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
#include <fcntl.h>

#define MAXCLIENTS 10

int main()
{
    int nFileSenders;
    int servSock;
    struct sockaddr_in servaddr;
    int ret;
    int max_sd;
    int des_ready;
    fd_set master_set, working_set;

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

    // init master_set
    FD_ZERO(&master_set);
    FD_SET(servSock, &master_set);
    max_sd = servSock;
    do
    {
        memcpy(&working_set, &master_set, sizeof(master_set));

        printf("Waiting on select()...\n");
        ret = select(max_sd + 1, &working_set, NULL, NULL, NULL);
        //printf("%d\n", ret);
        if (ret < 0)
        {
            perror("select() fail");
            return 1;
        }

        des_ready = ret;

        for (int i = 0; i <= max_sd && des_ready > 0; ++i)
        {
            if (FD_ISSET(i, &working_set))
            {
                --des_ready;
                int fd;
                if (i == servSock)
                {
                    fd = accept(servSock, NULL, NULL);
                    //printf("%d\n", fd);
                    if (fd != -1)
                    {
                        FD_SET(fd, &master_set);
                        if (fd > max_sd)
                        {
                            max_sd = fd;
                        }
                    }
                    else
                        return 1;
                }
                else
                {
                    char buffer[1024];
                    ret = read(i, &buffer, sizeof(buffer));
                    if (ret < 0)
                    {
                        perror("read() error");
                        break;
                    }
                    printf("  %d bytes received\n", ret);
                    buffer[ret] = 0;
                    printf("%s", buffer);
                    ret = send(i, buffer, sizeof(buffer), 0);
                }
            }
        }
    } while (true);
}
