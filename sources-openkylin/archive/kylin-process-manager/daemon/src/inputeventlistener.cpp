#include "inputeventlistener.h"
#include "loghelper.h"
#include <QDebug>
#include <dirent.h>
#include <fcntl.h>
#include <libudev.h>
#include <linux/input.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

InputMonitor::InputMonitor(QObject *parent) : QObject(parent) {
    initUdev();
    scanExistingDevices();

    idleTimer.setSingleShot(true);
    connect(&idleTimer, &QTimer::timeout, this, &InputMonitor::onIdle);
}

InputMonitor::~InputMonitor()
{
    stopListening();

    if (mon) {
        udev_monitor_unref(mon);
    }
    if (udev) {
        udev_unref(udev);
    }
}

void InputMonitor::stopListening()
{
    idleTimer.stop();
    removeAllDeviceListeners();
    PRINT_NOTICE("Input event listening stopped\n");
}

void InputMonitor::removeAllDeviceListeners()
{
    for (auto it = m_deviceMap.begin(); it != m_deviceMap.end(); ++it) {
        it.value().notifier->setEnabled(false);
        delete it.value().notifier;
        close(it.value().fd);
    }

    m_deviceMap.clear();
    if (udev_notifier) {
        udev_notifier->setEnabled(false);
        delete udev_notifier;
        udev_notifier = nullptr;
    }
}

void InputMonitor::initUdev() {
    udev = udev_new();
    if (!udev) {
        PRINT_NOTICE("Failed to create udev context\n");
        return;
    }

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
    udev_monitor_enable_receiving(mon);

    int monFd = udev_monitor_get_fd(mon);
    udev_notifier = new QSocketNotifier(monFd, QSocketNotifier::Read, this);
    connect(udev_notifier, &QSocketNotifier::activated, this, [this]() {
            struct udev_device *dev = udev_monitor_receive_device(this->mon);
            if (!dev)
                return;
            const char *action = udev_device_get_action(dev);
            const char *devNode = udev_device_get_devnode(dev);

            if (strcmp(action, "add") == 0 && devNode && strstr(devNode,"event")) {
                PRINT_NOTICE("Hotplug Device added:%s\n", devNode);
                monitorDevice(devNode);
            } else if (strcmp(action, "remove") == 0) {
                for (auto it = m_deviceMap.begin(); it != m_deviceMap.end(); ) {
                    if (it.value().path == QString(devNode)) {
                        // 禁用并删除notifier
                        it.value().notifier->setEnabled(false);
                        PRINT_NOTICE("%s remove\n", it.value().path.toStdString().c_str());
                        delete it.value().notifier;
                        if (it.value().fd)
                            close(it.value().fd);
                            it = m_deviceMap.erase(it);
                        } else {
                            ++it;
                    }
                }
                PRINT_NOTICE("Hotplug Device removed:%s\n", devNode);
            }
            udev_device_unref(dev);
    });
}

void InputMonitor::onIdle() {
    isIdle = true;
    PRINT_NOTICE("input onIdle");
    Q_EMIT idle();
}

// 检查设备是否支持EV_KEY或EV_REL事件类型
bool InputMonitor::checkDeviceSupportsEvents(int fd) {
    unsigned long bitmask[EV_MAX / 8 + 1] = {0};

    // 获取设备支持的事件类型
    if (ioctl(fd, EVIOCGBIT(0, sizeof(bitmask)), bitmask) == -1) {
        PRINT_NOTICE("Failed to get event bitmask for device, error: %s\n", strerror(errno));
        return false;
    }

    // 检查是否支持EV_KEY或EV_REL
    bool supportsKey = (bitmask[EV_KEY / 8] & (1 << (EV_KEY % 8))) != 0;
    bool supportsRel = (bitmask[EV_REL / 8] & (1 << (EV_REL % 8))) != 0;

    return supportsKey || supportsRel;
}

void InputMonitor::monitorDevice(const QString &path) {
    int fd = open(path.toStdString().c_str(), O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        // 检查设备是否支持所需事件类型
        if (!checkDeviceSupportsEvents(fd)) {
            //qDebug() << "Device" << path << "does not support EV_KEY or EV_REL, skipping";
            close(fd);
            return;
        }

        QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this, [this, fd]() { onDeviceActivity(fd); });
        DeviceInfo info = {fd, notifier, path};
        m_deviceMap[fd] = info;
        PRINT_NOTICE("Started watching::%s\n", path.toUtf8().data());
    }
}

void InputMonitor::onDeviceActivity(int fd) {
    //qDebug() << "onDeviceActivity:" << fd;
    readInputEvent(fd);
}

void InputMonitor::readInputEvent(int fd) {
    struct input_event ev;
    bool hasevent = false;
    while (1) {
        ssize_t n = read(fd, &ev, sizeof(ev)) == sizeof(ev);
        if (n > 0) {
            if (ev.type == EV_KEY && ev.value == 1 || ev.type == EV_REL) {
                //qDebug() << "[Input] Key pressed:" << ev.code;
                hasevent = true;
                break;
            }
        } else if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                PRINT_NOTICE("Error reading from fd %d, %s\n", fd, strerror(errno));
                removeDevice(fd);
                break;
            }
        } else {
            break;
        }
    }

    if (hasevent) {
        idleTimer.start(mSecsidle);
        if (isIdle) {
            isIdle = false;
            Q_EMIT active();
            PRINT_NOTICE("input onActive\n");
        }
    }
}

void InputMonitor::removeDevice(int fd) {
    auto it = m_deviceMap.find(fd);
    if (it != m_deviceMap.end()) {
        PRINT_NOTICE("Removing device listener for fd:%d\n", fd);
        it.value().notifier->setEnabled(false);
        delete it.value().notifier;
        close(it.value().fd);
        m_deviceMap.erase(it);
    }
}

void InputMonitor::scanExistingDevices()
{
    DIR *dir;
    struct dirent *ent;

    dir = opendir("/sys/class/input");
    if (!dir) {
        PRINT_NOTICE("Failed to open /sys/class/input\n");
        return;
    }

    while ((ent = readdir(dir)) != nullptr) { 
        std::string name = ent->d_name; 
        if (name.find("event") != std::string::npos) {
            std::string str = "/dev/input/";
            str += name;
            monitorDevice(QString::fromStdString(str));
        }
    }

    closedir(dir); 
}