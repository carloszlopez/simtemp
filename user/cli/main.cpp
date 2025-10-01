#include <iostream>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>

#define ERROR_OPEN_FAILED 1
#define ERROR_POLL_FAILED 2
#define ERROR_READ_FAILED 3

int main() {
    int ret; /* Return value */
    int poll_ret; /* Poll function return value */
    ssize_t read_ret; /* Read function return value */
    char buf[32]; /* Message buffer */
    struct pollfd pfd; /* */

    int fd = open("/dev/nxp_simtemp", O_RDONLY | O_NONBLOCK);
    if (0 > fd) {
        std::cerr << "Failed to open device\n";
        ret = ERROR_OPEN_FAILED;
    } else {
        pfd.fd = fd;
        pfd.events = POLLIN;
        while (true) {
            std::cout << "Waiting for poll event\n";
            poll_ret = poll(&pfd, 1, 5000);
            if (0 > poll_ret) {
                std::cerr << "Failed to open device\n";
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
        close(fd);
    }
    return ret;
}
