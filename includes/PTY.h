#ifndef PTY_H
#define PTY_H

#include<bits/stdc++.h>

#include<pty.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<poll.h>
#include<termios.h>
#include<sys/wait.h>
#include<fcntl.h>

using namespace std;

pid_t create_pty_with_bash(int &master_fd);

#endif
