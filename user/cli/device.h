#ifndef DEVICE_H
#define DEVICE_H

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
#ifdef USE_QT
#include <QObject>
#endif

 /************************************
 * DEFINE
 ************************************/
#define EVENT_MASK_TIMER (0x1)
#define EVENT_MASK_TH (0x2)
#define TEST_CYCLES (2)

 /************************************
 * STRUCT
 ************************************/
struct nxp_simtemp_sample_t {
    uint64_t timestamp_ns;
    int32_t temp_mC;
    uint32_t flags;
} __attribute__((packed));

struct SysfsAttrs {
    int sampling_ms;
    int threshold_mC;
    int mode;
};

 /************************************
 * CLASS
 ************************************/
#ifdef USE_QT
class Device : public QObject {
    Q_OBJECT
#else
class Device {
#endif
public:
#ifdef USE_QT
Device(const std::string& devPath, const std::string& sysfsBase, QObject* parent = nullptr);
#else
    Device(const std::string& devPath, const std::string& sysfsBase);
#endif
    ~Device();
    void read();
    void read_attr(const std::string& name);
    void write_attr(const std::string& name, const std::string& value);
    void test_mode (void);
    
#ifdef USE_QT
signals:
    void readSignal(const nxp_simtemp_sample_t &sample);
#endif

private:
    std::string devPath;
    std::string sysfsBase;
    int fd{-1};
    nxp_simtemp_sample_t sample{};
    SysfsAttrs attrs{};
    std::unordered_map<std::string, int*> attr_map;
};

#endif // DEVICE_H
