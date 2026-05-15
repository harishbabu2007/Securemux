#include "PTY.h"

pid_t create_pty_with_bash(int &master_fd) {
    int slave_fd;
    char slave_name[100];

    if (openpty(&master_fd, &slave_fd, slave_name, nullptr, nullptr) == -1) {
        perror("openpty");
    }

    pid_t pid = fork();

    if (pid  == 0) {
        close(master_fd);

        // close all fds inherited from server except slave
        for (int fd = 3; fd < 256; fd++) {
            if (fd != slave_fd) close(fd);
        }
        
        setsid();
        ioctl(slave_fd, TIOCSCTTY, 0);
        dup2(slave_fd, 0);
        dup2(slave_fd, 1);
        dup2(slave_fd, 2);
        close(slave_fd);
        execl("/usr/bin/bash", "bash", (char*)nullptr);
        perror("execl failed");
        _exit(1);
    }

    // parent - just close slave and return
    close(slave_fd);
    return pid;
}

