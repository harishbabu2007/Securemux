#include "connection.h"

string read_line(int fd){
    string line;
    char c;

    while (read(fd, &c, 1) == 1) {
        if (c == '\n') break;
        line += c;
    }

    return line;
}

vector<string> tokenize_command(string command) {
    vector<string> tokens;
    char *cmd = (char *)command.c_str();
    char *token = strtok(cmd, " ");

    while (token != nullptr) {
        string str(token);
        tokens.push_back(str);
        token = strtok(nullptr, " ");
    }

    return tokens;
}

bool is_new_command(string command){
    vector<string> tokens = tokenize_command(command);
    if (tokens.size() == 2 && tokens[0] == "NEW") return true;
    return false;
}

bool is_attach_command(string command) {
    vector<string> tokens = tokenize_command(command);
    if (tokens.size() == 2 && tokens[0] == "ATTACH") return true;
    return false;
}

bool is_list_command(string command) {
    vector<string> tokens = tokenize_command(command);
    if (tokens.size() == 1 && tokens[0] == "LIST") return true;
    return false;
}

bool is_error_response(string response) {
    vector<string> tokens = tokenize_command(response);
    if (tokens.size() == 2 && tokens[0] == "ERROR") return true;
    return false;
}

bool is_ok_response(string response) {
    vector<string> tokens = tokenize_command(response);
    if (tokens.size() == 1 && tokens[0] == "OK") return true;
    return false;
}

string extract_name(string command) {
    vector<string> tokens = tokenize_command(command);
    return tokens[1];
}

bool Server::is_session_exists(string session_name) {
    for (auto session: sessions) {
        if (session.name == session_name) return true;
    }
    return false;
}

string get_username() {
    struct passwd *pw = getpwuid(getuid());
    return string(pw->pw_name);
}

int Server::get_session_idx(string session_name) {
    for (int i=0; i<sessions.size(); i++){
        if (session_name == sessions[i].name) return i;
    }
    
    return -1;
}