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
#include "filesender.h"
#include <fcntl.h>

#define MAXCLIENTS 10

int setNonblocking(int fd)
{
    return fcntl(fd, F_SETFL, O_NONBLOCK);
}

int main()
{
    FileSender fileSenders[MAXCLIENTS];
    int nFileSenders;
    int socket = fileno(stdout);

    // set nonblocking
    //setNonblocking(socket);

    nFileSenders = 0;
    do
    {
        if (nFileSenders < MAXCLIENTS)
        {
            fileSenders[nFileSenders].sendFile("test.txt", socket);
            nFileSenders++;
        }

        for (int i = 0; i < nFileSenders; ++i)
        {
            fileSenders[i].handle_io();
        }
    } while (true);
}
