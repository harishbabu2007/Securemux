#include "connection.h"

int main(){
    Server server;
    server.create_socket();
    server.listen_to_connections();
    return 0;
}