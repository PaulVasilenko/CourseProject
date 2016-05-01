//
// Created by user on 9.2.16.
//

#include "UnixSocket.h"
#include <string.h>

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SOCKET_ERROR -1
#define TIMEOUT 500

UnixSocket::~UnixSocket() {
    Close();
}

UnixSocket* UnixSocket::Open(int port) {

    int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor < 0)
    {
        perror("Error during socket opening");
        return 0;
    }

    struct sockaddr_in server_address;
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    if (bind(socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        perror("Error during binding");
        return 0;
    }

    listen(socket_file_descriptor, 5);
    return new UnixSocket(socket_file_descriptor);
}

UnixSocket* UnixSocket::Accept()
{
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);

    if (!WaitForReading(TIMEOUT))
    {
        return 0;
    }

    int client = accept(socket_, (struct sockaddr *) &cli_addr, &clilen);
    if (client < 0)
    {
        perror("Error on accept");
        close(socket_);
        return 0;
    }

    return new UnixSocket(client);
}

int UnixSocket::Init() {
    return 0;
}

void UnixSocket::Close() {
    if (socket_ != 0) {
        close(socket_);
        socket_ = 0;
    }
}

int UnixSocket::Send(const char* aBuf, int aSize) {
    return (int)write(socket_, aBuf, (size_t)aSize);
}

bool UnixSocket::WaitForReading(unsigned timeout) {
    fd_set socks;
    FD_ZERO(&socks);
    FD_SET((unsigned)socket_, &socks);

    struct timeval to;
    to.tv_sec = 0;
    to.tv_usec = timeout;

    return select(socket_ + 1, &socks, 0, 0, &to) > 0;
}

int UnixSocket::Receive(char* aBuf, int aSize) {
    memset(aBuf, 0, (size_t)aSize);
    int r = (int)read(socket_, aBuf, (size_t)aSize);
    if (r < 0) {
        fprintf(stderr, "Receive failed\n");
    }
    return r;
}

string UnixSocket::GetIP() const {
    char buf[1024];
    if (gethostname(buf, sizeof(buf)) == SOCKET_ERROR) {
        return "Unknown";
    }

    struct hostent *host = gethostbyname(buf);

    if (host == NULL) {
        return "Unknown";
    }

    char* localIP;
    localIP = inet_ntoa(*(struct in_addr *)*host->h_addr_list);
    return string(localIP);
}

int UnixSocket::Shutdown() {
    return 0;
}
