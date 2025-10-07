#include "device.h"

Device::Device(const std::string& devPath, const std::string& sysfsBase) 
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

Device::~Device() {
    if (fd >= 0) {
        ::close(fd);
        std::cout << "Device closed: " << devPath << "\n";
    }
}

void Device::read() {
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

void Device::read_attr(const std::string& name) {
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

void Device::write_attr(const std::string& name, const std::string& value) {
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

void Device::test_mode (void) {
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