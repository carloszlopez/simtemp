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
#include <iomanip>
#include <stdint.h>
#include <ctime>

/************************************
 * DEFINE
 ************************************/
#define EVENT_MASK_TIMER (0x1) /* Event mask for time expired */
#define EVENT_MASK_TH (0x2) /* Event mask for time threshold crossed */
#define TEST_CYCLES (2) /* Number of cycles to pass test */

/************************************
 * STRUCT
 ************************************/
struct nxp_simtemp_sample_t {
    uint64_t timestamp_ns; /* monotonic timestamp */
    int32_t temp_mC; /* milli-degree Celsius */
    uint32_t flags; /* Events */
} __attribute__((packed));

struct SysfsAttrs {
    int sampling_ms;
    int threshold_mC;
    int mode;
};

/************************************
 * STATIC VARIABLE
 ************************************/
static volatile sig_atomic_t keep_running = 1;

/************************************
 * CLASS
 ************************************/
class Device {
public:
    Device(const std::string& devPath, const std::string& sysfsBase) 
        : devPath(devPath), sysfsBase(sysfsBase) {
        fd = ::open(devPath.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            throw std::runtime_error("Failed to open device: " + devPath + " (" 
                                     + strerror(errno) + ")");
        }
        std::cout << "Device opened: " << devPath << "\n";

        /* Init attr dispatcher */
        attr_map["sampling_ms"]  = &attrs.sampling_ms;
        attr_map["threshold_mC"] = &attrs.threshold_mC;
        attr_map["mode"]         = &attrs.mode;
    }

    ~Device() {
        if (fd >= 0) {
            ::close(fd);
            std::cout << "Device closed: " << devPath << "\n";
        }
    }
    
    void read() {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;

        std::cout << "Reading from " << devPath << "\n";

        int ret = poll(&pfd, 1, 5000); // 5-second timeout
        if (ret < 0) {
            std::cerr << "Poll failed: " << strerror(errno) << "\n";
            return;
        } else if (ret == 0) {
            std::cout << "Poll timeout, no data yet\n";
            return;
        }

        if (pfd.revents & POLLIN) {
            ssize_t ret_read = ::read(fd, &sample, sizeof(sample));
            if (ret_read < 0) {
                std::cerr << "Read error: " << strerror(errno) << "\n";
                return;
            }
            if (ret_read == sizeof(sample)){
                /* Split nanoseconds into seconds and milliseconds */
                uint64_t seconds = sample.timestamp_ns / 1000000000ULL;
                uint64_t millis  = (sample.timestamp_ns / 1000000ULL) % 1000;

                /* Convert seconds to UTC time struct */
                std::time_t t = static_cast<std::time_t>(seconds);
                std::tm tm_utc{};
                gmtime_r(&t, &tm_utc);

                /* Time stamp in 2025-09-22T20:15:04.123Z fromat */
                std::cout << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%S")
                          << '.' << std::setw(3) << std::setfill('0') << millis 
                          << 'Z'
                    /* Time in Celsius */
                          << " temp=" << sample.temp_mC / 1000.0 << "°C"
                    /* Timer alert */
                          << " tmr_flag=" << !!(sample.flags & EVENT_MASK_TIMER)
                    /* Temperature threshold alert */
                          << " th_flag=" << !!(sample.flags & EVENT_MASK_TH)
                          << "\n";
            }
        }
    }

    void read_attr(const std::string& name) {
        auto it = attr_map.find(name);
        if (it == attr_map.end())
            throw std::runtime_error("Unknown sysfs attribute: " + name);

        std::string path = sysfsBase + "/" + name;

        int fd_attr = ::open(path.c_str(), O_RDONLY);
        if (fd_attr < 0) {
            throw std::runtime_error("Failed to open " + path + ": " 
                + strerror(errno));
        }

        char buf[64];
        ssize_t len = ::read(fd_attr, buf, sizeof(buf) - 1);
        if (len < 0) {
        ::close(fd_attr);
            throw std::runtime_error("Failed to read " + path + ": " 
                + strerror(errno));
        }

        buf[len] = '\0';
        std::cout << name << " = " << buf;

        try {
            *(it->second) = std::stoi(buf);
        } catch (const std::exception& e) {
            std::cerr << "Warning: failed to update attribute '" << name << "': " 
                      << e.what() << "\n";
        }
    }

    void write_attr(const std::string& name, const std::string& value) {
        auto it = attr_map.find(name);
        if (it == attr_map.end())
            throw std::runtime_error("Unknown sysfs attribute: " + name);

        std::string path = sysfsBase + "/" + name;
        int fd_attr = ::open(path.c_str(), O_WRONLY);
        if (fd_attr < 0) {
            throw std::runtime_error("Failed to open " + path + " : " 
                + strerror(errno));
        }

        std::string buf = value + "\n";
        ssize_t len = ::write(fd_attr, buf.c_str(), buf.size());
        ::close(fd_attr);

        if (len != static_cast<ssize_t>(buf.size())) {
            throw std::runtime_error("Failed to write value to " + path);
        }

        std::cout << "Wrote " << value << " to " << path << "\n";

        try {
            *(it->second) = std::stoi(value);
        } catch (const std::exception& e) {
            std::cerr << "Warning: failed to update attribute '" << name << "': " 
                      << e.what() << "\n";
        }
    }
    
    void test_mode (void) {
        /* Read current threshold_mC */
        read_attr("threshold_mC");
        int last_threshold_mC = attrs.threshold_mC;

        /* Set threshold_mC to current temperature - 10°C for testing */
        read();
        write_attr("threshold_mC", std::to_string(sample.temp_mC - 10000));

        /* Read TEST_CYCLES times */
        for (int test_cycles = 0; test_cycles < TEST_CYCLES; test_cycles++) {
            read();
            if (!!(sample.flags & EVENT_MASK_TH)) {
                std::cout << "Alert raised in " 
                    << test_cycles + 1 
                    << " cycles " << "\n";

                /* Set last threshold_mC */
                write_attr("threshold_mC", std::to_string(last_threshold_mC));
                return;
            }
        }

        /* Check if treshold flag was raised */
        if (!(sample.flags & EVENT_MASK_TH)) {
            throw std::runtime_error("threshold_mC test failed ");
        }
    }

private:
    std::string devPath;
    std::string sysfsBase;
    int fd{-1};
    struct nxp_simtemp_sample_t sample;
    struct SysfsAttrs attrs;
    std::unordered_map<std::string, int*> attr_map;
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
            std::cerr << "Usage: " << argv[0]
                      << " read | testmode | readattr <name> | writeattr <name> <value>\n";
            return 1;
        }

        std::string operation = argv[1];
        std::string devPath = "/dev/nxp_simtemp";
        std::string sysfsBase = "/sys/class/misc/nxp_simtemp";

        Device dev(devPath, sysfsBase);

        /* Command dispatcher */
        std::unordered_map<std::string, std::function<void()>> commands;

        if (operation == "read") {
            commands["read"] = [&dev]() { while (keep_running) { dev.read(); } };
        } else if (operation == "testmode") {
            commands["testmode"] = [&dev]() { dev.test_mode(); };
        } else if (operation == "readattr" && argc >= 3) {
            std::string arg = argv[2];
            commands["readattr"] = [&dev, arg]() { dev.read_attr(arg); };
        } else if (operation == "writeattr" && argc >= 4) {
            std::string name = argv[2];
            std::string value = argv[3];
            commands["writeattr"] = [&dev, name, value]() { dev.write_attr(name, value); };
        }

        if (commands.find(operation) == commands.end()) {
            std::cerr << "Unknown or invalid operation: " << operation << "\n";
            std::cerr << "Usage: " << argv[0]
                      << " read | testmode | readattr <name> | writeattr <name> <value>\n";
            return 1;
        }

        commands[operation]();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}