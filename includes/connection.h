#ifndef CONNECTION_H
#define CONNECTION_H


#include<bits/stdc++.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
using namespace std;

#define SOCKET_NAME "/tmp/securemux-daemon.socket"
#define BUFFER_SIZE 1024

class Server {
    private:
        int                 ret;
        int                 connection_socket;
        int                 data_socket;
        ssize_t             r, w;
        struct sockaddr_un  name;
        char                buffer[BUFFER_SIZE];
    public:
        Server();
        
        void create_socket();
        void listen_to_connections();
        void pty_handler();
};

class Client {
    private:
        int                 ret;
        int                 data_socket;
        ssize_t             r, w;
        struct sockaddr_un  addr;
        char                buffer[BUFFER_SIZE];
    public:
        Client();

        void create_socket();
        void relay_io();
};

#endif