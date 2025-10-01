#include <iostream>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>

#define ERROR_OPEN_FAILED 1
#define ERROR_POLL_FAILED 2
#define ERROR_READ_FAILED 3

static volatile sig_atomic_t keep_running = 1;

void handle_signal(int sig) {
    if (sig == SIGTERM) {
        std::cout << "\nCaught SIGTERM, exiting...\n";
    } else if (sig == SIGINT) {
        std::cout << "\nCaught SIGINT (Ctrl+C), exiting...\n";
    } else {
        /* Do nothing */
    }
    keep_running = 0;
}


int main() {
    int ret; /* Return value */
    int poll_ret; /* Poll function return value */
    ssize_t read_ret; /* Read function return value */
    char buf[32]; /* Message buffer */
    struct pollfd pfd; /* poll file */

    /* Register handlers for SIGINT and SIGTERM */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    /* Open simtemp */
    int fd = open("/dev/nxp_simtemp", O_RDONLY | O_NONBLOCK);
    if (0 > fd) {
        std::cerr << "Failed to open device\n";
        ret = ERROR_OPEN_FAILED;
    } else {
        /* Read when data is ready (poll) */
        pfd.fd = fd;
        pfd.events = POLLIN;
        while (keep_running) {
            std::cout << "Waiting for poll event\n";
            poll_ret = poll(&pfd, 1, 5000);
            if (0 > poll_ret) {
                std::cerr << "Failed in poll\n";
                ret = ERROR_POLL_FAILED;
                break;
            } else if (0 == poll_ret) {
                std::cout << "Timeout\n";
            } else {
                /* Read temperature */
                if (pfd.revents & POLLIN) {
                    read_ret = read(fd, buf, sizeof(buf) - 1);
                    if (0 > read_ret) {
                        std::cerr << "Read error: " << strerror(errno) << "\n";
                        ret = ERROR_READ_FAILED;
                    } else if (read_ret == 0) {
                        // std::cout << "EOF reached\n";    
                    } else {
                        buf[read_ret] = '\0';
                        std::cout << "Temperature: " << buf;
                    }
                }
            }
        }
        std::cout << "Exiting, closing device\n";
        close(fd);
    }
    return ret;
}
