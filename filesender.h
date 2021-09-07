#define BUFFERSIZE 1024

class FileSender
{
private:
    int fd; //file descriptor
    char buffer[BUFFERSIZE];
    int len;
    int len_used;
    enum
    {
        INIT,
        SENDING,
        DONE
    } state;
    int socket; // file socket descriptor
    int len_write;

public:
    FileSender();
    void sendFile(const char *filename, int socket);
    int handle_io();
    int getFd();
    void setState(const char *filename);
};