#ifndef CONNECTION_H
#define CONNECTION_H


#include<bits/stdc++.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
#include <pwd.h>
using namespace std;

#define SOCKET_NAME "/tmp/securemux-daemon.socket"
#define BUFFER_SIZE 1024

typedef struct Session {
    string name;
    int    master_fd;
    pid_t  bash_pid;
    string owner;
    bool   attached;
} session_t;

class Server {
    private:
        int                 ret;
        int                 connection_socket;
        int                 data_socket;
        ssize_t             r, w;
        struct sockaddr_un  name;
        char                buffer[BUFFER_SIZE];
        vector<session_t>   sessions;
    public:
        Server();
        
        void create_socket();
        void listen_to_connections();
        void pty_handler();

        bool is_session_exists(string session_name);
        void add_session(session_t session);
        void mark_session(string session_name, bool attached);
        void remove_session(string session_name);
        int get_session_idx(string session_name);
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

        int get_data_socket();
        void send_command(string cmd);
        string read_response();
};


/* Command utilites */
string read_line(int fd);
vector<string> tokenize_command(string command);

bool is_new_command(string command);
bool is_attach_command(string command);
bool is_list_command(string command);

bool is_error_response(string message);
bool is_ok_response(string response);

string extract_name(string command);
string get_username();

#endif