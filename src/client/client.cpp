#include<bits/stdc++.h>
#include"connection.h"
using namespace std;

void print_help(){
    cout << "Usage: securemux new <name>" << endl;
    cout << "       securemux attach <name>" << endl;
    cout << "       securemux list" << endl;
}


int main(int argc, char* argv[]){
    if (argc < 2) {
        print_help();
        return 1;
    }

    Client client;
    client.create_socket();

    string cmd = string(argv[1]);
    
    if (cmd == "new"){
        if (argc < 3) {
            cout << "Usage: securemux new <name>" << endl; return 1;
        }

        string name = string(argv[2]);
        string msg = "NEW " + name + '\n';
        client.send_command(msg);

        string buffer_recieved = client.read_response();

        if (is_error_response(buffer_recieved)) {
            cout << "Error creating new session" << endl;
            cout << buffer_recieved << endl;
            return 1; 
        } else if (is_ok_response(buffer_recieved)) {
            client.relay_io();
        }
    } else if (cmd == "attach") {
        if (argc < 3) {
            cout << "Usage: securemux attach <name>" << endl; return 1;
        }

        string name = string(argv[2]);
        string msg = "ATTACH " + name + '\n';
        client.send_command(msg);

        string buffer_recieved = client.read_response();

        if (is_error_response(buffer_recieved)) {
            cout << "Error attaching session" << endl;
            cout << buffer_recieved << endl;
            return 1; 
        } else if (is_ok_response(buffer_recieved)) {
            client.relay_io();
        }
    } else if (cmd == "list") {
        client.send_command("LIST\n");

        while (true) {
            string line = client.read_response();
            if (line == "END") break;
            cout << line << endl;
        }

    } else {
        print_help();
        return 1;
    }

    return 0;
}