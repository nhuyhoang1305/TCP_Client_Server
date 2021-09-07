#include "filesender.h"
#include <stdio.h>

int main()
{
    FileSender fs;
    int socket = fileno(stdout);
    fs.sendFile("test.txt", socket);
    do
    {
        int done = fs.handle_io();
        if (done)
            break;
    } while (true);
    return 0;
}