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

    cout << "Socket created and bound" << endl;
}

void Server::listen_to_connections(){
    /* Prepare for accepting connections. The backlog size is set to 20. */
    ret = listen(connection_socket, 20);
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    cout << "Server listening..." << endl;

    // call the pty_handler
    pty_handler();

    /* Cleanup */
    close(connection_socket);
    unlink(SOCKET_NAME);
}

void Server::pty_handler() {
    cout << "Waiting for client..." << endl;

    int client_fd = accept(connection_socket, NULL, NULL);
    if (client_fd == -1) { perror("accept"); exit(EXIT_FAILURE); }

    cout << "Client connected! fd=" << client_fd << endl;

    int master_fd;
    pid_t bash_pid = create_pty_with_bash(master_fd);

    cout << "Bash started, pid=" << bash_pid << " master_fd=" << master_fd << endl;

    struct pollfd fds[2];
    fds[0].fd = client_fd; // keyboard
    fds[0].events = POLLIN;
    fds[1].fd = master_fd; // Bash's output
    fds[1].events = POLLIN;

    char buf[1024];

    while (true) {
        if (poll(fds, 2, -1) < 0) break;

        // Keyboard -> Master (Shell Input)
        if (fds[0].revents & POLLIN) {
            ssize_t n = read(client_fd, buf, sizeof(buf));
            if (n <= 0) break;
            write(master_fd, buf, n);
        }

        // Master -> Stdout (Shell Output)
        if (fds[1].revents & POLLIN) {
            ssize_t n = read(master_fd, buf, sizeof(buf));
            if (n <= 0) break; // Shell exited
            write(client_fd, buf, n);
        }

        // PTY closed (bash exited) — Linux sends POLLHUP
        if (fds[1].revents & POLLHUP) {
            break;
        }
    }

    // Cleanup
    close(client_fd);
    close(master_fd);
    waitpid(bash_pid, nullptr, 0);
}