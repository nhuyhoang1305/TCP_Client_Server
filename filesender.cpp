#include "filesender.h"
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


FileSender::FileSender()
{
    state = INIT;
}

void FileSender::sendFile(const char *fileName, int socket)
{
    // set nonblocking
    //fcntl(socket, F_SETFL, O_NONBLOCK);
    fd = open(fileName, O_RDONLY);
    if (fd < 0)
    {
        perror("Open failed");
        return;
    }
    len = 0;
    len_used = 0;
    this->socket = socket;
    state = SENDING;
}

int FileSender::getFd() { return fd; }

int FileSender::handle_io()
{
    //printf("hello");
    if (state != SENDING)
    {
        return 2;
    }
    if (len_used == len)
    {
        len = read(fd, buffer, BUFFERSIZE);
        if (len == 0)
        {
            // file sent
            //printf("len = 0");
            close(fd);
            //close(socket);
            state = DONE;
            return 1;
        }
        len_used = 0;
    }
    len_write = write(socket, buffer+len_used, len-len_used);
    len_used += len_write;
    
    return 0;
}

void FileSender::setState(const char *fileName)
{
    len = 0;
    len_used = 0;
    fd = open(fileName, O_RDONLY);
    state = SENDING;
}