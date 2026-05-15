#include "connection.h"
#include "PTY.h"

Client::Client(){}

void Client::create_socket(){
    /* Create local socket. */
    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));

    /* Connect socket to socket address. */
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

    ret = connect(data_socket, (const struct sockaddr *) &addr,
                    sizeof(addr));
    if (ret == -1) {
        fprintf(stderr, "The server/daemon is down.\n");
        exit(EXIT_FAILURE);
    }
}

void Client::relay_io(){    
    /* set raw mode on stdin */
    struct termios orig_termios, raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    cfmakeraw(&raw);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO; // Your keyboard
    fds[0].events = POLLIN;
    fds[1].fd = data_socket;    // Bash's output
    fds[1].events = POLLIN;

    char buf[1024];

    while (true) {
        if (poll(fds, 2, -1) < 0) break;

        // Keyboard -> Master (Shell Input)
        if (fds[0].revents & POLLIN) {
            ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
            if (n <= 0) break;
            if (n > 0) write(data_socket, buf, n);
        }

        // Master -> Stdout (Shell Output)
        if (fds[1].revents & POLLIN) {
            ssize_t n = read(data_socket, buf, sizeof(buf));
            if (n <= 0) break; // Shell exited
            write(STDOUT_FILENO, buf, n);
        }

        // PTY closed (bash exited) — Linux sends POLLHUP
        if (fds[1].revents & POLLHUP) {
            break;
        }
    }

    // Cleanup: Restore terminal mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int Client::get_data_socket(){
    return data_socket;
}

void Client::send_command(string cmd) {
    char *buf = (char *)cmd.c_str();
    write(data_socket, buf, cmd.size());
}

string Client::read_response() {
    return read_line(data_socket);
}