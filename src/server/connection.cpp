#include "connection.h"
#include "PTY.h"



Server::Server(){
    cout << "Server instance created" << endl;
}

void Server::create_socket(){
    /* Create local socket. */
    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connection_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&name, 0, sizeof(name));

    /* Bind socket to socket name. */
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    unlink(SOCKET_NAME); // remove stale socket file if it exists

    ret = bind(
        connection_socket,
        (const struct sockaddr *) &name,
        sizeof(name)
    );  

    if (ret == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

void Server::listen_to_connections(){
    /* Prepare for accepting connections. The backlog size is set to 20. */
    ret = listen(connection_socket, 20);
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // call the pty_handler in a loop
    while (true) {
        pty_handler();
    }
    

    /* Cleanup */
    close(connection_socket);
    unlink(SOCKET_NAME);
}

void Server::pty_handler() {
    int client_fd = accept(connection_socket, NULL, NULL);
    if (client_fd == -1) { perror("accept"); exit(EXIT_FAILURE); }

    string command = read_line(client_fd);

    if (is_new_command(command)) {
        // NEW
        string name = extract_name(command);
        
        if (is_session_exists(name)) {
            char buf[] = "ERROR SessionExists\n";
            write(client_fd, buf, sizeof(buf));
        } else {
            write(client_fd, "OK\n", 3);

            int master_fd;
            pid_t bash_pid = create_pty_with_bash(master_fd);

            session_t new_session = {
                .name = name,
                .master_fd = master_fd,
                .bash_pid = bash_pid,
                .owner = get_username(),
                .attached = true
            };
            add_session(new_session);

            struct pollfd fds[2];
            fds[0].fd = client_fd; // keyboard
            fds[0].events = POLLIN;
            fds[1].fd = master_fd; // Bash's output
            fds[1].events = POLLIN;

            char buf[1024];

            while (true) {
                if (poll(fds, 2, -1) < 0) break;

                // client disconnects + detach, keep session alive
                if (fds[0].revents & POLLHUP) {
                    mark_session(name, false);
                    close(client_fd);
                    break;
                }

                // PTY closed (bash exited) + remove_session
                if (fds[1].revents & POLLHUP) {
                    remove_session(name);
                    close(master_fd);
                    waitpid(bash_pid, nullptr, 0);
                    close(client_fd);
                    break;
                }

                // Keyboard -> Master (Shell Input)
                if (fds[0].revents & POLLIN) {
                    ssize_t n = read(client_fd, buf, sizeof(buf));
                    if (n <= 0) { mark_session(name, false); break; }
                    write(master_fd, buf, n);
                }

                // Master -> Stdout (Shell Output)
                if (fds[1].revents & POLLIN) {
                    ssize_t n = read(master_fd, buf, sizeof(buf));
                    if (n <= 0) { mark_session(name, false); break; }
                    write(client_fd, buf, n);
                }
            }
        }
    } else if (is_attach_command(command)) {
        // ATTACH
        string name = extract_name(command);

        if (is_session_exists(name)){
            int idx = get_session_idx(name);

            if (sessions[idx].attached) {
                write(client_fd, "ERROR SessionBusy\n", 18);
                close(client_fd);
                return;
            }

            mark_session(name, true);
            write(client_fd, "OK\n", 3);

            int master_fd = sessions[idx].master_fd;
            pid_t bash_pid = sessions[idx].bash_pid;

            struct pollfd fds[2];
            fds[0].fd = client_fd; // keyboard
            fds[0].events = POLLIN;
            fds[1].fd = master_fd; // Bash's output
            fds[1].events = POLLIN;

            char buf[1024];

            while (true) {
                if (poll(fds, 2, -1) < 0) break;

                // client disconnects + detach, keep session alive
                if (fds[0].revents & POLLHUP) {
                    mark_session(name, false);
                    close(client_fd);
                    break;
                }

                // PTY closed (bash exited) + remove_session
                if (fds[1].revents & POLLHUP) {
                    remove_session(name);
                    close(master_fd);
                    waitpid(bash_pid, nullptr, 0);
                    close(client_fd);
                    break;
                }

                // Keyboard -> Master (Shell Input)
                if (fds[0].revents & POLLIN) {
                    ssize_t n = read(client_fd, buf, sizeof(buf));
                    if (n <= 0) { mark_session(name, false); break; }
                    write(master_fd, buf, n);
                }

                // Master -> Stdout (Shell Output)
                if (fds[1].revents & POLLIN) {
                    ssize_t n = read(master_fd, buf, sizeof(buf));
                    if (n <= 0) { mark_session(name, false); break; }
                    write(client_fd, buf, n);
                }
            }
        } else {
            char buf[] = "ERROR SessionDoesNOTExists\n";
            write(client_fd, buf, sizeof(buf));
        }
    } else if (is_list_command(command)) {
        string response = "";
        for (auto& s : sessions) {
            response += s.name + " " + s.owner + " ";
            response += (s.attached ? "attached" : "free");
            response += "\n";
        }
        response += "END\n";
        char *buf = (char *)response.c_str();
        write(client_fd, buf, response.size());
        close(client_fd);
    }
}

void Server::add_session(session_t session){
    sessions.push_back(session);
}

void Server::mark_session(string session_name, bool attached) {
    int s_idx = get_session_idx(session_name);
    sessions[s_idx].attached = attached;
}

void Server::remove_session(string session_name) {
    int idx = get_session_idx(session_name);
    if (idx != -1) sessions.erase(sessions.begin() + idx);
}