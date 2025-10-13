/************************************
 * INCLUDE
 ************************************/
#include "device.h"

/************************************
 * STATIC VARIABLE
 ************************************/
static volatile sig_atomic_t keep_running = 1;

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