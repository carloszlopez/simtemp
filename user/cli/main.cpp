/************************************
 * INCLUDE
 ************************************/
#include <iostream>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <unordered_map>
#include <functional>

/************************************
 * STATIC VARIABLE
 ************************************/
static volatile sig_atomic_t keep_running = 1;

/************************************
 * CLASS
 ************************************/
class Device {
public:
    Device(const std::string& path) : devPath(path) {
        fd = ::open(devPath.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            throw std::runtime_error("Failed to open device: " + devPath + " (" + strerror(errno) + ")");
        }
        std::cout << "Device opened: " << devPath << "\n";
    }

    ~Device() {
        if (fd >= 0) {
            ::close(fd);
            std::cout << "Device closed: " << devPath << "\n";
        }
    }

    void read() {
        char buf[32];
        struct pollfd pfd;
        ssize_t ret_read;

        pfd.fd = fd;
        pfd.events = POLLIN;

        std::cout << "Reading from " << devPath << "\n";

        while (keep_running) {
            int ret = poll(&pfd, 1, 5000); // 5-second timeout
            if (ret < 0) {
                std::cerr << "Poll failed: " << strerror(errno) << "\n";
                return;
            } else if (ret == 0) {
                std::cout << "Poll timeout, no data yet\n";
                return;
            }

            if (pfd.revents & POLLIN) {
                ret_read = ::read(fd, buf, sizeof(buf) - 1);
                if (ret_read < 0) {
                    std::cerr << "Read error: " << strerror(errno) << "\n";
                    return;
                }
                buf[ret_read] = '\0';
                std::cout << "Temperature: " << buf;
            }
        }
    }

private:
    std::string devPath;
    int fd{-1};
};

/************************************
 * STATIC FUNCTION
 ************************************/
void handle_signal(int sig) {
    (void)sig;
    std::cout << "\nExiting...\n";
    keep_running = 0;
}

int main(int argc, char* argv[]) {
    /* Register handlers for SIGINT and SIGTERM */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    try {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <operation>\n";
            return 0;
        }

        std::string operation = argv[1];
        std::string devPath = "/dev/nxp_simtemp";

        Device dev(devPath);

        /* Command dispatcher */
        std::unordered_map<std::string, std::function<void()>> commands;
        commands["read"] = [&dev]() { dev.read(); };

        if (commands.find(operation) == commands.end()) {
            std::cerr << "Unknown operation: " << operation << "\n";
            return 0;
        }

        commands[operation]();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}